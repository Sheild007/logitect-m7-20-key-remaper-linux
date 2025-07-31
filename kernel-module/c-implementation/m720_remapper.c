/*
 * Logitech M720 Triathlon Button Remapper Kernel Module
 * 
 * This kernel module intercepts button events from Logitech M720 mouse
 * and remaps side buttons to workspace switching key combinations.
 * 
 * Author: GitHub Copilot
 * License: GPL v2
 * Version: 1.0.0
 */

#include "m720_remapper.h"

/* Module parameters */
static int debug_mode = 0;
module_param(debug_mode, int, 0644);
MODULE_PARM_DESC(debug_mode, "Enable debug output (0=disabled, 1=enabled)");

static int remap_side_buttons = 1;
module_param(remap_side_buttons, int, 0644);
MODULE_PARM_DESC(remap_side_buttons, "Remap side buttons (0=disabled, 1=enabled)");

static int remap_extra_buttons = 1;
module_param(remap_extra_buttons, int, 0644);
MODULE_PARM_DESC(remap_extra_buttons, "Remap extra buttons (0=disabled, 1=enabled)");

/* Global variables */
static struct input_handler m720_handler;
static struct input_dev *global_virtual_kbd = NULL;
static int device_count = 0;

/* Device ID table for M720 variants */
static const struct input_device_id m720_ids[] = {
    {
        .flags = INPUT_DEVICE_ID_MATCH_EVBIT | INPUT_DEVICE_ID_MATCH_KEYBIT,
        .evbit = { BIT_MASK(EV_KEY) },
        .keybit = { [BIT_WORD(BTN_MOUSE)] = BIT_MASK(BTN_LEFT) },
    },
    { }, /* Terminating entry */
};

MODULE_DEVICE_TABLE(input, m720_ids);

/* Debug macro */
#define m720_debug(fmt, args...) \
    do { \
        if (debug_mode) \
            printk(KERN_INFO MODULE_NAME ": " fmt, ##args); \
    } while (0)

/*
 * Check if the input device is a Logitech M720
 */
static bool is_m720_device(struct input_dev *dev)
{
    struct usb_device *usb_dev;
    struct hid_device *hid_dev;
    
    if (!dev || !dev->name)
        return false;
    
    /* Check device name patterns */
    if (strstr(dev->name, "M720") || 
        strstr(dev->name, "Logitech MX Master") ||
        strstr(dev->name, "Logitech Wireless Mouse")) {
        
        m720_debug("Found potential M720 device by name: %s\n", dev->name);
        
        /* Try to get USB device info */
        if (dev->dev.parent && dev->dev.parent->parent) {
            usb_dev = to_usb_device(dev->dev.parent->parent);
            if (usb_dev && 
                usb_dev->descriptor.idVendor == LOGITECH_VENDOR_ID) {
                
                m720_debug("USB device - Vendor: 0x%04x, Product: 0x%04x\n",
                          usb_dev->descriptor.idVendor,
                          usb_dev->descriptor.idProduct);
                
                /* Check for known M720 product IDs */
                if (usb_dev->descriptor.idProduct == M720_PRODUCT_ID_1 ||
                    usb_dev->descriptor.idProduct == M720_PRODUCT_ID_2 ||
                    usb_dev->descriptor.idProduct == M720_PRODUCT_ID_3) {
                    return true;
                }
            }
        }
        
        /* Try HID device info */
        if (dev->dev.parent) {
            hid_dev = to_hid_device(dev->dev.parent);
            if (hid_dev && hid_dev->vendor == LOGITECH_VENDOR_ID) {
                m720_debug("HID device - Vendor: 0x%04x, Product: 0x%04x\n",
                          hid_dev->vendor, hid_dev->product);
                return true;
            }
        }
        
        /* Fallback: if name matches and has required buttons, assume it's M720 */
        if (test_bit(BTN_SIDE, dev->keybit) && 
            test_bit(BTN_EXTRA, dev->keybit)) {
            m720_debug("Device has required buttons, assuming M720\n");
            return true;
        }
    }
    
    return false;
}

/*
 * Print device information for debugging
 */
static void print_device_info(struct input_dev *dev)
{
    if (!debug_mode || !dev)
        return;
        
    printk(KERN_INFO MODULE_NAME ": Device Info:\n");
    printk(KERN_INFO MODULE_NAME ":   Name: %s\n", dev->name ?: "Unknown");
    printk(KERN_INFO MODULE_NAME ":   Phys: %s\n", dev->phys ?: "Unknown");
    printk(KERN_INFO MODULE_NAME ":   Uniq: %s\n", dev->uniq ?: "Unknown");
    
    if (test_bit(EV_KEY, dev->evbit)) {
        printk(KERN_INFO MODULE_NAME ":   Key events supported\n");
        if (test_bit(BTN_LEFT, dev->keybit))
            printk(KERN_INFO MODULE_NAME ":     BTN_LEFT\n");
        if (test_bit(BTN_RIGHT, dev->keybit))
            printk(KERN_INFO MODULE_NAME ":     BTN_RIGHT\n");
        if (test_bit(BTN_MIDDLE, dev->keybit))
            printk(KERN_INFO MODULE_NAME ":     BTN_MIDDLE\n");
        if (test_bit(BTN_SIDE, dev->keybit))
            printk(KERN_INFO MODULE_NAME ":     BTN_SIDE\n");
        if (test_bit(BTN_EXTRA, dev->keybit))
            printk(KERN_INFO MODULE_NAME ":     BTN_EXTRA\n");
        if (test_bit(BTN_FORWARD, dev->keybit))
            printk(KERN_INFO MODULE_NAME ":     BTN_FORWARD\n");
        if (test_bit(BTN_BACK, dev->keybit))
            printk(KERN_INFO MODULE_NAME ":     BTN_BACK\n");
    }
}

/*
 * Create virtual keyboard device for sending key combinations
 */
static struct input_dev *create_virtual_keyboard(void)
{
    struct input_dev *virt_kbd;
    int error;
    
    virt_kbd = input_allocate_device();
    if (!virt_kbd) {
        printk(KERN_ERR MODULE_NAME ": Failed to allocate virtual keyboard\n");
        return NULL;
    }
    
    virt_kbd->name = "M720 Virtual Keyboard";
    virt_kbd->phys = "m720/input/kbd";
    virt_kbd->id.bustype = BUS_VIRTUAL;
    virt_kbd->id.vendor = 0x0001;
    virt_kbd->id.product = 0x0001;
    virt_kbd->id.version = 0x0100;
    
    /* Set up key capabilities */
    __set_bit(EV_KEY, virt_kbd->evbit);
    __set_bit(EV_SYN, virt_kbd->evbit);
    
    /* Add keys we need for workspace switching */
    __set_bit(KEY_LEFTMETA, virt_kbd->keybit);
    __set_bit(KEY_PAGEUP, virt_kbd->keybit);
    __set_bit(KEY_PAGEDOWN, virt_kbd->keybit);
    __set_bit(KEY_LEFTALT, virt_kbd->keybit);
    __set_bit(KEY_TAB, virt_kbd->keybit);
    
    error = input_register_device(virt_kbd);
    if (error) {
        printk(KERN_ERR MODULE_NAME ": Failed to register virtual keyboard: %d\n", error);
        input_free_device(virt_kbd);
        return NULL;
    }
    
    m720_debug("Virtual keyboard created successfully\n");
    return virt_kbd;
}

/*
 * Destroy virtual keyboard device
 */
static void destroy_virtual_keyboard(struct input_dev *virt_kbd)
{
    if (virt_kbd) {
        input_unregister_device(virt_kbd);
        m720_debug("Virtual keyboard destroyed\n");
    }
}

/*
 * Send key combination via virtual keyboard
 */
static void send_key_combination(struct input_dev *virt_kbd, 
                                unsigned int key1, unsigned int key2)
{
    if (!virt_kbd) {
        printk(KERN_ERR MODULE_NAME ": Virtual keyboard not available\n");
        return;
    }
    
    m720_debug("Sending key combination: %d + %d\n", key1, key2);
    
    /* Press keys */
    input_report_key(virt_kbd, key1, 1);
    input_report_key(virt_kbd, key2, 1);
    input_sync(virt_kbd);
    
    /* Small delay */
    msleep(10);
    
    /* Release keys */
    input_report_key(virt_kbd, key2, 0);
    input_report_key(virt_kbd, key1, 0);
    input_sync(virt_kbd);
}

/*
 * Handle input events from M720 mouse
 */
static void m720_event(struct input_handle *handle, unsigned int type,
                      unsigned int code, int value)
{
    struct m720_device *m720_dev = handle->private;
    
    /* Only process key events */
    if (type != EV_KEY)
        return;
    
    m720_debug("Event: type=%d, code=%d, value=%d\n", type, code, value);
    
    /* Only process key press events (value == 1) */
    if (value != 1)
        return;
    
    /* Handle side buttons */
    if (remap_side_buttons) {
        switch (code) {
        case BTN_SIDE:
            m720_debug("Side button 1 pressed - sending workspace down\n");
            send_key_combination(global_virtual_kbd, 
                               WORKSPACE_DOWN_KEY1, WORKSPACE_DOWN_KEY2);
            return; /* Don't forward original event */
            
        case BTN_EXTRA:
            m720_debug("Side button 2 pressed - sending workspace up\n");
            send_key_combination(global_virtual_kbd, 
                               WORKSPACE_UP_KEY1, WORKSPACE_UP_KEY2);
            return; /* Don't forward original event */
        }
    }
    
    /* Handle forward/back buttons if enabled */
    if (remap_extra_buttons) {
        switch (code) {
        case BTN_FORWARD:
            m720_debug("Forward button pressed - sending Alt+Tab\n");
            send_key_combination(global_virtual_kbd, KEY_LEFTALT, KEY_TAB);
            return; /* Don't forward original event */
            
        case BTN_BACK:
            m720_debug("Back button pressed - sending workspace down\n");
            send_key_combination(global_virtual_kbd, 
                               WORKSPACE_DOWN_KEY1, WORKSPACE_DOWN_KEY2);
            return; /* Don't forward original event */
        }
    }
    
    /* For all other events, let them pass through normally */
}

/*
 * Filter function - determines if we should process this event
 */
static bool m720_filter(struct input_handle *handle, unsigned int type,
                       unsigned int code, int value)
{
    /* We want to intercept and potentially block certain key events */
    if (type == EV_KEY && value == 1) { /* Key press events only */
        if (remap_side_buttons && 
            (code == BTN_SIDE || code == BTN_EXTRA)) {
            /* Block these events - we'll handle them ourselves */
            m720_event(handle, type, code, value);
            return true; /* true = filter out (don't pass through) */
        }
        
        if (remap_extra_buttons && 
            (code == BTN_FORWARD || code == BTN_BACK)) {
            /* Block these events - we'll handle them ourselves */
            m720_event(handle, type, code, value);
            return true; /* true = filter out (don't pass through) */
        }
    }
    
    return false; /* false = let event pass through normally */
}

/*
 * Match function - determines if we should handle this device
 */
static bool m720_match(struct input_handler *handler, struct input_dev *dev)
{
    return is_m720_device(dev);
}

/*
 * Connect to a new M720 device
 */
static int m720_connect(struct input_handler *handler, struct input_dev *dev,
                       const struct input_device_id *id)
{
    struct m720_device *m720_dev;
    struct input_handle *handle;
    int error;
    
    /* Check if this is actually an M720 device */
    if (!is_m720_device(dev)) {
        m720_debug("Device %s is not an M720, skipping\n", 
                  dev->name ?: "Unknown");
        return -ENODEV;
    }
    
    printk(KERN_INFO MODULE_NAME ": Connecting to M720 device: %s\n", 
           dev->name ?: "Unknown");
    print_device_info(dev);
    
    /* Allocate memory for our device structure */
    m720_dev = kzalloc(sizeof(struct m720_device), GFP_KERNEL);
    if (!m720_dev) {
        printk(KERN_ERR MODULE_NAME ": Failed to allocate device memory\n");
        return -ENOMEM;
    }
    
    /* Initialize the handle */
    handle = &m720_dev->handle;
    handle->dev = dev;
    handle->handler = handler;
    handle->name = MODULE_NAME;
    handle->private = m720_dev;
    
    /* Store device info */
    snprintf(m720_dev->name, sizeof(m720_dev->name), "%s", 
             dev->name ?: "M720");
    snprintf(m720_dev->phys, sizeof(m720_dev->phys), "%s", 
             dev->phys ?: "unknown");
    
    m720_dev->input_dev = dev;
    m720_dev->enabled = true;
    
    /* Register the handle */
    error = input_register_handle(handle);
    if (error) {
        printk(KERN_ERR MODULE_NAME ": Failed to register handle: %d\n", error);
        kfree(m720_dev);
        return error;
    }
    
    /* Open the handle */
    error = input_open_device(handle);
    if (error) {
        printk(KERN_ERR MODULE_NAME ": Failed to open device: %d\n", error);
        input_unregister_handle(handle);
        kfree(m720_dev);
        return error;
    }
    
    device_count++;
    printk(KERN_INFO MODULE_NAME ": Successfully connected to M720 device (total: %d)\n", 
           device_count);
    
    return 0;
}

/*
 * Disconnect from M720 device
 */
static void m720_disconnect(struct input_handle *handle)
{
    struct m720_device *m720_dev = handle->private;
    
    printk(KERN_INFO MODULE_NAME ": Disconnecting from M720 device: %s\n",
           m720_dev ? m720_dev->name : "Unknown");
    
    input_close_device(handle);
    input_unregister_handle(handle);
    
    if (m720_dev) {
        kfree(m720_dev);
        device_count--;
    }
    
    printk(KERN_INFO MODULE_NAME ": M720 device disconnected (remaining: %d)\n", 
           device_count);
}

/* Input handler structure */
static struct input_handler m720_handler = {
    .filter     = m720_filter,
    .match      = m720_match,
    .connect    = m720_connect,
    .disconnect = m720_disconnect,
    .name       = MODULE_NAME,
    .id_table   = m720_ids,
};

/*
 * Module initialization
 */
static int __init m720_remapper_init(void)
{
    int error;
    
    printk(KERN_INFO MODULE_NAME ": Loading Logitech M720 Button Remapper v%s\n", 
           MODULE_VERSION);
    printk(KERN_INFO MODULE_NAME ": Debug mode: %s\n", 
           debug_mode ? "enabled" : "disabled");
    printk(KERN_INFO MODULE_NAME ": Side button remapping: %s\n",
           remap_side_buttons ? "enabled" : "disabled");
    printk(KERN_INFO MODULE_NAME ": Extra button remapping: %s\n",
           remap_extra_buttons ? "enabled" : "disabled");
    
    /* Create virtual keyboard */
    global_virtual_kbd = create_virtual_keyboard();
    if (!global_virtual_kbd) {
        printk(KERN_ERR MODULE_NAME ": Failed to create virtual keyboard\n");
        return -ENOMEM;
    }
    
    /* Register input handler */
    error = input_register_handler(&m720_handler);
    if (error) {
        printk(KERN_ERR MODULE_NAME ": Failed to register input handler: %d\n", error);
        destroy_virtual_keyboard(global_virtual_kbd);
        return error;
    }
    
    printk(KERN_INFO MODULE_NAME ": Module loaded successfully\n");
    return 0;
}

/*
 * Module cleanup
 */
static void __exit m720_remapper_exit(void)
{
    printk(KERN_INFO MODULE_NAME ": Unloading module\n");
    
    /* Unregister input handler */
    input_unregister_handler(&m720_handler);
    
    /* Destroy virtual keyboard */
    destroy_virtual_keyboard(global_virtual_kbd);
    global_virtual_kbd = NULL;
    
    printk(KERN_INFO MODULE_NAME ": Module unloaded (handled %d devices)\n", 
           device_count);
}

/* Module metadata */
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("GitHub Copilot");
MODULE_DESCRIPTION(MODULE_DESCRIPTION);
MODULE_VERSION(MODULE_VERSION);

module_init(m720_remapper_init);
module_exit(m720_remapper_exit);

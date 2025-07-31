#ifndef M720_REMAPPER_H
#define M720_REMAPPER_H

#include <linux/input.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/uinput.h>
#include <linux/usb.h>
#include <linux/hid.h>

/* Module information */
#define MODULE_NAME "m720_remapper"
#define MODULE_VERSION "1.0.0"
#define MODULE_DESCRIPTION "Logitech M720 Triathlon Button Remapper"

/* Logitech M720 identifiers */
#define LOGITECH_VENDOR_ID 0x046d
#define M720_PRODUCT_ID_1  0x405e  /* USB receiver */
#define M720_PRODUCT_ID_2  0xb015  /* Bluetooth */
#define M720_PRODUCT_ID_3  0xb013  /* Unifying receiver */

/* Button mappings */
#define M720_SIDE_BUTTON_1 BTN_SIDE
#define M720_SIDE_BUTTON_2 BTN_EXTRA
#define M720_FORWARD_BTN   BTN_FORWARD
#define M720_BACK_BTN      BTN_BACK

/* Key combinations to send */
#define WORKSPACE_UP_KEY1   KEY_LEFTMETA
#define WORKSPACE_UP_KEY2   KEY_PAGEUP
#define WORKSPACE_DOWN_KEY1 KEY_LEFTMETA
#define WORKSPACE_DOWN_KEY2 KEY_PAGEDOWN

/* Module parameters */
extern int debug_mode;
extern int remap_side_buttons;
extern int remap_extra_buttons;

/* Main structures */
struct m720_device {
    struct input_dev *input_dev;
    struct input_handle handle;
    struct input_dev *virtual_kbd;
    char name[128];
    char phys[128];
    bool enabled;
};

/* Function prototypes */
static int __init m720_remapper_init(void);
static void __exit m720_remapper_exit(void);
static int m720_connect(struct input_handler *handler, struct input_dev *dev,
                       const struct input_device_id *id);
static void m720_disconnect(struct input_handle *handle);
static bool m720_filter(struct input_handle *handle, unsigned int type,
                       unsigned int code, int value);
static bool m720_match(struct input_handler *handler, struct input_dev *dev);
static void m720_event(struct input_handle *handle, unsigned int type,
                      unsigned int code, int value);

/* Virtual keyboard functions */
static struct input_dev *create_virtual_keyboard(void);
static void destroy_virtual_keyboard(struct input_dev *virt_kbd);
static void send_key_combination(struct input_dev *virt_kbd, 
                                unsigned int key1, unsigned int key2);

/* Utility functions */
static bool is_m720_device(struct input_dev *dev);
static void print_device_info(struct input_dev *dev);

#endif /* M720_REMAPPER_H */

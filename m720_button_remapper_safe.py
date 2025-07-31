#!/usr/bin/env python3
"""
M720 Mouse Button Remapper for Wayland (Non-Grabbing Version)
This script monitors mouse button events without breaking normal mouse functionality
"""

import evdev
import subprocess
import asyncio
import sys
import os
import time
from selectors import DefaultSelector, EVENT_READ

def find_m720_device():
    """Find the M720 mouse device"""
    devices = [evdev.InputDevice(path) for path in evdev.list_devices()]
    
    for device in devices:
        if "M720" in device.name or "Logitech" in device.name:
            print(f"Found potential M720 device: {device.name}")
            print(f"Device path: {device.path}")
            
            # Check if it has mouse buttons
            capabilities = device.capabilities()
            if evdev.ecodes.EV_KEY in capabilities:
                keys = capabilities[evdev.ecodes.EV_KEY]
                # Check for common mouse button codes
                mouse_buttons = [
                    evdev.ecodes.BTN_LEFT, evdev.ecodes.BTN_RIGHT, evdev.ecodes.BTN_MIDDLE,
                    evdev.ecodes.BTN_SIDE, evdev.ecodes.BTN_EXTRA, evdev.ecodes.BTN_FORWARD, evdev.ecodes.BTN_BACK
                ]
                
                if any(btn in keys for btn in mouse_buttons):
                    print(f"This device has mouse buttons: {[evdev.ecodes.BTN[btn] for btn in keys if btn in mouse_buttons]}")
                    return device
    
    return None

def send_key_combo(keys):
    """Send keyboard combination using ydotool"""
    try:
        # Convert keys to ydotool format
        if keys == "super+page_up":
            subprocess.run(["ydotool", "key", "125:1", "104:1", "104:0", "125:0"], check=True)
        elif keys == "super+page_down":
            subprocess.run(["ydotool", "key", "125:1", "109:1", "109:0", "125:0"], check=True)
        return True
    except subprocess.CalledProcessError as e:
        print(f"Error sending key combo: {e}")
        return False
    except FileNotFoundError:
        print("ydotool not found. Please install it: sudo dnf install ydotool")
        return False

def handle_mouse_events(device):
    """Handle mouse button events without grabbing the device"""
    print(f"Monitoring {device.name} for button presses...")
    print("Button mapping:")
    print("- Side button (BTN_SIDE): Super + Page Down")
    print("- Extra button (BTN_EXTRA): Super + Page Up")
    print("- Forward button (BTN_FORWARD): Super + Page Up")
    print("- Back button (BTN_BACK): Super + Page Down")
    print("\nNOTE: Normal mouse functions (click, move, scroll) will continue to work")
    print("Press Ctrl+C to stop")
    print("-" * 50)
    
    # Use selector for non-blocking read
    selector = DefaultSelector()
    selector.register(device, EVENT_READ)
    
    try:
        while True:
            # Check for events with timeout
            events = selector.select(timeout=1)
            
            if events:
                try:
                    for event in device.read():
                        if event.type == evdev.ecodes.EV_KEY:
                            # Only handle button press events (value == 1), not release (value == 0)
                            if event.value == 1:
                                button_name = evdev.ecodes.BTN.get(event.code, f"BTN_{event.code}")
                                
                                # Only intercept specific buttons we want to remap
                                if event.code in [evdev.ecodes.BTN_SIDE, evdev.ecodes.BTN_EXTRA, 
                                                 evdev.ecodes.BTN_FORWARD, evdev.ecodes.BTN_BACK, 275, 276, 277, 278]:
                                    print(f"Button pressed: {button_name} (code: {event.code})")
                                    
                                    if event.code == evdev.ecodes.BTN_SIDE or event.code == evdev.ecodes.BTN_BACK:
                                        print("  → Sending Super + Page Down")
                                        send_key_combo("super+page_down")
                                        
                                    elif event.code == evdev.ecodes.BTN_EXTRA or event.code == evdev.ecodes.BTN_FORWARD:
                                        print("  → Sending Super + Page Up") 
                                        send_key_combo("super+page_up")
                                        
                                    # Handle additional button codes that might be used by M720
                                    elif event.code in [275, 276, 277, 278]:  # Common extra button codes
                                        if event.code in [275, 278]:  # Thumb buttons typically
                                            print("  → Sending Super + Page Down")
                                            send_key_combo("super+page_down")
                                        else:
                                            print("  → Sending Super + Page Up")
                                            send_key_combo("super+page_up")
                                
                except BlockingIOError:
                    # No events available right now
                    continue
                    
    except KeyboardInterrupt:
        print("\nStopping mouse button remapper...")
    finally:
        selector.unregister(device)
        selector.close()

def setup_ydotool():
    """Setup ydotool daemon if needed"""
    try:
        # Check if ydotoold is running
        result = subprocess.run(["pgrep", "ydotoold"], capture_output=True)
        if result.returncode != 0:
            print("Starting ydotool daemon...")
            subprocess.Popen(["sudo", "ydotoold"], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
            time.sleep(2)  # Give it time to start
        return True
    except Exception as e:
        print(f"Error setting up ydotool: {e}")
        return False

def main():
    """Main function"""
    print("M720 Mouse Button Remapper for Wayland (Non-Grabbing Version)")
    print("=" * 60)
    
    # Check if running as root (needed for input device access)
    if os.geteuid() != 0:
        print("This script needs to access input devices.")
        print("Please run with sudo:")
        print(f"sudo python3 {sys.argv[0]}")
        return
    
    # Setup ydotool
    if not setup_ydotool():
        print("Failed to setup ydotool")
        return
    
    # Find M720 device
    device = find_m720_device()
    if not device:
        print("M720 device not found!")
        print("\nAvailable devices:")
        devices = [evdev.InputDevice(path) for path in evdev.list_devices()]
        for i, dev in enumerate(devices):
            print(f"{i}: {dev.name} ({dev.path})")
        return
    
    try:
        print(f"\nUsing device: {device.name}")
        print("NOT grabbing device - normal mouse functions will continue to work")
        
        # Start event handling without grabbing
        handle_mouse_events(device)
        
    except PermissionError:
        print("Permission denied accessing input device")
        print("Make sure to run with sudo")
    except KeyboardInterrupt:
        print("\nStopping mouse button remapper...")
    except Exception as e:
        print(f"Error: {e}")

if __name__ == "__main__":
    main()

# Logitech M720 Triathlon Button Remapper for Linux Wayland

A complete solution for remapping the side buttons on Logitech M720 Triathlon mouse under Linux with Wayland (GNOME), specifically tested on Fedora.

## What This Does

- **Remaps M720 side buttons** to workspace switching (Super + Page Up/Down)
- **Preserves normal mouse functionality** - clicking, moving, scrolling all work perfectly
- **Works with Bluetooth M720** - no USB receiver required
- **Automatic startup** - runs automatically when you log in
- **Wayland compatible** - specifically designed for modern Linux desktops

## Tested Configuration

- **OS**: Fedora 42 (should work on other Linux distributions)
- **Desktop**: GNOME with Wayland
- **Mouse**: Logitech M720 Triathlon connected via Bluetooth
- **Connection**: Bluetooth (Device ID: CD:D5:09:F5:8B:E6)

## Quick Setup

### 1. Install Dependencies

```bash
# Install required packages
sudo dnf install python3-evdev ydotool

# For other distributions:
# Ubuntu/Debian: sudo apt install python3-evdev ydotool
# Arch: sudo pacman -S python-evdev ydotool
```

### 2. Download and Setup

```bash
# Clone this repository
git clone <your-repo-url>
cd logitech-m720-button-remapper

# Make scripts executable
chmod +x *.sh *.py

# Install autostart entry
mkdir -p ~/.config/autostart
cp m720-remapper.desktop ~/.config/autostart/
```

### 3. Start the Remapper

```bash
# Test it manually first
./manage_m720_manual.sh test

# Or start it in background
./manage_m720_manual.sh start
```

## Button Mapping

| Physical Button | Mapped Action | Description |
|----------------|---------------|-------------|
| Side Button 1 (Thumb) | Super + Page Down | Switch to workspace below |
| Side Button 2 (Extra) | Super + Page Up | Switch to workspace above |
| Left Click | Left Click | Normal function preserved |
| Right Click | Right Click | Normal function preserved |
| Middle Click | Middle Click | Normal function preserved |
| Scroll Wheel | Scroll | Normal function preserved |

## Files Included

### Core Scripts
- **`m720_button_remapper_safe.py`** - Main remapper script (safe version that doesn't break normal mouse functions)
- **`start_m720_remapper.sh`** - Startup script that handles ydotool daemon and launches remapper
- **`manage_m720_manual.sh`** - Management script for controlling the remapper

### Autostart
- **`m720-remapper.desktop`** - GNOME autostart entry for automatic startup on login

## Management Commands

Use the management script to control the remapper:

```bash
# Check current status
./manage_m720_manual.sh

# Start/stop the remapper
./manage_m720_manual.sh start
./manage_m720_manual.sh stop
./manage_m720_manual.sh restart

# Check if it's running
./manage_m720_manual.sh status

# Control autostart on login
./manage_m720_manual.sh enable-autostart
./manage_m720_manual.sh disable-autostart

# Test/debug mode (runs in foreground)
./manage_m720_manual.sh test
```

## How It Works

1. **Device Detection**: Finds your M720 mouse via evdev interface (`/dev/input/eventX`)
2. **Non-Grabbing Monitor**: Monitors button events without taking exclusive control
3. **Selective Remapping**: Only intercepts specific side buttons (BTN_SIDE, BTN_EXTRA, etc.)
4. **Key Simulation**: Uses ydotool to send Super+PageUp/PageDown key combinations
5. **Workspace Switching**: GNOME receives the key combinations and switches workspaces

## Troubleshooting

### Mouse Not Detected
```bash
# Check if mouse is connected
bluetoothctl info CD:D5:09:F5:8B:E6

# List available input devices
sudo libinput list-devices
```

### Buttons Not Working
```bash
# Test in debug mode
./manage_m720_manual.sh test

# Check if ydotool daemon is running
pgrep ydotoold

# Restart ydotool daemon
sudo pkill ydotoold
sudo ydotoold &
```

### Permission Issues
```bash
# The script needs root access to read input devices
# Make sure you're running with sudo when testing
sudo python3 m720_button_remapper_safe.py
```

### Autostart Not Working
```bash
# Check if autostart file exists
ls ~/.config/autostart/m720-remapper.desktop

# Re-enable autostart
./manage_m720_manual.sh enable-autostart
```

## Customization

### Change Button Mappings

Edit `m720_button_remapper_safe.py` and modify the key combinations in the `send_key_combo()` function:

```python
def send_key_combo(keys):
    if keys == "super+page_up":
        # Change these ydotool key codes for different actions
        subprocess.run(["ydotool", "key", "125:1", "104:1", "104:0", "125:0"], check=True)
```

### Common Key Combinations

| Action | ydotool Command |
|--------|----------------|
| Alt+Tab | `["ydotool", "key", "56:1", "15:1", "15:0", "56:0"]` |
| Ctrl+C | `["ydotool", "key", "29:1", "46:1", "46:0", "29:0"]` |
| Super+D | `["ydotool", "key", "125:1", "32:1", "32:0", "125:0"]` |

## Architecture

### Why This Approach?

1. **Non-Grabbing**: Unlike other solutions, this doesn't take exclusive control of the mouse
2. **Wayland Compatible**: Uses ydotool instead of X11-specific tools
3. **Selective**: Only intercepts the buttons you want to remap
4. **Reliable**: Autostart via GNOME instead of systemd (better desktop integration)

### Alternative Approaches Tried

- **imwheel**: Doesn't work with Wayland
- **xinput**: X11 only, doesn't work with Wayland
- **systemd service**: Permission and environment issues
- **Full device grabbing**: Breaks normal mouse functionality
- **This solution**: Non-grabbing evdev monitoring with ydotool

## System Requirements

- Linux with evdev support
- Python 3.6+
- Wayland display server
- GNOME desktop environment
- ydotool for key simulation
- python3-evdev for device monitoring
- Root access (for reading input devices)

## Contributing

Feel free to submit issues and enhancement requests!

### Known Limitations

- Requires root access for input device monitoring
- Wayland/GNOME specific (though could be adapted for other DEs)
- Button codes might vary between different M720 units

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Acknowledgments

- Based on the original logitech-m720-config by fin-ger
- Inspired by various Logitech mouse configuration tools
- Thanks to the evdev and ydotool developers

## Support

If you encounter issues:

1. Check the troubleshooting section above
2. Run in test mode: `./manage_m720_manual.sh test`
3. Check that your M720 is detected: `sudo libinput list-devices`
4. Ensure ydotool daemon is running: `pgrep ydotoold`

---

**Enjoy your perfectly configured M720 Triathlon!**

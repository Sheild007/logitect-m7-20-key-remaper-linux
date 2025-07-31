#!/bin/bash
# M720 Button Remapper Installation Script

set -e

echo "Logitech M720 Button Remapper Installation"
echo "=============================================="

# Check if running on supported system
if [[ "$XDG_SESSION_TYPE" != "wayland" ]]; then
    echo "Warning: This script is designed for Wayland. You're running: $XDG_SESSION_TYPE"
    read -p "Continue anyway? (y/N): " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        exit 1
    fi
fi

if [[ "$XDG_CURRENT_DESKTOP" != "GNOME" ]]; then
    echo "Warning: This script is designed for GNOME. You're running: $XDG_CURRENT_DESKTOP"
    read -p "Continue anyway? (y/N): " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        exit 1
    fi
fi

# Install dependencies
echo "Installing dependencies..."

if command -v dnf &> /dev/null; then
    sudo dnf install -y python3-evdev ydotool
elif command -v apt &> /dev/null; then
    sudo apt update
    sudo apt install -y python3-evdev ydotool
elif command -v pacman &> /dev/null; then
    sudo pacman -S --noconfirm python-evdev ydotool
else
    echo "Unsupported package manager. Please install python3-evdev and ydotool manually."
    exit 1
fi

# Make scripts executable
echo "Setting up scripts..."
chmod +x *.sh *.py

# Install autostart entry
echo "Setting up autostart..."
mkdir -p ~/.config/autostart
cp m720-remapper.desktop ~/.config/autostart/

# Start ydotool daemon
echo "Starting ydotool daemon..."
if ! pgrep ydotoold > /dev/null; then
    sudo ydotoold &
    sleep 2
fi

echo ""
echo "Installation complete!"
echo ""
echo "To test your M720 configuration:"
echo "   ./manage_m720_manual.sh test"
echo ""
echo "To start the remapper in background:"
echo "   ./manage_m720_manual.sh start"
echo ""
echo "To check status anytime:"
echo "   ./manage_m720_manual.sh"
echo ""
echo "The remapper will automatically start on your next login!"
echo ""
echo "Happy workspace switching!"

#!/bin/bash
# M720 Kernel Module Installation Script

set -e

MODULE_NAME="m720_remapper"
MODULE_VERSION="1.0"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

echo "M720 Remapper Kernel Module Installer"
echo "====================================="

# Check if running as root for installation
if [[ $EUID -eq 0 ]]; then
    echo "Warning: Running as root. This will install the module system-wide."
    INSTALL_MODE="system"
else
    echo "Running as user. Module will be loaded temporarily."
    INSTALL_MODE="temporary"
fi

# Check for required tools
echo "Checking dependencies..."

if ! command -v make &> /dev/null; then
    echo "Error: 'make' is required but not installed."
    echo "Install with: sudo apt install build-essential"
    exit 1
fi

if [[ ! -d "/lib/modules/$(uname -r)/build" ]]; then
    echo "Error: Kernel headers not found for kernel $(uname -r)"
    echo "Install with: sudo apt install linux-headers-$(uname -r)"
    exit 1
fi

# Check if module is already loaded
if lsmod | grep -q "$MODULE_NAME"; then
    echo "Module $MODULE_NAME is already loaded. Unloading..."
    sudo rmmod "$MODULE_NAME" || {
        echo "Warning: Could not unload existing module"
    }
fi

# Build the module
echo "Building kernel module..."
cd "$SCRIPT_DIR"
make clean
make all

if [[ ! -f "${MODULE_NAME}.ko" ]]; then
    echo "Error: Module build failed"
    exit 1
fi

echo "Module built successfully: ${MODULE_NAME}.ko"

# Show module info
echo ""
echo "Module Information:"
modinfo "${MODULE_NAME}.ko"
echo ""

# Installation options
if [[ "$INSTALL_MODE" == "system" ]]; then
    echo "Choose installation method:"
    echo "1) Temporary load (unloads on reboot)"
    echo "2) DKMS install (survives kernel updates)"
    echo "3) Manual install (copy to modules directory)"
    read -p "Enter choice [1-3]: " choice
    
    case $choice in
        1)
            echo "Loading module temporarily..."
            insmod "${MODULE_NAME}.ko" debug_mode=1
            ;;
        2)
            echo "Installing with DKMS..."
            make dkms-install
            modprobe "$MODULE_NAME" debug_mode=1
            ;;
        3)
            echo "Installing manually..."
            cp "${MODULE_NAME}.ko" "/lib/modules/$(uname -r)/kernel/drivers/input/"
            depmod -a
            modprobe "$MODULE_NAME" debug_mode=1
            ;;
        *)
            echo "Invalid choice. Loading temporarily..."
            insmod "${MODULE_NAME}.ko" debug_mode=1
            ;;
    esac
else
    # Non-root installation
    echo "Loading module temporarily (requires sudo)..."
    sudo insmod "${MODULE_NAME}.ko" debug_mode=1
fi

# Check if module loaded successfully
if lsmod | grep -q "$MODULE_NAME"; then
    echo ""
    echo "Success! Module loaded successfully."
    echo ""
    echo "Module Status:"
    lsmod | grep "$MODULE_NAME"
    echo ""
    
    echo "Module Parameters:"
    if [[ -d "/sys/module/$MODULE_NAME/parameters" ]]; then
        for param in /sys/module/$MODULE_NAME/parameters/*; do
            if [[ -r "$param" ]]; then
                param_name=$(basename "$param")
                param_value=$(cat "$param" 2>/dev/null || echo "N/A")
                echo "  $param_name: $param_value"
            fi
        done
    fi
    
    echo ""
    echo "Testing Instructions:"
    echo "1. Connect your Logitech M720 mouse"
    echo "2. Try pressing the side buttons"
    echo "3. Check kernel messages: dmesg | tail -10"
    echo "4. Monitor module output: make dmesg"
    echo ""
    echo "To unload: sudo rmmod $MODULE_NAME"
    echo "To reload: make reload"
    
else
    echo "Error: Module failed to load"
    echo "Check kernel messages: dmesg | tail -10"
    exit 1
fi

echo ""
echo "Installation complete!"
echo ""
echo "Available commands:"
echo "  make status     - Check module status"
echo "  make dmesg      - Show kernel messages"
echo "  make debug-on   - Enable debug output"
echo "  make debug-off  - Disable debug output"
echo "  make uninstall  - Unload module"
echo "  make help       - Show all available commands"

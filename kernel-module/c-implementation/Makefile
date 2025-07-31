# Makefile for M720 Remapper Kernel Module

# Module name
MODULE_NAME := m720_remapper

# Object files
obj-m := $(MODULE_NAME).o

# Kernel build directory
KERNEL_DIR := /lib/modules/$(shell uname -r)/build

# Current directory
PWD := $(shell pwd)

# Compiler flags
ccflags-y := -DDEBUG

# Default target
all:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) modules

# Clean target
clean:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) clean
	rm -f modules.order Module.symvers

# Install target
install: all
	sudo insmod $(MODULE_NAME).ko

# Uninstall target
uninstall:
	sudo rmmod $(MODULE_NAME) || true

# Reload target
reload: uninstall install

# Show module info
info:
	modinfo $(MODULE_NAME).ko

# Show loaded module status
status:
	lsmod | grep $(MODULE_NAME) || echo "Module not loaded"

# Show kernel log messages
dmesg:
	dmesg | tail -20 | grep $(MODULE_NAME) || echo "No recent messages"

# Enable debug mode
debug-on:
	sudo bash -c 'echo 1 > /sys/module/$(MODULE_NAME)/parameters/debug_mode' || true

# Disable debug mode
debug-off:
	sudo bash -c 'echo 0 > /sys/module/$(MODULE_NAME)/parameters/debug_mode' || true

# Show current parameters
params:
	@echo "Current module parameters:"
	@cat /sys/module/$(MODULE_NAME)/parameters/debug_mode 2>/dev/null | sed 's/^/  debug_mode: /' || echo "  Module not loaded"
	@cat /sys/module/$(MODULE_NAME)/parameters/remap_side_buttons 2>/dev/null | sed 's/^/  remap_side_buttons: /' || true
	@cat /sys/module/$(MODULE_NAME)/parameters/remap_extra_buttons 2>/dev/null | sed 's/^/  remap_extra_buttons: /' || true

# Test target - builds and loads module with debug enabled
test: reload debug-on
	@echo "Module loaded with debug enabled. Check 'make dmesg' for output."
	@echo "Test your M720 mouse buttons now."
	@echo "Run 'make uninstall' when done testing."

# DKMS install
dkms-install:
	sudo cp -r . /usr/src/$(MODULE_NAME)-1.0/
	sudo dkms add -m $(MODULE_NAME) -v 1.0
	sudo dkms build -m $(MODULE_NAME) -v 1.0
	sudo dkms install -m $(MODULE_NAME) -v 1.0

# DKMS uninstall
dkms-uninstall:
	sudo dkms remove -m $(MODULE_NAME) -v 1.0 --all || true
	sudo rm -rf /usr/src/$(MODULE_NAME)-1.0/ || true

# Help target
help:
	@echo "Available targets:"
	@echo "  all          - Build the kernel module"
	@echo "  clean        - Clean build files"
	@echo "  install      - Load the module"
	@echo "  uninstall    - Unload the module"
	@echo "  reload       - Unload and reload the module"
	@echo "  test         - Build, load, and enable debug mode"
	@echo "  info         - Show module information"
	@echo "  status       - Show if module is loaded"
	@echo "  dmesg        - Show recent kernel log messages"
	@echo "  debug-on     - Enable debug mode"
	@echo "  debug-off    - Disable debug mode"
	@echo "  params       - Show current module parameters"
	@echo "  dkms-install - Install using DKMS (survives kernel updates)"
	@echo "  dkms-uninstall - Uninstall DKMS version"
	@echo "  help         - Show this help"

.PHONY: all clean install uninstall reload info status dmesg debug-on debug-off params test dkms-install dkms-uninstall help

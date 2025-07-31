# Hybrid Approach: Kernel Module + Userspace Configuration

This directory contains a hybrid implementation that combines:
- **Kernel module** for low-latency event handling
- **Userspace daemon** for configuration and advanced features

## Architecture

```
┌─────────────────┐    ┌───────────────────┐    ┌─────────────────┐
│   M720 Mouse    │────│  Kernel Module    │────│  Virtual Input  │
│                 │    │  (Fast Path)      │    │  Device         │
└─────────────────┘    └───────────────────┘    └─────────────────┘
                               │
                               │ (Netlink/sysfs)
                               │
                       ┌───────────────────┐
                       │  Userspace Daemon │
                       │  (Configuration)  │
                       └───────────────────┘
                               │
                       ┌───────────────────┐
                       │  GUI/CLI Config   │
                       │  Tools            │
                       └───────────────────┘
```

## Benefits

1. **Low Latency**: Kernel module handles basic remapping
2. **Flexibility**: Userspace daemon provides advanced configuration
3. **User-Friendly**: GUI tools for easy setup
4. **Extensible**: Easy to add new features without kernel changes

## Components

### 1. Kernel Module (`m720_hybrid.ko`)
- Basic button remapping
- Configurable via sysfs
- Minimal latency overhead
- Falls back to userspace for complex operations

### 2. Userspace Daemon (`m720d`)
- Monitors for new M720 devices
- Applies user configurations
- Handles complex mappings (gestures, application-specific)
- Provides D-Bus interface

### 3. Configuration Tools
- CLI tool: `m720-config`
- GUI tool: `m720-gui` (GTK-based)
- Configuration files in `/etc/m720/`

### 4. Integration
- systemd service for daemon
- DKMS for kernel module
- Desktop integration

## Files

- `kernel/` - Hybrid kernel module
- `daemon/` - Userspace daemon (C)
- `config-tools/` - CLI and GUI configuration tools
- `install.sh` - Complete installation script
- `systemd/` - Service files
- `dbus/` - D-Bus interface definitions

## Installation

```bash
# Build and install everything
sudo ./install.sh

# Or install components separately
cd kernel && make install
cd daemon && make install
cd config-tools && make install
```

## Usage

```bash
# Check status
systemctl status m720d

# Configure button mapping
m720-config --side-button-1 "workspace-down"
m720-config --side-button-2 "workspace-up"

# Advanced gestures
m720-config --gesture "side1+side2" "show-overview"

# GUI configuration
m720-gui
```

This hybrid approach provides the best of both worlds: kernel-level performance with userspace flexibility.

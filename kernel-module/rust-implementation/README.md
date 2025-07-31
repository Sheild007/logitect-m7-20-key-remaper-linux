# Rust Kernel Module for M720 Remapper

This directory contains a Rust implementation of the M720 button remapper kernel module.

## Status: Experimental

**Note**: Rust support in the Linux kernel is still experimental and only available in:
- Linux kernel 6.1+ with Rust support enabled
- Requires Rust toolchain with kernel-specific components
- Limited API coverage compared to C kernel modules

## Requirements

1. **Kernel with Rust Support**:
   ```bash
   # Check if your kernel has Rust support
   grep CONFIG_RUST /boot/config-$(uname -r)
   # Should show: CONFIG_RUST=y
   ```

2. **Rust Toolchain**:
   ```bash
   # Install Rust kernel toolchain
   rustup toolchain install nightly
   rustup component add rust-src --toolchain nightly
   ```

3. **Kernel Headers with Rust**:
   ```bash
   # Fedora/RHEL
   sudo dnf install kernel-devel rust
   
   # Ubuntu (if available)
   sudo apt install linux-headers-$(uname -r) rustc
   ```

## Current Implementation Status

- 🚧 **In Development**: Basic structure created
- ❌ **Not Ready**: Rust kernel input APIs are limited
- 🔄 **Alternative**: Use C implementation for now

## Future Plans

When Rust kernel support matures:
1. Memory-safe device handling
2. Zero-cost abstractions
3. Better error handling
4. Type-safe event processing

## Recommendation

**Use the C implementation** in `../c-implementation/` for production use.
The Rust version will be updated as kernel Rust support improves.

## Files

- `Cargo.toml` - Rust project configuration
- `src/lib.rs` - Main module implementation (experimental)
- `Makefile` - Build configuration
- `README.md` - This file

## Building (When Ready)

```bash
# When Rust kernel support is available
make LLVM=1 RUST_ENABLED=1
```

For now, the C implementation provides full functionality and better kernel compatibility.

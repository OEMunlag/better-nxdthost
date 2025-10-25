# Developer Notes

## Conversion from Python to C++/Qt6

### Architecture Changes

#### Threading Model
- **Python**: Used Python's `threading` module with manual event synchronization
- **C++**: Uses Qt's `QThread` class for background USB operations
- The USB manager runs in a separate thread to prevent GUI blocking

#### GUI Framework
- **Python**: Tkinter
- **C++**: Qt6 Widgets
- Qt provides better cross-platform consistency and modern UI elements

#### USB Library
- **Python**: PyUSB wrapper
- **C++**: Direct libusb-1.0 C API
- More control over USB operations but requires manual memory management

### Key Implementation Differences

#### Progress Tracking
- Python used tqdm for terminal progress bars
- C++ implements a custom `ProgressDialog` with:
  - Real-time speed calculation
  - Elapsed/remaining time estimation
  - Percentage and size display
  - Smooth updates (500ms intervals to prevent flicker)

#### Logging
- Python used Python's logging module with custom queue handler
- C++ uses Qt signals/slots for thread-safe logging
- Four log levels: Debug (gray), Info (black), Warning (orange), Error (red)

#### File I/O
- Python used built-in file objects
- C++ uses Qt's `QFile` for cross-platform file handling
- Better integration with Qt's path handling

### Protocol Implementation

The USB protocol is identical to the Python version:

```
Command Header (16 bytes):
  - Magic: "NXDT" (4 bytes)
  - Command ID: uint32
  - Block Size: uint32
  - Reserved: 4 bytes

Status Response (16 bytes):
  - Magic: "NXDT" (4 bytes)
  - Status: uint32
  - Max Packet Size: uint16
  - Reserved: 6 bytes
```

### Memory Management

#### Critical Points
1. **USB Buffers**: All USB read/write operations use QByteArray for automatic memory management
2. **NSP Files**: QFile* stored in member variable, explicitly deleted in destructor
3. **Thread Safety**: All communication between threads uses Qt signals (queued connections)

#### Potential Issues
- Ensure `resetNspInfo()` is called properly to avoid memory leaks
- USB device handle must be closed before context destruction
- Progress dialog must be hidden before deletion

### Building on Different Platforms

#### Windows
- Requires Visual Studio or MinGW
- Qt6 should be built with same compiler
- libusb-1.0 can be installed via vcpkg: `vcpkg install libusb`
- Alternative: Download prebuilt libusb from official website

#### macOS  
- Use Homebrew for all dependencies
- Qt6 from Homebrew: `brew install qt@6`
- May need to set CMAKE_PREFIX_PATH: `export CMAKE_PREFIX_PATH=$(brew --prefix qt@6)`

#### Linux
- Each distribution has different package names for Qt6
- Ubuntu 22.04+ has qt6-base-dev in repositories
- Older versions may need PPA or manual Qt6 installation

### Future Improvements

#### Possible Enhancements
1. **CLI Mode**: Add command-line only mode (like Python version)
2. **Settings Dialog**: Persistent output directory, theme selection
3. **Transfer History**: Log of all successful transfers
4. **Dark Mode**: Qt6 supports native dark mode on all platforms
5. **Drag & Drop**: Allow dragging output folder to window
6. **Tray Icon**: Minimize to system tray option
7. **Auto-reconnect**: Automatically detect device reconnection

#### Code Optimizations
1. Use `QThreadPool` for potentially concurrent operations
2. Implement zero-copy transfers where possible
3. Add transfer verification (checksums)
4. Implement resume capability for interrupted transfers

### Testing Checklist

- [ ] File transfer < 4MB (no progress bar)
- [ ] File transfer > 4MB (with progress bar)
- [ ] NSP file with header
- [ ] Multiple NSP entries
- [ ] Extracted FS dump
- [ ] Transfer cancellation from Switch
- [ ] Device disconnection during transfer
- [ ] Invalid output directory
- [ ] Insufficient disk space
- [ ] Verbose mode logging
- [ ] Window close during transfer

### Common Issues During Development

#### Issue: "undefined reference to Qt6::XXX"
**Solution**: Ensure all required Qt modules are linked in CMakeLists.txt

#### Issue: "libusb_bulk_transfer returns -7 (LIBUSB_ERROR_TIMEOUT)"
**Solution**: Check USB timeout values, ensure device is responding

#### Issue: Progress dialog freezes UI
**Solution**: Ensure `processEvents()` or use proper threading

#### Issue: Files corrupted after transfer
**Solution**: Check for proper ZLT (Zero Length Termination) packet handling

### Debugging Tips

1. **Enable Verbose Mode**: Shows all USB commands and data sizes
2. **USB Analyzer**: Use Wireshark with USBPcap (Windows) or usbmon (Linux)
3. **Qt Creator**: Excellent debugger integration for Qt applications
4. **Valgrind**: Memory leak detection on Linux
5. **AddressSanitizer**: Compile with `-fsanitize=address` for memory error detection

### Code Style

- Use Qt coding conventions
- camelCase for member functions
- m_prefix for member variables
- Use Qt containers (QString, QByteArray) over STL when interfacing with Qt APIs
- Prefer Qt signals/slots over callbacks
- Use nullptr instead of NULL

### License Compliance

This port maintains GPL v3 licensing as per the original nxdumptool project:
- All source files include original copyright header
- Changes are documented
- GPL v3 text should be included in distribution

## Contributing

When contributing to this C++ port:
1. Follow existing code style
2. Test on multiple platforms if possible
3. Update documentation for new features
4. Ensure thread safety for any Qt signal/slot additions
5. Add logging for significant operations (use appropriate log level)

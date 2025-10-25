# nxdumptool Host (C++ Qt6 Version)

A C++ Qt6 port of the nxdumptool Python host script. This application provides a graphical interface for receiving files from a Nintendo Switch running nxdumptool.

## Features

- Modern Qt6 GUI interface
- Real-time file transfer progress with speed indicators
- Verbose logging option
- Optional free space check for target storage
- NSP file support with header handling
- Extracted filesystem dump support
- Cross-platform (Windows, macOS, Linux)

## Requirements

### Build Requirements
- CMake 3.16 or later
- Qt6 (Core and Widgets modules)
- libusb-1.0
- C++17 compatible compiler

### Runtime Requirements
- libusb-1.0 library installed
- **Windows**: libusbK driver installed via Zadig
- **macOS**: libusb installed via Homebrew (`brew install libusb`)
- **Linux**: libusb-1.0 from your package manager

## Building

### Linux/macOS

```bash
# Install dependencies (Ubuntu/Debian)
sudo apt install build-essential cmake qt6-base-dev libusb-1.0-0-dev

# Install dependencies (macOS with Homebrew)
brew install cmake qt@6 libusb

# Build
mkdir build
cd build
cmake ..
make -j$(nproc)

# Run
./nxdumptool_host
```

### Windows

```bash
# Install Qt6 and libusb using vcpkg or download binaries
# Then configure CMake with proper paths

mkdir build
cd build
cmake -DCMAKE_PREFIX_PATH="C:/Qt/6.x.x/msvc2019_64" ..
cmake --build . --config Release

# Run
Release\nxdumptool_host.exe
```

## Usage

1. Launch the application
2. Choose an output directory where files will be saved
3. Click "Start Server"
4. Connect your Nintendo Switch running nxdumptool via USB
5. Use nxdumptool on your Switch to dump files
6. Files will be transferred and saved automatically

### Command-line Options

The application supports several command-line switches that let you control its
behaviour when launching from a terminal:

- `-o, --outdir <DIR>` – start with the specified output directory selected.
- `-V, --verbose` – enable verbose logging so that debug-level messages are displayed in the log window.
- `-F, --no-free-space-check` – disable the free space validation performed
  before each transfer. This is useful when the host system cannot correctly
  detect the available space.

### Verbose Mode
Enable the "Verbose output" checkbox to see detailed debug information including:
- USB command details
- Transfer block sizes
- Internal state changes

## File Structure

```
nxdumptool_host/
├── CMakeLists.txt           # Build configuration
├── main.cpp                 # Application entry point
├── mainwindow.h/cpp        # Main window GUI
├── usbmanager.h/cpp        # USB communication handler
├── progressdialog.h/cpp    # Progress dialog for transfers
├── usbcommands.h           # USB protocol constants
└── README.md               # This file
```

## Protocol Support

This application implements the nxdumptool USB protocol version 1.2:
- Start/End Session commands
- File property negotiation
- Streaming file transfers with progress tracking
- NSP header handling
- Extracted filesystem dumps
- Transfer cancellation

## Troubleshooting

### Windows
- **"Failed to initialize libusb"**: Install the libusbK driver using Zadig
  1. Download Zadig from https://zadig.akeo.ie
  2. Connect your Switch running nxdumptool
  3. Select the Nintendo Switch device in Zadig
  4. Choose libusbK driver and click "Install Driver"

### macOS
- **"Failed to initialize libusb"**: Install libusb via Homebrew
  ```bash
  brew install libusb
  ```

### Linux
- **Permission denied**: Add udev rules for the Switch device
  ```bash
  # Create /etc/udev/rules.d/99-switch.rules with:
  SUBSYSTEM=="usb", ATTRS{idVendor}=="057e", ATTRS{idProduct}=="3000", MODE="0666"
  
  # Reload rules:
  sudo udevadm control --reload-rules
  sudo udevadm trigger
  ```

## Known Limitations

- The application must be kept running during transfers
- Large file transfers may take significant time depending on USB speed
- Cancel operations must be initiated from the Switch console

## Credits

Original Python script by DarkMatterCore
C++ Qt6 port conversion

## License

GPLv3 - Same as the original nxdumptool project

Copyright (c) 2020-2024, DarkMatterCore <pabloacurielz@gmail.com>

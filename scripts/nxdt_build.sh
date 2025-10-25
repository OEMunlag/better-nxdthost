#!/bin/bash

# Build script for nxdumptool host (Linux/macOS)

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}nxdumptool Host Build Script${NC}"
echo "================================"

# Check for required tools
echo -e "\n${YELLOW}Checking dependencies...${NC}"

if ! command -v cmake &> /dev/null; then
    echo -e "${RED}Error: CMake is not installed${NC}"
    exit 1
fi

if ! command -v qmake6 &> /dev/null && ! command -v qmake &> /dev/null; then
    echo -e "${RED}Error: Qt6 is not installed${NC}"
    exit 1
fi

if ! pkg-config --exists libusb-1.0; then
    echo -e "${RED}Error: libusb-1.0 is not installed${NC}"
    echo "Install it with:"
    if [[ "$OSTYPE" == "linux-gnu"* ]]; then
        echo "  Ubuntu/Debian: sudo apt install libusb-1.0-0-dev"
        echo "  Fedora: sudo dnf install libusb-devel"
        echo "  Arch: sudo pacman -S libusb"
    elif [[ "$OSTYPE" == "darwin"* ]]; then
        echo "  macOS: brew install libusb"
    fi
    exit 1
fi

echo -e "${GREEN}All dependencies found!${NC}"

# Create build directory
BUILD_DIR="build"
if [ -d "$BUILD_DIR" ]; then
    echo -e "\n${YELLOW}Cleaning existing build directory...${NC}"
    rm -rf "$BUILD_DIR"
fi

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure
echo -e "\n${YELLOW}Configuring project...${NC}"
cmake .. || {
    echo -e "${RED}CMake configuration failed${NC}"
    exit 1
}

# Build
echo -e "\n${YELLOW}Building project...${NC}"
NPROC=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
make -j${NPROC} || {
    echo -e "${RED}Build failed${NC}"
    exit 1
}

echo -e "\n${GREEN}Build completed successfully!${NC}"
echo -e "Executable: ${GREEN}${BUILD_DIR}/nxdumptool_host${NC}"
echo ""
echo "To run:"
echo "  cd ${BUILD_DIR}"
echo "  ./nxdumptool_host"
echo ""
echo "To install (optional):"
echo "  sudo make install"

#!/bin/bash

# Installation script for nxdumptool host (Linux)

set -e

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

echo -e "${GREEN}nxdumptool Host Installation Script${NC}"
echo "===================================="

# Check if running as root
if [ "$EUID" -ne 0 ]; then 
    echo -e "${YELLOW}This script needs sudo privileges for some operations.${NC}"
    echo "You may be prompted for your password."
    echo ""
fi

# Detect distribution
if [ -f /etc/os-release ]; then
    . /etc/os-release
    OS=$ID
    VER=$VERSION_ID
else
    echo -e "${RED}Cannot detect Linux distribution${NC}"
    exit 1
fi

echo -e "${BLUE}Detected OS: $OS${NC}"
echo ""

# Install dependencies based on distribution
echo -e "${YELLOW}Installing dependencies...${NC}"

case "$OS" in
    ubuntu|debian|linuxmint)
        sudo apt update
        sudo apt install -y build-essential cmake qt6-base-dev libusb-1.0-0-dev
        ;;
    fedora)
        sudo dnf install -y gcc-c++ cmake qt6-qtbase-devel libusb-devel
        ;;
    arch|manjaro)
        sudo pacman -Sy --noconfirm base-devel cmake qt6-base libusb
        ;;
    opensuse*)
        sudo zypper install -y gcc-c++ cmake qt6-base-devel libusb-1_0-devel
        ;;
    *)
        echo -e "${YELLOW}Unknown distribution. Please install manually:${NC}"
        echo "  - build-essential / gcc-c++"
        echo "  - cmake"
        echo "  - qt6-base-dev / qt6-qtbase-devel"
        echo "  - libusb-1.0-0-dev / libusb-devel"
        exit 1
        ;;
esac

echo -e "${GREEN}Dependencies installed!${NC}"
echo ""

# Install udev rules
echo -e "${YELLOW}Installing udev rules...${NC}"

if [ -f "99-switch.rules" ]; then
    sudo cp 99-switch.rules /etc/udev/rules.d/
    sudo udevadm control --reload-rules
    sudo udevadm trigger
    echo -e "${GREEN}udev rules installed!${NC}"
else
    echo -e "${YELLOW}Warning: 99-switch.rules not found. You may need to set up USB permissions manually.${NC}"
fi

echo ""

# Add user to plugdev group (if it exists)
if getent group plugdev > /dev/null 2>&1; then
    echo -e "${YELLOW}Adding user to plugdev group...${NC}"
    sudo usermod -a -G plugdev $USER
    echo -e "${GREEN}Done! You may need to log out and back in for group changes to take effect.${NC}"
fi

echo ""

# Build the application
if [ ! -f "build.sh" ]; then
    echo -e "${RED}Error: build.sh not found${NC}"
    exit 1
fi

echo -e "${YELLOW}Building application...${NC}"
bash build.sh

echo ""
echo -e "${GREEN}Installation complete!${NC}"
echo ""
echo "To run the application:"
echo -e "  ${BLUE}cd build && ./nxdumptool_host${NC}"
echo ""
echo "Or install system-wide:"
echo -e "  ${BLUE}cd build && sudo make install${NC}"
echo ""
echo -e "${YELLOW}Note: If you were added to the plugdev group, log out and back in for changes to take effect.${NC}"

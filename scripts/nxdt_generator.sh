#!/bin/bash

# This script will help you download all the source files
# Save this script and run it to create the project structure

echo "nxdumptool Host Project Generator"
echo "=================================="
echo ""
echo "This project was generated from a conversation with Claude."
echo "You'll find all source files as artifacts in the conversation."
echo ""
echo "To get the files, you have two options:"
echo ""
echo "Option 1: Manual Copy (Recommended)"
echo "-----------------------------------"
echo "1. Scroll up in the conversation"
echo "2. Find each artifact/code block (they have titles like 'CMakeLists.txt', 'main.cpp', etc.)"
echo "3. Click the copy button on each artifact"
echo "4. Save each to a file with the corresponding name"
echo ""
echo "Option 2: Use This Guide"
echo "------------------------"
echo "Create a directory structure and copy each file:"
echo ""

cat << 'EOF'
mkdir -p nxdumptool_host
cd nxdumptool_host

# Create each file by copying from the artifacts above:

# 1. CMakeLists.txt - Build configuration
# 2. usbcommands.h - USB protocol constants
# 3. usbmanager.h - USB manager header
# 4. usbmanager.cpp - USB manager implementation
# 5. progressdialog.h - Progress dialog header
# 6. progressdialog.cpp - Progress dialog implementation
# 7. mainwindow.h - Main window header
# 8. mainwindow.cpp - Main window implementation
# 9. main.cpp - Application entry point
# 10. README.md - Documentation
# 11. DEVELOPER_NOTES.md - Developer documentation
# 12. build.sh - Build script
# 13. install.sh - Installation script (Linux)
# 14. 99-switch.rules - udev rules (Linux)
# 15. Makefile - Makefile wrapper
# 16. .gitignore - Git ignore file

EOF

echo ""
echo "File List (in recommended creation order):"
echo "==========================================="
echo ""
echo "Core Headers:"
echo "  usbcommands.h"
echo "  usbmanager.h"
echo "  progressdialog.h"
echo "  mainwindow.h"
echo ""
echo "Implementation Files:"
echo "  usbmanager.cpp"
echo "  progressdialog.cpp"
echo "  mainwindow.cpp"
echo "  main.cpp"
echo ""
echo "Build System:"
echo "  CMakeLists.txt"
echo "  Makefile"
echo "  build.sh (chmod +x after creating)"
echo "  install.sh (chmod +x after creating)"
echo ""
echo "Linux Support:"
echo "  99-switch.rules"
echo ""
echo "Documentation:"
echo "  README.md"
echo "  DEVELOPER_NOTES.md"
echo "  .gitignore"
echo ""
echo "After creating all files, build with:"
echo "  ./build.sh"
echo ""
echo "Or manually:"
echo "  mkdir build && cd build"
echo "  cmake .."
echo "  make"
echo ""

# Create directory structure
mkdir -p nxdumptool_host
cd nxdumptool_host

echo "✓ Created directory: nxdumptool_host"
echo ""
echo "Now copy each artifact from the conversation into the corresponding file."
echo "Look for artifacts with these exact titles:"
echo ""
echo "  - 'nxdumptool Host - CMakeLists.txt'"
echo "  - 'nxdumptool Host - usbcommands.h'"
echo "  - 'nxdumptool Host - usbmanager.h'"
echo "  - 'nxdumptool Host - usbmanager.cpp'"
echo "  - 'nxdumptool Host - progressdialog.h'"
echo "  - 'nxdumptool Host - progressdialog.cpp'"
echo "  - 'nxdumptool Host - mainwindow.h'"
echo "  - 'nxdumptool Host - mainwindow.cpp'"
echo "  - 'nxdumptool Host - main.cpp'"
echo "  - 'nxdumptool Host - README.md'"
echo "  - 'nxdumptool Host - DEVELOPER_NOTES.md'"
echo "  - 'nxdumptool Host - build.sh'"
echo "  - 'nxdumptool Host - install.sh'"
echo "  - 'nxdumptool Host - 99-switch.rules'"
echo "  - 'nxdumptool Host - Makefile'"
echo "  - 'nxdumptool Host - .gitignore'"

# Simple Makefile wrapper for CMake
# This provides convenient make targets for common operations

.PHONY: all build clean install uninstall run help

BUILD_DIR = build
TARGET = $(BUILD_DIR)/nxdumptool_host

# Default target
all: build

# Build the project
build:
	@echo "Building nxdumptool host..."
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake .. && $(MAKE)
	@echo "Build complete! Executable: $(TARGET)"

# Clean build artifacts
clean:
	@echo "Cleaning build directory..."
	@rm -rf $(BUILD_DIR)
	@echo "Clean complete!"

# Install system-wide (requires sudo)
install: build
	@echo "Installing..."
	@cd $(BUILD_DIR) && sudo $(MAKE) install
	@echo "Installation complete!"

# Uninstall
uninstall:
	@echo "Uninstalling..."
	@cd $(BUILD_DIR) && sudo $(MAKE) uninstall 2>/dev/null || echo "Nothing to uninstall"

# Run the application
run: build
	@echo "Running nxdumptool host..."
	@$(TARGET)

# Install udev rules (Linux only)
install-udev:
	@if [ -f "99-switch.rules" ]; then \
		echo "Installing udev rules..."; \
		sudo cp 99-switch.rules /etc/udev/rules.d/; \
		sudo udevadm control --reload-rules; \
		sudo udevadm trigger; \
		echo "udev rules installed!"; \
	else \
		echo "Error: 99-switch.rules not found"; \
		exit 1; \
	fi

# Rebuild from scratch
rebuild: clean build

# Show help
help:
	@echo "nxdumptool Host Makefile"
	@echo "========================"
	@echo ""
	@echo "Available targets:"
	@echo "  make              - Build the project (default)"
	@echo "  make build        - Build the project"
	@echo "  make clean        - Remove build artifacts"
	@echo "  make rebuild      - Clean and rebuild"
	@echo "  make install      - Install system-wide (requires sudo)"
	@echo "  make uninstall    - Uninstall from system"
	@echo "  make run          - Build and run the application"
	@echo "  make install-udev - Install udev rules (Linux only)"
	@echo "  make help         - Show this help message"
	@echo ""
	@echo "Example workflow:"
	@echo "  make              # Build"
	@echo "  make run          # Build and run"
	@echo "  make install-udev # Set up USB permissions (Linux)"
	@echo "  make install      # Install system-wide"

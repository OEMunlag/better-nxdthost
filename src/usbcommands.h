#ifndef USBCOMMANDS_H
#define USBCOMMANDS_H

#include <cstdint>

// USB VID/PID
constexpr uint16_t USB_DEV_VID = 0x057E;
constexpr uint16_t USB_DEV_PID = 0x3000;

// USB strings
constexpr const char* USB_DEV_MANUFACTURER = "DarkMatterCore";
constexpr const char* USB_DEV_PRODUCT = "nxdumptool";

// USB timeout (milliseconds)
constexpr int USB_TRANSFER_TIMEOUT = 10000;

// USB transfer block size
constexpr size_t USB_TRANSFER_BLOCK_SIZE = 0x800000;

// USB transfer threshold
constexpr size_t USB_TRANSFER_THRESHOLD = (USB_TRANSFER_BLOCK_SIZE * 4);

// USB magic word
constexpr uint8_t USB_MAGIC_WORD[4] = {'N', 'X', 'D', 'T'};

// Supported USB ABI version
constexpr uint8_t USB_ABI_VERSION_MAJOR = 1;
constexpr uint8_t USB_ABI_VERSION_MINOR = 2;

// USB command header size
constexpr size_t USB_CMD_HEADER_SIZE = 0x10;

// USB command IDs
enum UsbCommandId : uint32_t {
    USB_CMD_START_SESSION = 0,
    USB_CMD_SEND_FILE_PROPERTIES = 1,
    USB_CMD_CANCEL_FILE_TRANSFER = 2,
    USB_CMD_SEND_NSP_HEADER = 3,
    USB_CMD_END_SESSION = 4,
    USB_CMD_START_EXTRACTED_FS_DUMP = 5,
    USB_CMD_END_EXTRACTED_FS_DUMP = 6
};

// USB command block sizes
constexpr size_t USB_CMD_BLOCK_SIZE_START_SESSION = 0x10;
constexpr size_t USB_CMD_BLOCK_SIZE_SEND_FILE_PROPERTIES = 0x320;
constexpr size_t USB_CMD_BLOCK_SIZE_START_EXTRACTED_FS_DUMP = 0x310;

// Max filename length
constexpr size_t USB_FILE_PROPERTIES_MAX_NAME_LENGTH = 0x300;

// USB status codes
enum UsbStatusCode : uint32_t {
    USB_STATUS_SUCCESS = 0,
    USB_STATUS_INVALID_MAGIC_WORD = 4,
    USB_STATUS_UNSUPPORTED_CMD = 5,
    USB_STATUS_UNSUPPORTED_ABI_VERSION = 6,
    USB_STATUS_MALFORMED_CMD = 7,
    USB_STATUS_HOST_IO_ERROR = 8
};

// USB command header structure
#pragma pack(push, 1)
struct UsbCommandHeader {
    uint8_t magic[4];
    uint32_t cmdId;
    uint32_t cmdBlockSize;
    uint8_t reserved[4];
};

struct UsbStatusResponse {
    uint8_t magic[4];
    uint32_t status;
    uint16_t maxPacketSize;
    uint8_t reserved[6];
};
#pragma pack(pop)

#endif // USBCOMMANDS_H

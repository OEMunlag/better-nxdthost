#include "usbmanager.h"
#include <QDir>
#include <QElapsedTimer>
#include <QFileInfo>
#include <QStorageInfo>
#include <QThread>
#include <cstring>
#include <algorithm>

UsbManager::UsbManager(const QString& outputDir, QObject* parent)
    : QThread(parent)
    , m_context(nullptr)
    , m_deviceHandle(nullptr)
    , m_epIn(0)
    , m_epOut(0)
    , m_epMaxPacketSize(0)
    , m_outputDir(outputDir)
    , m_stopRequested(false)
    , m_nxdtVersionMajor(0)
    , m_nxdtVersionMinor(0)
    , m_nxdtVersionMicro(0)
    , m_nxdtAbiVersionMajor(0)
    , m_nxdtAbiVersionMinor(0)
    , m_nspTransferMode(false)
    , m_nspSize(0)
    , m_nspHeaderSize(0)
    , m_nspRemainingSize(0)
    , m_nspFile(nullptr)
{
}

UsbManager::~UsbManager() {
    resetNspInfo(false);
    
    if (m_deviceHandle) {
        libusb_release_interface(m_deviceHandle, 0);
        libusb_close(m_deviceHandle);
    }
    
    if (m_context) {
        libusb_exit(m_context);
    }
}

void UsbManager::run() {
    m_stopRequested = false;

    if (libusb_init(&m_context) < 0) {
        emit logMessage("Failed to initialize libusb!", 3);
        return;
    }

    commandHandler();
    
    emit serverStopped();
}

void UsbManager::stopServer() {
    m_stopRequested = true;
}

bool UsbManager::getDeviceEndpoints() {
    emit logMessage("Please connect a Nintendo Switch console running nxdumptool.", 1);
    
    libusb_device** devList = nullptr;
    ssize_t devCount = 0;
    
    while (!m_stopRequested) {
        devCount = libusb_get_device_list(m_context, &devList);
        if (devCount < 0) {
            QThread::msleep(100);
            continue;
        }
        
        for (ssize_t i = 0; i < devCount; i++) {
            libusb_device* dev = devList[i];
            libusb_device_descriptor desc;
            
            if (libusb_get_device_descriptor(dev, &desc) < 0) {
                continue;
            }
            
            if (desc.idVendor != USB_DEV_VID || desc.idProduct != USB_DEV_PID) {
                continue;
            }
            
            if (libusb_open(dev, &m_deviceHandle) < 0) {
                continue;
            }
            
            // Check manufacturer string
            unsigned char strBuf[256];
            if (libusb_get_string_descriptor_ascii(m_deviceHandle, desc.iManufacturer, 
                strBuf, sizeof(strBuf)) < 0) {
                libusb_close(m_deviceHandle);
                m_deviceHandle = nullptr;
                continue;
            }
            
            if (strcmp(reinterpret_cast<char*>(strBuf), USB_DEV_MANUFACTURER) != 0) {
                libusb_close(m_deviceHandle);
                m_deviceHandle = nullptr;
                continue;
            }
            
            // Reset device
            libusb_reset_device(m_deviceHandle);
            
            // Set configuration
            libusb_set_configuration(m_deviceHandle, 1);
            
            // Claim interface
            if (libusb_claim_interface(m_deviceHandle, 0) < 0) {
                libusb_close(m_deviceHandle);
                m_deviceHandle = nullptr;
                continue;
            }
            
            // Get endpoints
            libusb_config_descriptor* config;
            if (libusb_get_active_config_descriptor(dev, &config) < 0) {
                libusb_release_interface(m_deviceHandle, 0);
                libusb_close(m_deviceHandle);
                m_deviceHandle = nullptr;
                continue;
            }
            
            const libusb_interface* intf = &config->interface[0];
            const libusb_interface_descriptor* intfDesc = &intf->altsetting[0];
            
            for (int ep = 0; ep < intfDesc->bNumEndpoints; ep++) {
                const libusb_endpoint_descriptor* epDesc = &intfDesc->endpoint[ep];
                
                if ((epDesc->bEndpointAddress & LIBUSB_ENDPOINT_DIR_MASK) == LIBUSB_ENDPOINT_IN) {
                    m_epIn = epDesc->bEndpointAddress;
                    m_epMaxPacketSize = epDesc->wMaxPacketSize;
                } else {
                    m_epOut = epDesc->bEndpointAddress;
                }
            }
            
            libusb_free_config_descriptor(config);
            
            m_usbVersion = QString("%1.%2").arg(desc.bcdUSB >> 8).arg((desc.bcdUSB & 0xFF) >> 4);
            
            libusb_free_device_list(devList, 1);
            
            emit logMessage(QString("Successfully connected! Max packet size: 0x%1, USB: %2")
                .arg(m_epMaxPacketSize, 0, 16).arg(m_usbVersion), 0);
            emit logMessage("Exit nxdumptool on your console or disconnect it to stop the server.", 1);
            
            return true;
        }
        
        libusb_free_device_list(devList, 1);
        QThread::msleep(100);
    }
    
    return false;
}

QByteArray UsbManager::usbRead(size_t size, int timeout) {
    if (!m_deviceHandle) {
        return QByteArray();
    }

    QByteArray data(size, 0);

    const int pollTimeout = (timeout < 0) ? 500 : std::max(1, std::min(timeout, 500));
    QElapsedTimer timer;
    if (timeout >= 0) {
        timer.start();
    }

    while (!m_stopRequested) {
        int transferred = 0;
        int result = libusb_bulk_transfer(m_deviceHandle, m_epIn,
            reinterpret_cast<unsigned char*>(data.data()), size, &transferred, pollTimeout);

        if (result == LIBUSB_ERROR_TIMEOUT) {
            if (timeout >= 0 && timer.hasExpired(timeout)) {
                emit logMessage("USB read timed out!", 3);
                return QByteArray();
            }
            continue;
        }

        if (m_stopRequested) {
            break;
        }

        if (result < 0 || transferred != static_cast<int>(size)) {
            if (!m_stopRequested) {
                emit logMessage("USB read error!", 3);
            }
            return QByteArray();
        }

        return data;
    }

    return QByteArray();
}

bool UsbManager::usbWrite(const QByteArray& data, int timeout) {
    if (!m_deviceHandle) {
        return false;
    }

    const int pollTimeout = (timeout < 0) ? 500 : std::max(1, std::min(timeout, 500));
    QElapsedTimer timer;
    if (timeout >= 0) {
        timer.start();
    }

    while (!m_stopRequested) {
        int transferred = 0;
        int result = libusb_bulk_transfer(m_deviceHandle, m_epOut,
            reinterpret_cast<unsigned char*>(const_cast<char*>(data.data())),
            data.size(), &transferred, pollTimeout);

        if (result == LIBUSB_ERROR_TIMEOUT) {
            if (timeout >= 0 && timer.hasExpired(timeout)) {
                emit logMessage("USB write timed out!", 3);
                return false;
            }
            continue;
        }

        if (m_stopRequested) {
            break;
        }

        if (result < 0 || transferred != data.size()) {
            if (!m_stopRequested) {
                emit logMessage("USB write error!", 3);
            }
            return false;
        }

        return true;
    }

    return false;
}

bool UsbManager::usbSendStatus(uint32_t code) {
    UsbStatusResponse status;
    std::memcpy(status.magic, USB_MAGIC_WORD, 4);
    status.status = code;
    status.maxPacketSize = m_epMaxPacketSize;
    std::memset(status.reserved, 0, 6);
    
    QByteArray statusData(reinterpret_cast<char*>(&status), sizeof(status));
    return usbWrite(statusData, USB_TRANSFER_TIMEOUT);
}

// Command handlers implementation
uint32_t UsbManager::handleStartSession(const QByteArray& cmdBlock) {
    emit logMessage("Received StartSession command", 0);
    
    m_nxdtVersionMajor = static_cast<uint8_t>(cmdBlock[0]);
    m_nxdtVersionMinor = static_cast<uint8_t>(cmdBlock[1]);
    m_nxdtVersionMicro = static_cast<uint8_t>(cmdBlock[2]);
    
    uint8_t abiVersion = static_cast<uint8_t>(cmdBlock[3]);
    m_nxdtAbiVersionMajor = (abiVersion >> 4) & 0x0F;
    m_nxdtAbiVersionMinor = abiVersion & 0x0F;
    
    m_nxdtGitCommit = QString::fromLatin1(cmdBlock.mid(4, 8)).trimmed();
    
    emit logMessage(QString("Client: nxdumptool v%1.%2.%3, ABI v%4.%5 (commit %6), USB %7")
        .arg(m_nxdtVersionMajor).arg(m_nxdtVersionMinor).arg(m_nxdtVersionMicro)
        .arg(m_nxdtAbiVersionMajor).arg(m_nxdtAbiVersionMinor)
        .arg(m_nxdtGitCommit).arg(m_usbVersion), 1);
    
    if (m_nxdtAbiVersionMajor != USB_ABI_VERSION_MAJOR || 
        m_nxdtAbiVersionMinor != USB_ABI_VERSION_MINOR) {
        emit logMessage("Unsupported ABI version!", 3);
        return USB_STATUS_UNSUPPORTED_ABI_VERSION;
    }
    
    return USB_STATUS_SUCCESS;
}

uint32_t UsbManager::handleSendFileProperties(const QByteArray& cmdBlock) {
    emit logMessage("Received SendFileProperties command", 0);
    
    qint64 fileSize = *reinterpret_cast<const qint64*>(cmdBlock.constData());
    uint32_t filenameLength = *reinterpret_cast<const uint32_t*>(cmdBlock.constData() + 8);
    uint32_t nspHeaderSize = *reinterpret_cast<const uint32_t*>(cmdBlock.constData() + 12);
    QString filename = QString::fromUtf8(cmdBlock.mid(16, filenameLength));
    
    emit logMessage(QString("File: \"%1\" (size: 0x%2)").arg(filename).arg(fileSize, 0, 16), 0);
    
    // Validation checks
    if (!m_nspTransferMode && fileSize && nspHeaderSize >= fileSize) {
        emit logMessage("NSP header size must be smaller than full NSP size!", 3);
        return USB_STATUS_MALFORMED_CMD;
    }
    
    if (m_nspTransferMode && nspHeaderSize) {
        emit logMessage("Received non-zero NSP header size during NSP transfer!", 3);
        return USB_STATUS_MALFORMED_CMD;
    }
    
    // Enable NSP transfer mode if needed
    if (!m_nspTransferMode && fileSize && nspHeaderSize) {
        m_nspTransferMode = true;
        m_nspSize = fileSize;
        m_nspHeaderSize = nspHeaderSize;
        m_nspRemainingSize = fileSize - nspHeaderSize;
        emit logMessage("NSP transfer mode enabled", 0);
    }
    
    // Get file path and create directories
    QFile* file = nullptr;
    QString fullPath;
    
    if (!m_nspTransferMode || !m_nspFile) {
        fullPath = QDir(m_outputDir).filePath(filename);
        QFileInfo fileInfo(fullPath);
        QDir().mkpath(fileInfo.absolutePath());
        
        if (fileInfo.exists() && fileInfo.isDir()) {
            resetNspInfo();
            emit logMessage("Output path points to existing directory!", 3);
            return USB_STATUS_HOST_IO_ERROR;
        }
        
        QStorageInfo storage(fileInfo.absolutePath());
        if (storage.bytesAvailable() < fileSize) {
            resetNspInfo();
            emit logMessage("Not enough free space!", 3);
            return USB_STATUS_HOST_IO_ERROR;
        }
        
        file = new QFile(fullPath);
        if (!file->open(QIODevice::WriteOnly)) {
            delete file;
            resetNspInfo();
            emit logMessage("Failed to open output file!", 3);
            return USB_STATUS_HOST_IO_ERROR;
        }
        
        if (m_nspTransferMode) {
            m_nspFile = file;
            m_nspFilePath = fullPath;
            
            // Write NSP header padding
            QByteArray padding(m_nspHeaderSize, '\0');
            file->write(padding);
        }
    } else {
        file = m_nspFile;
        fullPath = m_nspFilePath;
    }
    
    if (!fileSize || (m_nspTransferMode && fileSize == m_nspSize)) {
        if (!m_nspTransferMode) {
            file->close();
            delete file;
        }
        return USB_STATUS_SUCCESS;
    }
    
    // Send success before data transfer
    usbSendStatus(USB_STATUS_SUCCESS);
    
    QString fileType = m_nspTransferMode ? "NSP entry" : "file";
    emit logMessage(QString("Receiving %1: \"%2\"").arg(fileType).arg(filename), 1);
    
    // Start progress tracking
    bool useProgressBar = ((!m_nspTransferMode && fileSize > USB_TRANSFER_THRESHOLD) ||
                          (m_nspTransferMode && m_nspSize > USB_TRANSFER_THRESHOLD));
    
    if (useProgressBar) {
        qint64 initialOffset = m_nspTransferMode ? m_nspHeaderSize : 0;
        qint64 progressTotal = m_nspTransferMode ? m_nspSize : fileSize;
        emit startOffset(progressTotal, filename);
    }
    
    // Transfer data
    qint64 offset = 0;
    size_t blockSize = USB_TRANSFER_BLOCK_SIZE;
    
    while (offset < fileSize) {
        qint64 remaining = fileSize - offset;
        if (blockSize > remaining) {
            blockSize = remaining;
        }
        
        size_t readSize = blockSize;
        if ((offset + blockSize) >= fileSize && isValueAlignedToEndpointPacketSize(blockSize)) {
            readSize += 1; // Handle ZLT
        }
        
        QByteArray chunk = usbRead(readSize, USB_TRANSFER_TIMEOUT);
        if (chunk.isEmpty()) {
            if (!m_stopRequested) {
                emit logMessage("Failed to read data chunk!", 3);
            }
            if (m_nspTransferMode) {
                resetNspInfo(true);
            } else {
                file->close();
                delete file;
                QFile::remove(fullPath);
            }
            if (useProgressBar) emit progressEnd();
            return USB_STATUS_HOST_IO_ERROR;
        }
        
        // Check for cancel command
        if (chunk.size() == USB_CMD_HEADER_SIZE) {
            UsbCommandHeader* hdr = reinterpret_cast<UsbCommandHeader*>(chunk.data());
            if (std::memcmp(hdr->magic, USB_MAGIC_WORD, 4) == 0 && 
                hdr->cmdId == USB_CMD_CANCEL_FILE_TRANSFER) {
                if (m_nspTransferMode) {
                    resetNspInfo(true);
                } else {
                    file->close();
                    delete file;
                    QFile::remove(fullPath);
                }
                if (useProgressBar) emit progressEnd();
                emit logMessage("Transfer cancelled by console", 2);
                return USB_STATUS_SUCCESS;
            }
        }
        
        file->write(chunk);
        file->flush();
        
        offset += chunk.size();
        if (m_nspTransferMode) {
            m_nspRemainingSize -= chunk.size();
        }
        
        if (useProgressBar) {
            emit progressUpdate(offset, fileSize, filename);
        }
    }
    
    emit logMessage("File transfer completed successfully", 0);
    
    if (!m_nspTransferMode) {
        file->close();
        delete file;
    }
    
    if (useProgressBar && (!m_nspTransferMode || !m_nspRemainingSize)) {
        emit progressEnd();
    }
    
    return USB_STATUS_SUCCESS;
}

uint32_t UsbManager::handleCancelFileTransfer(const QByteArray& cmdBlock) {
    emit logMessage("Received CancelFileTransfer command", 0);
    
    if (m_nspTransferMode) {
        resetNspInfo(true);
        emit logMessage("Transfer cancelled", 2);
        return USB_STATUS_SUCCESS;
    }
    
    emit logMessage("Unexpected transfer cancellation", 3);
    return USB_STATUS_MALFORMED_CMD;
}

uint32_t UsbManager::handleSendNspHeader(const QByteArray& cmdBlock) {
    emit logMessage("Received SendNspHeader command", 0);
    
    if (!m_nspTransferMode) {
        emit logMessage("Received NSP header outside NSP transfer mode!", 3);
        return USB_STATUS_MALFORMED_CMD;
    }
    
    if (m_nspRemainingSize) {
        emit logMessage(QString("NSP header received before all data! (missing 0x%1 bytes)")
            .arg(m_nspRemainingSize, 0, 16), 3);
        return USB_STATUS_MALFORMED_CMD;
    }
    
    if (cmdBlock.size() != m_nspHeaderSize) {
        emit logMessage("NSP header size mismatch!", 3);
        return USB_STATUS_MALFORMED_CMD;
    }
    
    m_nspFile->seek(0);
    m_nspFile->write(cmdBlock);
    
    emit logMessage(QString("Wrote NSP header (0x%1 bytes)").arg(m_nspHeaderSize, 0, 16), 0);
    
    resetNspInfo();
    
    return USB_STATUS_SUCCESS;
}

uint32_t UsbManager::handleEndSession(const QByteArray& cmdBlock) {
    emit logMessage("Received EndSession command", 0);
    return USB_STATUS_SUCCESS;
}

uint32_t UsbManager::handleStartExtractedFsDump(const QByteArray& cmdBlock) {
    emit logMessage("Received StartExtractedFsDump command", 0);
    
    if (m_nspTransferMode) {
        emit logMessage("StartExtractedFsDump received during NSP transfer!", 3);
        return USB_STATUS_MALFORMED_CMD;
    }
    
    qint64 fsSize = *reinterpret_cast<const qint64*>(cmdBlock.constData());
    QString rootPath = QString::fromUtf8(cmdBlock.mid(8)).trimmed();
    
    emit logMessage(QString("Starting extracted FS dump (size: 0x%1, path: \"%2\")")
        .arg(fsSize, 0, 16).arg(rootPath), 1);
    
    return USB_STATUS_SUCCESS;
}

uint32_t UsbManager::handleEndExtractedFsDump(const QByteArray& cmdBlock) {
    emit logMessage("Received EndExtractedFsDump command", 0);
    emit logMessage("Finished extracted FS dump", 1);
    return USB_STATUS_SUCCESS;
}

void UsbManager::commandHandler() {
    if (!getDeviceEndpoints()) {
        return;
    }
    
    resetNspInfo();
    
    while (!m_stopRequested) {
        QByteArray cmdHeader = usbRead(USB_CMD_HEADER_SIZE);
        if (cmdHeader.isEmpty()) {
            if (!m_stopRequested) {
                emit logMessage("Failed to read command header!", 3);
            }
            break;
        }
        
        UsbCommandHeader* hdr = reinterpret_cast<UsbCommandHeader*>(cmdHeader.data());
        
        emit logMessage(QString("Command header: ID=%1, BlockSize=0x%2")
            .arg(hdr->cmdId).arg(hdr->cmdBlockSize, 0, 16), 0);
        
        QByteArray cmdBlock;
        if (hdr->cmdBlockSize > 0) {
            size_t readSize = hdr->cmdBlockSize;
            if (isValueAlignedToEndpointPacketSize(hdr->cmdBlockSize)) {
                readSize += 1;
            }
            
            cmdBlock = usbRead(readSize, USB_TRANSFER_TIMEOUT);
            if (cmdBlock.isEmpty() || cmdBlock.size() != static_cast<int>(hdr->cmdBlockSize)) {
                if (!m_stopRequested) {
                    emit logMessage(QString("Failed to read command block (expected 0x%1 bytes)!")
                        .arg(hdr->cmdBlockSize, 0, 16), 3);
                }
                break;
            }
        }
        
        if (std::memcmp(hdr->magic, USB_MAGIC_WORD, 4) != 0) {
            emit logMessage("Invalid magic word in command header!", 3);
            usbSendStatus(USB_STATUS_INVALID_MAGIC_WORD);
            continue;
        }
        
        uint32_t status = USB_STATUS_UNSUPPORTED_CMD;
        
        switch (hdr->cmdId) {
            case USB_CMD_START_SESSION:
                status = handleStartSession(cmdBlock);
                break;
            case USB_CMD_SEND_FILE_PROPERTIES:
                status = handleSendFileProperties(cmdBlock);
                break;
            case USB_CMD_CANCEL_FILE_TRANSFER:
                status = handleCancelFileTransfer(cmdBlock);
                break;
            case USB_CMD_SEND_NSP_HEADER:
                status = handleSendNspHeader(cmdBlock);
                break;
            case USB_CMD_END_SESSION:
                status = handleEndSession(cmdBlock);
                break;
            case USB_CMD_START_EXTRACTED_FS_DUMP:
                status = handleStartExtractedFsDump(cmdBlock);
                break;
            case USB_CMD_END_EXTRACTED_FS_DUMP:
                status = handleEndExtractedFsDump(cmdBlock);
                break;
            default:
                emit logMessage(QString("Unsupported command ID: %1").arg(hdr->cmdId), 3);
                break;
        }
        
        if (!usbSendStatus(status) || hdr->cmdId == USB_CMD_END_SESSION || 
            status == USB_STATUS_UNSUPPORTED_ABI_VERSION) {
            break;
        }
    }
    
    if (!m_stopRequested) {
        emit logMessage("Stopping server", 1);
    }
}

void UsbManager::resetNspInfo(bool deleteFile) {
    if (m_nspFile) {
        m_nspFile->close();
        if (deleteFile && !m_nspFilePath.isEmpty()) {
            QFile::remove(m_nspFilePath);
        }
        delete m_nspFile;
        m_nspFile = nullptr;
    }
    
    m_nspTransferMode = false;
    m_nspSize = 0;
    m_nspHeaderSize = 0;
    m_nspRemainingSize = 0;
    m_nspFilePath.clear();
}

bool UsbManager::isValueAlignedToEndpointPacketSize(size_t value) const {
    return (value & (m_epMaxPacketSize - 1)) == 0;
}

QString UsbManager::getSizeUnit(qint64 size, qint64& divisor) const {
    const char* units[] = {"B", "KiB", "MiB", "GiB"};
    int unitCount = 4;
    
    for (int i = 0; i < unitCount; i++) {
        qint64 threshold = qint64(1) << (10 * (i + 1));
        if (size < threshold || i == unitCount - 1) {
            divisor = qint64(1) << (10 * i);
            return QString(units[i]);
        }
    }
    
    divisor = 1;
    return "B";
}

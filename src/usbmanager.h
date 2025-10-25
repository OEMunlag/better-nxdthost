#ifndef USBMANAGER_H
#define USBMANAGER_H

#include <QObject>
#include <QThread>
#include <QByteArray>
#include <QFile>
#include <libusb-1.0/libusb.h>
#include "usbcommands.h"

class UsbManager : public QThread {
    Q_OBJECT

public:
    explicit UsbManager(const QString& outputDir, bool disableFreeSpaceCheck,
        QObject* parent = nullptr);
    ~UsbManager() override;

    void stopServer();

signals:
    void logMessage(const QString& message, int level); // 0=debug, 1=info, 2=warning, 3=error
    void progressUpdate(qint64 current, qint64 total, const QString& filename);
    void startOffset(qint64 total, const QString& filename);
    void progressEnd();
    void serverStopped();

protected:
    void run() override;

private:
    bool getDeviceEndpoints();
    QByteArray usbRead(size_t size, int timeout = -1);
    bool usbWrite(const QByteArray& data, int timeout = -1);
    bool usbSendStatus(uint32_t code);
    
    // Command handlers
    uint32_t handleStartSession(const QByteArray& cmdBlock);
    uint32_t handleSendFileProperties(const QByteArray& cmdBlock);
    uint32_t handleCancelFileTransfer(const QByteArray& cmdBlock);
    uint32_t handleSendNspHeader(const QByteArray& cmdBlock);
    uint32_t handleEndSession(const QByteArray& cmdBlock);
    uint32_t handleStartExtractedFsDump(const QByteArray& cmdBlock);
    uint32_t handleEndExtractedFsDump(const QByteArray& cmdBlock);
    
    void commandHandler();
    void resetNspInfo(bool deleteFile = false);
    bool isValueAlignedToEndpointPacketSize(size_t value) const;
    QString getSizeUnit(qint64 size, qint64& divisor) const;
    QString sanitizeFilename(const QString& filename) const;

    libusb_context* m_context;
    libusb_device_handle* m_deviceHandle;
    uint8_t m_epIn;
    uint8_t m_epOut;
    uint16_t m_epMaxPacketSize;
    QString m_usbVersion;
    
    QString m_outputDir;
    bool m_stopRequested;
    bool m_disableFreeSpaceCheck;
    
    // nxdumptool version info
    uint8_t m_nxdtVersionMajor;
    uint8_t m_nxdtVersionMinor;
    uint8_t m_nxdtVersionMicro;
    uint8_t m_nxdtAbiVersionMajor;
    uint8_t m_nxdtAbiVersionMinor;
    QString m_nxdtGitCommit;
    
    // NSP transfer state
    bool m_nspTransferMode;
    qint64 m_nspSize;
    qint64 m_nspHeaderSize;
    qint64 m_nspRemainingSize;
    QFile* m_nspFile;
    QString m_nspFilePath;
};

#endif // USBMANAGER_H

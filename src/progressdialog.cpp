#include "progressdialog.h"
#include <QVBoxLayout>
#include <QDateTime>

ProgressDialog::ProgressDialog(QWidget* parent)
    : QDialog(parent)
    , m_lastBytes(0)
{
    setWindowTitle("File Transfer");
    setModal(true);
    setMinimumWidth(500);
    
    // Prevent closing the dialog
    setWindowFlags(windowFlags() & ~Qt::WindowCloseButtonHint);
    
    QVBoxLayout* layout = new QVBoxLayout(this);
    
    m_filenameLabel = new QLabel(this);
    m_filenameLabel->setWordWrap(true);
    layout->addWidget(m_filenameLabel);
    
    m_progressLabel = new QLabel(this);
    layout->addWidget(m_progressLabel);
    
    m_progressBar = new QProgressBar(this);
    m_progressBar->setMinimum(0);
    m_progressBar->setMaximum(100);
    layout->addWidget(m_progressBar);
    
    m_statusLabel = new QLabel(this);
    layout->addWidget(m_statusLabel);
    
    QLabel* tipLabel = new QLabel("Use your console to cancel the file transfer if you wish to do so.", this);
    tipLabel->setWordWrap(true);
    tipLabel->setStyleSheet("QLabel { color: gray; font-style: italic; }");
    layout->addWidget(tipLabel);
}

void ProgressDialog::start(qint64 total, const QString& filename) {
    m_startTime = QTime::currentTime();
    m_lastUpdateTime = m_startTime;
    m_lastBytes = 0;
    
    updateDisplay(0, total, filename);
    
    if (!isVisible()) {
        // Center on parent
        if (parentWidget()) {
            move(parentWidget()->geometry().center() - rect().center());
        }
        show();
    }
}

void ProgressDialog::update(qint64 current, qint64 total, const QString& filename) {
    updateDisplay(current, total, filename);
}

void ProgressDialog::end() {
    hide();
    m_lastBytes = 0;
}

void ProgressDialog::updateDisplay(qint64 current, qint64 total, const QString& filename) {
    // Update filename
    m_filenameLabel->setText(QString("Current file: %1").arg(filename));
    
    // Calculate percentage
    double percentage = (total > 0) ? (100.0 * current / total) : 0.0;
    
    // Update progress
    m_progressLabel->setText(QString("%1% - %2 / %3")
        .arg(percentage, 0, 'f', 2)
        .arg(formatSize(current))
        .arg(formatSize(total)));
    
    m_progressBar->setValue(static_cast<int>(percentage));
    
    // Calculate elapsed time
    int elapsedMs = m_startTime.msecsTo(QTime::currentTime());
    int elapsedSec = elapsedMs / 1000;
    
    // Calculate speed (update every 500ms to avoid flickering)
    QTime currentTime = QTime::currentTime();
    int timeSinceLastUpdate = m_lastUpdateTime.msecsTo(currentTime);
    
    QString speedStr = "Calculating...";
    QString remainingStr = "Unknown";
    
    if (timeSinceLastUpdate >= 500 && m_lastBytes > 0) {
        qint64 bytesDiff = current - m_lastBytes;
        double speed = (bytesDiff * 1000.0) / timeSinceLastUpdate; // bytes per second
        
        speedStr = formatSpeed(static_cast<qint64>(speed));
        
        if (speed > 0) {
            qint64 remaining = total - current;
            int remainingSec = static_cast<int>(remaining / speed);
            
            int hours = remainingSec / 3600;
            int minutes = (remainingSec % 3600) / 60;
            int seconds = remainingSec % 60;
            
            if (hours > 0) {
                remainingStr = QString("%1h %2m %3s").arg(hours).arg(minutes).arg(seconds);
            } else if (minutes > 0) {
                remainingStr = QString("%1m %2s").arg(minutes).arg(seconds);
            } else {
                remainingStr = QString("%1s").arg(seconds);
            }
        }
        
        m_lastBytes = current;
        m_lastUpdateTime = currentTime;
    } else if (m_lastBytes == 0) {
        m_lastBytes = current;
    }
    
    // Format elapsed time
    int hours = elapsedSec / 3600;
    int minutes = (elapsedSec % 3600) / 60;
    int seconds = elapsedSec % 60;
    
    QString elapsedStr;
    if (hours > 0) {
        elapsedStr = QString("%1h %2m %3s").arg(hours).arg(minutes).arg(seconds);
    } else if (minutes > 0) {
        elapsedStr = QString("%1m %2s").arg(minutes).arg(seconds);
    } else {
        elapsedStr = QString("%1s").arg(seconds);
    }
    
    m_statusLabel->setText(QString("Elapsed: %1 | Remaining: %2 | Speed: %3")
        .arg(elapsedStr)
        .arg(remainingStr)
        .arg(speedStr));
}

QString ProgressDialog::formatSize(qint64 bytes) const {
    const char* units[] = {"B", "KiB", "MiB", "GiB"};
    int unitIndex = 0;
    double size = bytes;
    
    while (size >= 1024.0 && unitIndex < 3) {
        size /= 1024.0;
        unitIndex++;
    }
    
    return QString("%1 %2").arg(size, 0, 'f', 2).arg(units[unitIndex]);
}

QString ProgressDialog::formatSpeed(qint64 bytesPerSecond) const {
    const char* units[] = {"B/s", "KiB/s", "MiB/s", "GiB/s"};
    int unitIndex = 0;
    double speed = bytesPerSecond;
    
    while (speed >= 1024.0 && unitIndex < 3) {
        speed /= 1024.0;
        unitIndex++;
    }
    
    return QString("%1 %2").arg(speed, 0, 'f', 2).arg(units[unitIndex]);
}

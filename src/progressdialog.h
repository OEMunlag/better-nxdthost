#ifndef PROGRESSDIALOG_H
#define PROGRESSDIALOG_H

#include <QDialog>
#include <QProgressBar>
#include <QLabel>
#include <QTime>

class ProgressDialog : public QDialog {
    Q_OBJECT

public:
    explicit ProgressDialog(QWidget* parent = nullptr);
    
public slots:
    void start(qint64 total, const QString& filename);
    void update(qint64 current, qint64 total, const QString& filename);
    void end();

private:
    void updateDisplay(qint64 current, qint64 total, const QString& filename);
    QString formatSize(qint64 bytes) const;
    QString formatSpeed(qint64 bytesPerSecond) const;
    
    QLabel* m_filenameLabel;
    QLabel* m_progressLabel;
    QProgressBar* m_progressBar;
    QLabel* m_statusLabel;
    
    QTime m_startTime;
    qint64 m_lastBytes;
    QTime m_lastUpdateTime;
};

#endif // PROGRESSDIALOG_H

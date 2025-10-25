#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QCheckBox>
#include <QLabel>
#include "usbmanager.h"
#include "progressdialog.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(const QString& outputDir = QString(), bool verboseMode = false,
        bool disableFreeSpaceCheck = false, QWidget* parent = nullptr);
    ~MainWindow() override;

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    void onChooseDirectory();
    void onStartServer();
    void onStopServer();
    void onLogMessage(const QString& message, int level);
    void onProgressStart(qint64 total, const QString& filename);
    void onProgressUpdate(qint64 current, qint64 total, const QString& filename);
    void onProgressEnd();
    void onServerStopped();
    void onVerboseToggled(int state);

private:
    void setupUi();
    void toggleElements(bool enabled);
    void appendLog(const QString& message, const QString& color);
    
    QLineEdit* m_dirLineEdit;
    QPushButton* m_chooseDirButton;
    QPushButton* m_serverButton;
    QLabel* m_tipLabel;
    QTextEdit* m_logTextEdit;
    QCheckBox* m_verboseCheckBox;
    
    UsbManager* m_usbManager;
    ProgressDialog* m_progressDialog;
    
    QString m_outputDir;
    bool m_verboseMode;
    bool m_disableFreeSpaceCheck;
};

#endif // MAINWINDOW_H

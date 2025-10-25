#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QDir>
#include <QStandardPaths>
#include <QCloseEvent>
#include <QScrollBar>

MainWindow::MainWindow(const QString& outputDir, bool verboseMode, QWidget* parent)
    : QMainWindow(parent)
    , m_usbManager(nullptr)
    , m_progressDialog(nullptr)
    , m_outputDir(outputDir)
    , m_verboseMode(verboseMode)
{
    setWindowTitle(QString("nxdumptool host v%1").arg(APP_VERSION));
    setMinimumSize(600, 550);
    resize(600, 550);

    if (m_outputDir.isEmpty()) {
        QString documentsPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
        m_outputDir = QDir(documentsPath).filePath("nxdumptool");
    }

    setupUi();

    m_verboseCheckBox->setChecked(m_verboseMode);

    m_progressDialog = new ProgressDialog(this);
}

MainWindow::~MainWindow() {
    if (m_usbManager && m_usbManager->isRunning()) {
        m_usbManager->stopServer();
        m_usbManager->wait(3000);
    }
}

void MainWindow::setupUi() {
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10);
    
    // Directory selection
    QHBoxLayout* dirLayout = new QHBoxLayout();
    QLabel* dirLabel = new QLabel("Output directory:", this);
    m_dirLineEdit = new QLineEdit(m_outputDir, this);
    m_dirLineEdit->setReadOnly(true);
    m_chooseDirButton = new QPushButton("Choose", this);
    m_chooseDirButton->setMaximumWidth(80);
    
    dirLayout->addWidget(dirLabel);
    dirLayout->addWidget(m_dirLineEdit);
    dirLayout->addWidget(m_chooseDirButton);
    mainLayout->addLayout(dirLayout);
    
    // Server button
    m_serverButton = new QPushButton("Start Server", this);
    m_serverButton->setMaximumWidth(150);
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_serverButton);
    buttonLayout->addStretch();
    mainLayout->addLayout(buttonLayout);
    
    // Tip label
    m_tipLabel = new QLabel(this);
    m_tipLabel->setWordWrap(true);
    m_tipLabel->setAlignment(Qt::AlignCenter);
    m_tipLabel->setStyleSheet("QLabel { color: blue; }");
    m_tipLabel->hide();
    mainLayout->addWidget(m_tipLabel);
    
    // Log output
    m_logTextEdit = new QTextEdit(this);
    m_logTextEdit->setReadOnly(true);
    m_logTextEdit->setMinimumHeight(300);
    mainLayout->addWidget(m_logTextEdit);
    
    // Bottom bar
    QHBoxLayout* bottomLayout = new QHBoxLayout();
    
    QLabel* copyrightLabel = new QLabel("Copyright (c) 2020-2024, DarkMatterCore", this);
    copyrightLabel->setStyleSheet("QLabel { color: gray; }");
    
    m_verboseCheckBox = new QCheckBox("Verbose output", this);
    
    bottomLayout->addWidget(copyrightLabel);
    bottomLayout->addStretch();
    bottomLayout->addWidget(m_verboseCheckBox);
    mainLayout->addLayout(bottomLayout);
    
    // Connect signals
    connect(m_chooseDirButton, &QPushButton::clicked, this, &MainWindow::onChooseDirectory);
    connect(m_serverButton, &QPushButton::clicked, this, &MainWindow::onStartServer);
    connect(m_verboseCheckBox, &QCheckBox::stateChanged, this, &MainWindow::onVerboseToggled);
}

void MainWindow::onChooseDirectory() {
    QString dir = QFileDialog::getExistingDirectory(this, 
        "Select Output Directory",
        m_outputDir,
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    
    if (!dir.isEmpty()) {
        m_outputDir = dir;
        m_dirLineEdit->setText(m_outputDir);
    }
}

void MainWindow::onStartServer() {
    // Validate output directory
    if (m_outputDir.isEmpty()) {
        QMessageBox::critical(this, "Error", "You must provide an output directory!");
        return;
    }
    
    // Create directory if it doesn't exist
    QDir dir;
    if (!dir.mkpath(m_outputDir)) {
        QMessageBox::critical(this, "Error", "Unable to create output directory!");
        return;
    }
    
    // Clear log
    m_logTextEdit->clear();
    
    // Create and start USB manager
    m_usbManager = new UsbManager(m_outputDir, this);
    
    connect(m_usbManager, &UsbManager::logMessage, this, &MainWindow::onLogMessage);
    connect(m_usbManager, &UsbManager::startOffset, this, &MainWindow::onProgressStart);
    connect(m_usbManager, &UsbManager::progressUpdate, this, &MainWindow::onProgressUpdate);
    connect(m_usbManager, &UsbManager::progressEnd, this, &MainWindow::onProgressEnd);
    connect(m_usbManager, &UsbManager::serverStopped, this, &MainWindow::onServerStopped);
    
    m_usbManager->start();
    
    // Update UI
    toggleElements(false);
}

void MainWindow::onStopServer() {
    if (m_usbManager) {
        m_usbManager->stopServer();
        // UI will be updated in onServerStopped slot
    }
}

void MainWindow::onLogMessage(const QString& message, int level) {
    QString color;
    
    switch (level) {
        case 0: // Debug
            if (!m_verboseMode) return;
            color = "gray";
            break;
        case 1: // Info
            color = "black";
            break;
        case 2: // Warning
            color = "orange";
            break;
        case 3: // Error
            color = "red";
            break;
        default:
            color = "black";
    }
    
    appendLog(message, color);
}

void MainWindow::onProgressStart(qint64 total, const QString& filename) {
    m_progressDialog->start(total, filename);
}

void MainWindow::onProgressUpdate(qint64 current, qint64 total, const QString& filename) {
    m_progressDialog->update(current, total, filename);
}

void MainWindow::onProgressEnd() {
    m_progressDialog->end();
}

void MainWindow::onServerStopped() {
    toggleElements(true);
    
    if (m_usbManager) {
        m_usbManager->deleteLater();
        m_usbManager = nullptr;
    }
}

void MainWindow::onVerboseToggled(int state) {
    m_verboseMode = (state == Qt::Checked);
}

void MainWindow::toggleElements(bool enabled) {
    if (enabled) {
        m_chooseDirButton->setEnabled(true);
        m_serverButton->setText("Start Server");
        QObject::disconnect(m_serverButton, &QPushButton::clicked, this, &MainWindow::onStopServer);
        QObject::connect(m_serverButton, &QPushButton::clicked, this, &MainWindow::onStartServer,
            Qt::UniqueConnection);
        m_tipLabel->hide();
        m_verboseCheckBox->setEnabled(true);
    } else {
        m_chooseDirButton->setEnabled(false);
        m_serverButton->setText("Stop Server");
        QObject::disconnect(m_serverButton, &QPushButton::clicked, this, &MainWindow::onStartServer);
        QObject::connect(m_serverButton, &QPushButton::clicked, this, &MainWindow::onStopServer,
            Qt::UniqueConnection);
        m_tipLabel->setText("Please connect a Nintendo Switch console running nxdumptool.\n"
                           "Exit nxdumptool on your console or disconnect it to stop the server.");
        m_tipLabel->show();
        m_verboseCheckBox->setEnabled(false);
    }
}

void MainWindow::appendLog(const QString& message, const QString& color) {
    QTextCursor cursor = m_logTextEdit->textCursor();
    cursor.movePosition(QTextCursor::End);
    
    QTextCharFormat format;
    format.setForeground(QColor(color));
    cursor.setCharFormat(format);
    cursor.insertText(message + "\n");
    
    // Scroll to bottom
    QScrollBar* scrollBar = m_logTextEdit->verticalScrollBar();
    scrollBar->setValue(scrollBar->maximum());
}

void MainWindow::closeEvent(QCloseEvent* event) {
    if (m_usbManager && m_usbManager->isRunning()) {
        QMessageBox::StandardButton reply = QMessageBox::question(this,
            "Server Running",
            "The server is still running. Are you sure you want to quit?",
            QMessageBox::Yes | QMessageBox::No);
        
        if (reply == QMessageBox::Yes) {
            m_usbManager->stopServer();
            m_usbManager->wait(3000);
            event->accept();
        } else {
            event->ignore();
        }
    } else {
        event->accept();
    }
}

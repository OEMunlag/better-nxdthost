#include <QApplication>
#include <QCommandLineParser>
#include <QMessageBox>
#include <QStyleFactory>
#include "mainwindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    app.setApplicationName("nxdumptool host");
    app.setApplicationVersion(APP_VERSION);
    app.setOrganizationName("DarkMatterCore");
    
    // Set a modern style if available
#ifdef Q_OS_WIN
    app.setStyle(QStyleFactory::create("Fusion"));
#endif
    
    // Parse command line arguments
    QCommandLineParser parser;
    parser.setApplicationDescription("nxdumptool host application");
    parser.addHelpOption();
    parser.addVersionOption();
    
    QCommandLineOption outputDirOption(QStringList() << "o" << "outdir",
        "Path to output directory", "DIR");
    parser.addOption(outputDirOption);
    
    QCommandLineOption verboseOption(QStringList() << "V" << "verbose",
        "Enable verbose output");
    parser.addOption(verboseOption);

    parser.process(app);

    const QString outputDir = parser.value(outputDirOption);
    const bool verboseMode = parser.isSet(verboseOption);
    
    // Check for libusb at startup
    libusb_context* testContext = nullptr;
    if (libusb_init(&testContext) < 0) {
        QMessageBox::critical(nullptr, "Error",
            "Failed to initialize libusb!\n\n"
            "On Windows, make sure the libusbK driver is installed using Zadig.\n"
            "On macOS, install libusb using: brew install libusb\n"
            "On Linux, install libusb-1.0 from your package manager.");
        return 1;
    }
    libusb_exit(testContext);
    
    MainWindow window(outputDir, verboseMode);
    window.show();
    
    return app.exec();
}

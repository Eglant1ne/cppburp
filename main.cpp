// SPDX-License-Identifier: MIT
// Encoding: UTF-8
#include "mainwindow.h"
#include <QApplication>
#include <QFont>
#include <QMessageBox>
#include <stdexcept>

/**
 * @brief Application entry point.
 *
 * Constructs the QApplication, sets default font, creates the main window,
 * and runs the event loop. All unhandled std::exception instances are caught
 * and shown as a message box before the application exits with code 1.
 *
 * @param argc  Command-line argument count.
 * @param argv  Command-line argument vector.
 * @return 0 on clean exit, 1 if a fatal exception was thrown.
 */
int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("ProxyLab");
    app.setOrganizationName("ProxyLab");

    QFont font("Segoe UI", 9);
    app.setFont(font);

    try {
        MainWindow w;
        w.show();
        return app.exec();
    } catch (const std::exception& ex) {
        QMessageBox::critical(nullptr, "Fatal Error",
                              QString("ProxyLab encountered a fatal error:\n%1").arg(ex.what()));
        return 1;
    }
}

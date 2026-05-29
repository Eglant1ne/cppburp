#pragma once

#include <QMainWindow>
#include <QTabWidget>
#include <QTextEdit>
#include <QPlainTextEdit>
#include <QTableWidget>
#include <QSplitter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QStatusBar>
#include <QMenuBar>
#include <QGroupBox>
#include <QTreeWidget>
#include <QHeaderView>
#include <QTimer>
#include <QRandomGenerator>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow() = default;

private slots:
    void onInterceptToggled(bool checked);
    void onForwardClicked();
    void onDropClicked();
    void onSendToRepeater();
    void onRepeatRequest();
    void onClearLog();

private:
    void setupUI();
    void applyStyleSheet();
    void setupProxyTab();
    void setupRepeaterTab();
    void setupScannerTab();
    void setupLogTab();
    void setupTopBar();
    void addLogRow(const QString &method, const QString &host,
                   const QString &path, int status, int length, const QString &type);

    // Tabs
    QTabWidget      *m_mainTabs;

    // Proxy tab
    QWidget         *m_proxyTab;
    QCheckBox       *m_interceptCheck;
    QPushButton     *m_forwardBtn;
    QPushButton     *m_dropBtn;
    QPushButton     *m_sendRepeaterBtn;
    QPlainTextEdit  *m_rawRequest;
    QPlainTextEdit  *m_rawResponse;
    QTableWidget    *m_historyTable;

    // Repeater tab
    QWidget         *m_repeaterTab;
    QLineEdit       *m_targetHost;
    QPlainTextEdit  *m_repeaterRequest;
    QPlainTextEdit  *m_repeaterResponse;
    QPushButton     *m_sendBtn;

    // Scanner tab
    QWidget         *m_scannerTab;
    QTreeWidget     *m_scanResults;
    QTextEdit       *m_scanDetail;
    QPushButton     *m_startScanBtn;

    // Log tab
    QWidget         *m_logTab;
    QTableWidget    *m_logTable;
    QPushButton     *m_clearLogBtn;

    // Top bar
    QLabel          *m_statusLabel;
    QLabel          *m_interceptStatus;

    bool             m_interceptOn = false;
};

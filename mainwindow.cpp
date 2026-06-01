// SPDX-License-Identifier: MIT
// Encoding: UTF-8
#include "mainwindow.h"
#include <QApplication>
#include <stdexcept>
#include <QDateTime>
#include <QStringList>
#include <QHostAddress>
#include <QMenu>
#include <QAction>
#include <QDir>

// ──────────────────────────────────────────────────────────────────────────────
//  Stylesheet  (Cyberpunk Dark / Crimson palette)
// ──────────────────────────────────────────────────────────────────────────────
static const char *STYLESHEET = R"(
QMainWindow, QWidget {
    background-color: #121214;       /* Глубокий чёрный/угольный фон */
    color: #e0e0e6;                  /* Светло-серый текст для отличной читаемости */
    font-family: "Segoe UI", "Arial";
    font-size: 9pt;
}
QMenuBar {
    background-color: #1a1a1f;       /* Чуть светлее основного фона */
    color: #e0e0e6;
    padding: 2px 4px;
    font-size: 9pt;
    border-bottom: 1px solid #2a1b24; /* Едва заметная малиновая граница */
}
QMenuBar::item:selected { 
    background-color: #dc143c;       /* Малиновый (Crimson) при наведении */
    color: #ffffff; 
    border-radius: 3px; 
}
QMenu { 
    background-color: #1a1a1f; 
    border: 1px solid #dc143c;       /* Яркая малиновая рамка меню */
    color: #e0e0e6; 
}
QMenu::item:selected { 
    background-color: #2a1b24;       /* Тёмно-малиновый фон при выборе */
    color: #ff4081;                  /* Неоново-розовый текст */
}

QTabWidget::pane {
    border: 1px solid #3a2533;       /* Приглушенная малиново-бордовая рамка */
    border-top: none;
    background: #16161a;
    border-radius: 0 0 4px 4px;
}
QTabBar::tab {
    background: #1a1a1f;
    color: #a0a0a8;
    padding: 6px 18px;
    border: 1px solid #2a1b24;
    border-bottom: none;
    margin-right: 2px;
    border-radius: 4px 4px 0 0;
    font-size: 9pt;
}
QTabBar::tab:selected { 
    background: #dc143c;             /* Активная вкладка горит малиновым */
    color: #ffffff; 
    font-weight: bold; 
    border-color: #dc143c;
}
QTabBar::tab:hover:!selected { 
    background: #2a1b24; 
    color: #ff4081; 
}

QPushButton {
    background-color: #2a1b24;       /* Тёмная кнопка с малиновым оттенком */
    color: #ff4081;                  /* Неоновый текст */
    border: 1px solid #dc143c;       /* Малиновая рамка */
    padding: 5px 16px;
    border-radius: 4px;
    font-size: 9pt;
    font-weight: bold;
}
QPushButton:hover  { 
    background-color: #dc143c;       /* Заливка малиновым при наведении */
    color: #ffffff; 
}
QPushButton:pressed {
    background-color: #a30f2e;       /* Более тёмный красный при клике */
    border-color: #a30f2e;
    padding: 6px 15px 4px 17px;
}
QPushButton:disabled { 
    background-color: #16161a; 
    color: #4a4a52; 
    border-color: #2a2a30; 
}

/* Специфические кнопки действий */
QPushButton#dangerBtn  { background-color: #5c0612; color: #ff5252; border-color: #ff5252; }
QPushButton#dangerBtn:hover { background-color: #ff5252; color: #ffffff; }
QPushButton#dangerBtn:pressed { background-color: #b71c1c; padding: 6px 15px 4px 17px; }

QPushButton#successBtn { background-color: #0b3010; color: #4caf50; border-color: #4caf50; }
QPushButton#successBtn:hover { background-color: #4caf50; color: #ffffff; }
QPushButton#successBtn:pressed { background-color: #1b5e20; padding: 6px 15px 4px 17px; }

QPushButton#warnBtn    { background-color: #4a2300; color: #ff9800; border-color: #ff9800; }
QPushButton#warnBtn:hover { background-color: #ff9800; color: #ffffff; }
QPushButton#warnBtn:pressed { background-color: #e65100; padding: 6px 15px 4px 17px; }

QCheckBox { spacing: 6px; color: #e0e0e6; }
QCheckBox::indicator {
    width: 16px; height: 16px;
    border: 2px solid #dc143c;
    border-radius: 3px;
    background: #16161a;
}
QCheckBox::indicator:checked { background-color: #dc143c; }

QLineEdit {
    background: #16161a;
    border: 1px solid #3a2533;
    border-radius: 4px;
    padding: 4px 8px;
    color: #ffffff;
}
QLineEdit:focus { border-color: #ff4081; }

QPlainTextEdit, QTextEdit {
    background: #16161a;             /* Темное поле для кода/дампов */
    border: 1px solid #3a2533;
    border-radius: 4px;
    color: #e0e0e6;
    font-family: "Consolas", "Courier New", monospace;
    font-size: 9pt;
    selection-background-color: #dc143c;
    selection-color: #ffffff;
}
QPlainTextEdit:focus, QTextEdit:focus { border-color: #ff4081; }

QTableWidget {
    background-color: #16161a;
    alternate-background-color: #1c1c22; /* Чередование строк */
    gridline-color: #2a2a30;
    border: 1px solid #3a2533;
    border-radius: 4px;
    selection-background-color: #2a1b24;
    selection-color: #ff4081;
}
QHeaderView::section {
    background-color: #1a1a1f;
    color: #ff4081;                  /* Малиновые заголовки таблиц */
    padding: 5px 8px;
    border: none;
    border-right: 1px solid #2a2a30;
    border-bottom: 1px solid #3a2533;
    font-weight: bold;
    font-size: 8.5pt;
}
QHeaderView::section:last { border-right: none; }
QTableWidget::item { padding: 2px 6px; }

QTreeWidget {
    background: #16161a;
    border: 1px solid #3a2533;
    alternate-background-color: #1c1c22;
    color: #e0e0e6;
}
QTreeWidget::item:selected { background: #2a1b24; color: #ff4081; }

QSplitter::handle { background-color: #2a2a30; }
QSplitter::handle:horizontal { width: 3px; }
QSplitter::handle:vertical   { height: 3px; }

QGroupBox {
    border: 1px solid #3a2533;
    border-radius: 5px;
    margin-top: 8px;
    padding-top: 6px;
    font-weight: bold;
    color: #ff4081;                  /* Малиновые рамки групп */
}
QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 4px; }

QScrollBar:vertical { background: #121214; width: 10px; border-radius: 5px; }
QScrollBar::handle:vertical { background: #3a2533; border-radius: 5px; min-height: 20px; }
QScrollBar::handle:vertical:hover { background: #dc143c; }
QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }
QScrollBar:horizontal { background: #121214; height: 10px; border-radius: 5px; }
QScrollBar::handle:horizontal { background: #3a2533; border-radius: 5px; min-width: 20px; }
QScrollBar::handle:horizontal:hover { background: #dc143c; }
QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width: 0; }

QStatusBar {
    background: #1a1a1f;
    color: #a0a0a8;
    border-top: 1px solid #2a2a30;
    font-size: 8.5pt;
}
QStatusBar::item { border: none; }

QComboBox {
    background: #16161a;
    border: 1px solid #3a2533;
    border-radius: 4px;
    padding: 4px 8px;
    color: #e0e0e6;
}
QComboBox::drop-down { border: none; width: 20px; }
QComboBox QAbstractItemView {
    background: #16161a;
    border: 1px solid #3a2533;
    selection-background-color: #2a1b24;
    selection-color: #ff4081;
}

QLabel#titleLabel { font-size: 13pt; font-weight: bold; color: #ff4081; }
QLabel#interceptON  { color: #4caf50; font-weight: bold; }
QLabel#interceptOFF { color: #ff5252; font-weight: bold; }

QDockWidget {
    titlebar-close-icon: none;
    border: 1px solid #3a2533;
}
QDockWidget::title {
    background: #1a1a1f;
    color: #ff4081;
    padding: 5px 8px;
    font-weight: bold;
}
)";

// ──────────────────────────────────────────────────────────────────────────────
//  Constructor
// ──────────────────────────────────────────────────────────────────────────────
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("ProxyLab – HTTP Interception Suite");
    resize(1280, 820);
    setStyleSheet(STYLESHEET);

    // ── Storage ──
    m_storage = new StorageManager(this);
    QString dbPath = QDir(QStandardPaths::writableLocation(
                              QStandardPaths::AppDataLocation)).filePath("proxylab.sqlite");
    QDir().mkpath(QFileInfo(dbPath).path());
    try {
        m_storage->open(dbPath);
    } catch (const std::exception& ex) {
        // Non-fatal: the app can run without persistence
        qWarning("StorageManager: %s", ex.what());
    }

    // ── Proxy server ──
    m_server = new ProxyServer(this);
    connect(m_server, &ProxyServer::requestIntercepted,
            this, &MainWindow::onRequestIntercepted);
    connect(m_server, &ProxyServer::nextIntercepted,
            this, &MainWindow::onNextIntercepted);
    connect(m_server, &ProxyServer::requestFinished,
            this, &MainWindow::onRequestFinished);
    connect(m_server, &ProxyServer::queueChanged, this, [this](int total) {
        m_queueLabel->setText(QString("Queue: %1").arg(total));
    });
    connect(m_server, &ProxyServer::serverError, this, [this](const QString &msg) {
        statusBar()->showMessage("  ⚠  " + msg);
    });
    m_server->startListening(QHostAddress::LocalHost, 8080);

    // ── Repeater client ──
    m_repeaterClient = new RepeaterClient(this);
    connect(m_repeaterClient, &RepeaterClient::responseReceived,
            this, &MainWindow::onRepeaterResponse);
    connect(m_repeaterClient, &RepeaterClient::errorOccurred,
            this, &MainWindow::onRepeaterError);

    setupUI();
    setupCollectionsDock();

    // Restore history from DB
    auto history = m_storage->loadHistory();
    for (int i = history.size() - 1; i >= 0; --i) {
        m_traffic.append(history[i]);
        addHistoryRow(history[i]);
        addLogRow(history[i]);
    }

    // Clear proxy intercept panels on startup (no pending request)
    m_rawRequest->clear();
    m_rawResponse->clear();
    m_pendingConnId  = -1;
    m_awaitingRespId = -1;
    m_forwardBtn->setEnabled(false);
    m_dropBtn->setEnabled(false);
}

// ──────────────────────────────────────────────────────────────────────────────
//  UI Setup
// ──────────────────────────────────────────────────────────────────────────────
void MainWindow::setupUI()
{
    // Menu bar
    auto *mb = new QMenuBar(this);
    setMenuBar(mb);
    auto *fileMenu    = mb->addMenu("File");
    auto *projectMenu = mb->addMenu("Project");
    auto *toolsMenu   = mb->addMenu("Tools");
    auto *helpMenu    = mb->addMenu("Help");

    fileMenu->addAction("New Session", this, [this]() {
        m_traffic.clear();
        m_historyTable->setRowCount(0);
        m_logTable->setRowCount(0);
        m_storage->clearHistory();
        statusBar()->showMessage("  New session started");
    });
    fileMenu->addSeparator();
    fileMenu->addAction("Exit", qApp, &QApplication::quit);

    projectMenu->addAction("Clear History", this, &MainWindow::onClearHistory);
    projectMenu->addSeparator();
    projectMenu->addAction("New Collection Group…", this, &MainWindow::onNewGroup);
    projectMenu->addAction("Save Request to Group…", this, &MainWindow::onSaveToGroup);

    toolsMenu->addAction("Change Proxy Port…", this, [this]() {
        bool ok;
        int p = QInputDialog::getInt(this, "Proxy Port",
                                      "Listening port (restart required):",
                                      static_cast<int>(m_server->port()), 1, 65535, 1, &ok);
        if (ok) {
            m_server->close();
            m_server->startListening(QHostAddress::LocalHost, static_cast<quint16>(p));
            m_statusLabel->setText(QString("Proxy: 127.0.0.1:%1").arg(p));
            statusBar()->showMessage(QString("  Listening on 127.0.0.1:%1").arg(p));
        }
    });

    helpMenu->addAction("About ProxyLab", this, [this]() {
        QMessageBox::about(this, "ProxyLab",
                           "<b>ProxyLab v2.0</b><br>"
                           "HTTP Interception Suite<br><br>"
                           "Qt 5/6 + C++17<br>"
                           "Proxy · Repeater · Collections");
    });

    // Central
    auto *central    = new QWidget(this);
    setCentralWidget(central);
    auto *mainLayout = new QVBoxLayout(central);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    setupTopBar();
    mainLayout->addWidget(m_statusLabel->parentWidget());

    m_mainTabs = new QTabWidget(this);
    m_mainTabs->setTabPosition(QTabWidget::North);
    mainLayout->addWidget(m_mainTabs);

    setupProxyTab();
    setupRepeaterTab();
    setupLogTab();

    m_mainTabs->addTab(m_proxyTab,    "🔒  Proxy");
    m_mainTabs->addTab(m_repeaterTab, "🔁  Repeater");
    m_mainTabs->addTab(m_logTab,      "📋  HTTP Log");

    statusBar()->showMessage(
        QString("  ProxyLab ready  |  Listening on 127.0.0.1:%1  |  No target scope set")
            .arg(m_server->port()));
}

// ──────────────────────────────────────────────────────────────────────────────
//  Top bar
// ──────────────────────────────────────────────────────────────────────────────
void MainWindow::setupTopBar()
{
    auto *bar = new QWidget(this);
    bar->setFixedHeight(42);
    bar->setStyleSheet("background-color: #1a1a1f; border-bottom: 2px solid #dc143c;"); // Черный фон + малиновый нижний бортик

    auto *hl = new QHBoxLayout(bar);
    hl->setContentsMargins(12, 4, 12, 4);

    auto *title = new QLabel("⚡  ProxyLab", bar);
    title->setObjectName("titleLabel");

    m_statusLabel = new QLabel(QString("Proxy: 127.0.0.1:%1").arg(8080), bar);
    m_statusLabel->setStyleSheet("color: #a0a0a8; font-size: 9pt;");

    m_connCountLabel = new QLabel("Connections: 0", bar);
    m_connCountLabel->setStyleSheet("color: #bbdefb; font-size: 9pt;");

    m_interceptStatus = new QLabel("INTERCEPT: OFF", bar);
    m_interceptStatus->setStyleSheet(
        "color: #ff5252; font-weight: bold; font-size: 9pt; "
        "background:#2a1b24; padding: 3px 10px; border-radius: 3px; border: 1px solid #ff5252;");

    auto *spacer = new QWidget(bar);
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    hl->addWidget(title);
    hl->addSpacing(20);
    hl->addWidget(m_statusLabel);
    hl->addSpacing(24);
    hl->addWidget(m_connCountLabel);
    hl->addWidget(spacer);
    hl->addWidget(m_interceptStatus);
}

// ──────────────────────────────────────────────────────────────────────────────
//  Proxy Tab
// ──────────────────────────────────────────────────────────────────────────────
void MainWindow::setupProxyTab()
{
    m_proxyTab = new QWidget;
    auto *vl = new QVBoxLayout(m_proxyTab);
    vl->setContentsMargins(8, 8, 8, 8);
    vl->setSpacing(6);

    // Controls bar
    auto *ctrlBar = new QWidget;
    ctrlBar->setFixedHeight(40);
    ctrlBar->setStyleSheet("background:#1a1a1f; border: 1px solid #3a2533; border-radius:5px;");
    auto *hl = new QHBoxLayout(ctrlBar);
    hl->setContentsMargins(10, 4, 10, 4);

    m_interceptCheck = new QCheckBox("Intercept is OFF");
    m_interceptCheck->setChecked(false);
    connect(m_interceptCheck, &QCheckBox::toggled,
            this, &MainWindow::onInterceptToggled);

    m_forwardBtn = new QPushButton("▶  Forward");
    m_forwardBtn->setObjectName("successBtn");
    m_forwardBtn->setEnabled(false);
    connect(m_forwardBtn, &QPushButton::clicked, this, &MainWindow::onForwardClicked);

    m_dropBtn = new QPushButton("✖  Drop");
    m_dropBtn->setObjectName("dangerBtn");
    m_dropBtn->setEnabled(false);
    connect(m_dropBtn, &QPushButton::clicked, this, &MainWindow::onDropClicked);

    m_sendRepeaterBtn = new QPushButton("↗  Send to Repeater");
    m_sendRepeaterBtn->setEnabled(false);
    connect(m_sendRepeaterBtn, &QPushButton::clicked, this, &MainWindow::onSendToRepeater);

    m_queueLabel = new QLabel("Queue: 0");
    m_queueLabel->setStyleSheet("color:#ff4081; font-weight:bold; padding: 0 6px;");

    auto *filterLbl  = new QLabel("Filter:");
    auto *filterEdit = new QLineEdit;
    filterEdit->setPlaceholderText("host, path, status…");
    filterEdit->setFixedWidth(180);
    connect(filterEdit, &QLineEdit::textChanged, this, [this](const QString &text) {
        for (int r = 0; r < m_historyTable->rowCount(); ++r) {
            bool match = text.isEmpty();
            if (!match) {
                for (int c = 0; c < m_historyTable->columnCount(); ++c) {
                    auto *it = m_historyTable->item(r, c);
                    if (it && it->text().contains(text, Qt::CaseInsensitive)) {
                        match = true; break;
                    }
                }
            }
            m_historyTable->setRowHidden(r, !match);
        }
    });

    hl->addWidget(m_interceptCheck);
    hl->addSpacing(12);
    hl->addWidget(m_forwardBtn);
    hl->addWidget(m_dropBtn);
    hl->addSpacing(12);
    hl->addWidget(m_sendRepeaterBtn);
    hl->addSpacing(8);
    hl->addWidget(m_queueLabel);
    hl->addStretch();
    hl->addWidget(filterLbl);
    hl->addWidget(filterEdit);
    vl->addWidget(ctrlBar);

    // Vertical splitter: history / request+response
    auto *vsplit = new QSplitter(Qt::Vertical);

    // History table
    m_historyTable = new QTableWidget(0, 7);
    m_historyTable->setHorizontalHeaderLabels(
        {"#", "Time", "Method", "Host", "Path", "Status", "Size"});
    m_historyTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    m_historyTable->horizontalHeader()->setStretchLastSection(true);
    m_historyTable->setColumnWidth(0, 45);
    m_historyTable->setColumnWidth(1, 90);
    m_historyTable->setColumnWidth(2, 65);
    m_historyTable->setColumnWidth(3, 220);
    m_historyTable->setColumnWidth(4, 280);
    m_historyTable->setColumnWidth(5, 60);
    m_historyTable->setAlternatingRowColors(true);
    m_historyTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_historyTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_historyTable->verticalHeader()->setVisible(false);
    m_historyTable->setSortingEnabled(true);
    connect(m_historyTable, &QTableWidget::cellClicked,
            this, &MainWindow::onHistoryRowClicked);

    // Context menu on history table
    m_historyTable->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_historyTable, &QWidget::customContextMenuRequested,
            this, [this](const QPoint &pos) {
        int row = m_historyTable->rowAt(pos.y());
        if (row < 0) return;
        m_historyTable->selectRow(row);
        QMenu menu(this);
        menu.addAction("Send to Repeater", this, &MainWindow::onSendToRepeater);
        menu.addAction("Save to Collection…", this, &MainWindow::onSaveToGroup);
        menu.exec(m_historyTable->viewport()->mapToGlobal(pos));
    });

    vsplit->addWidget(m_historyTable);

    // Request / Response panels
    auto *hsplit = new QSplitter(Qt::Horizontal);

    auto *reqGroup = new QGroupBox("Request  (editable)");
    auto *reqVl    = new QVBoxLayout(reqGroup);
    m_rawRequest   = new QPlainTextEdit;
    m_rawRequest->setPlaceholderText("Intercepted request will appear here…");
    reqVl->addWidget(m_rawRequest);

    auto *resGroup  = new QGroupBox("Response");
    auto *resVl     = new QVBoxLayout(resGroup);
    m_rawResponse   = new QPlainTextEdit;
    m_rawResponse->setReadOnly(true);
    m_rawResponse->setPlaceholderText("Response will appear here after forwarding…");
    resVl->addWidget(m_rawResponse);

    hsplit->addWidget(reqGroup);
    hsplit->addWidget(resGroup);
    hsplit->setSizes({560, 560});

    vsplit->addWidget(hsplit);
    vsplit->setSizes({280, 360});
    vl->addWidget(vsplit);
}

// ──────────────────────────────────────────────────────────────────────────────
//  Repeater Tab
// ──────────────────────────────────────────────────────────────────────────────
void MainWindow::setupRepeaterTab()
{
    m_repeaterTab = new QWidget;
    auto *vl = new QVBoxLayout(m_repeaterTab);
    vl->setContentsMargins(8, 8, 8, 8);
    vl->setSpacing(6);

    auto *topBar = new QWidget;
    topBar->setFixedHeight(40);
    topBar->setStyleSheet("background:#1a1a1f; border: 1px solid #3a2533; border-radius:5px;");
    auto *hl = new QHBoxLayout(topBar);
    hl->setContentsMargins(10, 4, 10, 4);

    auto *hostLbl = new QLabel("Target:");
    hostLbl->setStyleSheet("color: #ff4081; font-weight: bold;");
    
    m_targetHost = new QLineEdit("target.example.com:443");
    m_targetHost->setFixedWidth(280);

    m_sslCheck = new QCheckBox("HTTPS");
    m_sslCheck->setChecked(true);

    m_sendBtn = new QPushButton("▶  Send");
    m_sendBtn->setObjectName("successBtn");
    m_sendBtn->setFixedWidth(90);
    connect(m_sendBtn, &QPushButton::clicked, this, &MainWindow::onRepeatRequest);

    auto *saveReqBtn = new QPushButton("💾  Save…");
    saveReqBtn->setFixedWidth(90);
    connect(saveReqBtn, &QPushButton::clicked, this, &MainWindow::onSaveToGroup);

    hl->addWidget(hostLbl);
    hl->addWidget(m_targetHost);
    hl->addWidget(m_sslCheck);
    hl->addSpacing(12);
    hl->addWidget(m_sendBtn);
    hl->addWidget(saveReqBtn);
    hl->addStretch();
    vl->addWidget(topBar);

    auto *hsplit = new QSplitter(Qt::Horizontal);

    auto *reqGroup = new QGroupBox("Request");
    auto *rvl      = new QVBoxLayout(reqGroup);
    m_repeaterRequest = new QPlainTextEdit;
    m_repeaterRequest->setPlaceholderText(
        "Paste or send a raw HTTP request here…\n\n"
        "GET /api/v1/users HTTP/1.1\r\nHost: example.com\r\n\r\n");
    rvl->addWidget(m_repeaterRequest);

    auto *resGroup = new QGroupBox("Response");
    auto *resvl    = new QVBoxLayout(resGroup);
    m_repeaterResponse = new QPlainTextEdit;
    m_repeaterResponse->setReadOnly(true);
    m_repeaterResponse->setPlaceholderText("Response will appear here after sending…");
    resvl->addWidget(m_repeaterResponse);

    hsplit->addWidget(reqGroup);
    hsplit->addWidget(resGroup);
    hsplit->setSizes({580, 580});
    vl->addWidget(hsplit);
}



// ──────────────────────────────────────────────────────────────────────────────
//  HTTP Log Tab  (with packet inspector)
// ──────────────────────────────────────────────────────────────────────────────
void MainWindow::setupLogTab()
{
    m_logTab = new QWidget;
    auto *vl = new QVBoxLayout(m_logTab);
    vl->setContentsMargins(8, 8, 8, 8);
    vl->setSpacing(6);

    auto *ctrlBar = new QWidget;
    ctrlBar->setFixedHeight(40);
    ctrlBar->setStyleSheet("background:#1a1a1f; border: 1px solid #3a2533; border-radius:5px;");
    auto *hl = new QHBoxLayout(ctrlBar);
    hl->setContentsMargins(10, 4, 10, 4);

    auto *searchLbl  = new QLabel("Search:");
    searchLbl->setStyleSheet("color: #ff4081; font-weight: bold;");
    
    auto *searchEdit = new QLineEdit;
    searchEdit->setPlaceholderText("filter by host / path / status…");
    searchEdit->setFixedWidth(260);
    connect(searchEdit, &QLineEdit::textChanged, this, [this](const QString &text) {
        for (int r = 0; r < m_logTable->rowCount(); ++r) {
            bool match = text.isEmpty();
            if (!match) {
                for (int c = 0; c < m_logTable->columnCount(); ++c) {
                    auto *it = m_logTable->item(r, c);
                    if (it && it->text().contains(text, Qt::CaseInsensitive)) {
                        match = true; break;
                    }
                }
            }
            m_logTable->setRowHidden(r, !match);
        }
    });

    m_clearLogBtn = new QPushButton("Clear");
    m_clearLogBtn->setObjectName("dangerBtn");
    m_clearLogBtn->setFixedWidth(80);
    connect(m_clearLogBtn, &QPushButton::clicked, this, &MainWindow::onClearLog);

    auto *pauseBtn = new QPushButton("⏸ Pause");
    pauseBtn->setFixedWidth(80);
    pauseBtn->setCheckable(true);
    connect(pauseBtn, &QPushButton::toggled, this, [this](bool on) {
        m_logPaused = on;
    });

    hl->addWidget(searchLbl);
    hl->addWidget(searchEdit);
    hl->addSpacing(12);
    hl->addWidget(pauseBtn);
    hl->addWidget(m_clearLogBtn);
    hl->addStretch();
    vl->addWidget(ctrlBar);

    auto *vsplit = new QSplitter(Qt::Vertical);

    m_logTable = new QTableWidget(0, 7);
    m_logTable->setHorizontalHeaderLabels(
        {"Time", "Method", "Host", "Path", "Status", "Size", "ID"});
    m_logTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    m_logTable->setColumnWidth(0, 90);
    m_logTable->setColumnWidth(1, 62);
    m_logTable->setColumnWidth(2, 220);
    m_logTable->setColumnWidth(3, 300);
    m_logTable->setColumnWidth(4, 58);
    m_logTable->setColumnWidth(5, 65);
    m_logTable->setColumnWidth(6, 45);
    m_logTable->setAlternatingRowColors(true);
    m_logTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_logTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_logTable->verticalHeader()->setVisible(false);
    m_logTable->setSortingEnabled(true);
    connect(m_logTable, &QTableWidget::cellClicked,
            this, &MainWindow::onLogRowClicked);
    vsplit->addWidget(m_logTable);

    auto *inspectorWidget = new QWidget;
    auto *ihl = new QHBoxLayout(inspectorWidget);
    ihl->setContentsMargins(0, 0, 0, 0);
    ihl->setSpacing(4);

    auto *reqGroup = new QGroupBox("Packet Inspector — Request");
    auto *rvl      = new QVBoxLayout(reqGroup);
    m_logInspectReq = new QPlainTextEdit;
    m_logInspectReq->setReadOnly(true);
    m_logInspectReq->setPlaceholderText("Click a row to inspect…");
    rvl->addWidget(m_logInspectReq);

    auto *resGroup  = new QGroupBox("Packet Inspector — Response");
    auto *resvl     = new QVBoxLayout(resGroup);
    m_logInspectResp = new QPlainTextEdit;
    m_logInspectResp->setReadOnly(true);
    m_logInspectResp->setPlaceholderText("Click a row to inspect…");
    resvl->addWidget(m_logInspectResp);

    ihl->addWidget(reqGroup);
    ihl->addWidget(resGroup);
    vsplit->addWidget(inspectorWidget);
    vsplit->setSizes({420, 220});
    vl->addWidget(vsplit);
}

// ──────────────────────────────────────────────────────────────────────────────
//  Collections dock  (Stage 5)
// ──────────────────────────────────────────────────────────────────────────────
void MainWindow::setupCollectionsDock()
{
    m_collectionsDock = new QDockWidget("📁  Collections", this);
    m_collectionsDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

    auto *dockContent = new QWidget;
    auto *dvl = new QVBoxLayout(dockContent);
    dvl->setContentsMargins(4, 4, 4, 4);
    dvl->setSpacing(4);

    auto *btnBar = new QWidget;
    auto *bhl    = new QHBoxLayout(btnBar);
    bhl->setContentsMargins(0, 0, 0, 0);

    auto *newGroupBtn = new QPushButton("+ Group");
    newGroupBtn->setFixedHeight(26);
    connect(newGroupBtn, &QPushButton::clicked, this, &MainWindow::onNewGroup);

    auto *saveBtn = new QPushButton("Save Req");
    saveBtn->setFixedHeight(26);
    connect(saveBtn, &QPushButton::clicked, this, &MainWindow::onSaveToGroup);

    bhl->addWidget(newGroupBtn);
    bhl->addWidget(saveBtn);
    dvl->addWidget(btnBar);

    m_collectionsTree = new QTreeWidget;
    m_collectionsTree->setHeaderLabel("Saved Collections");
    m_collectionsTree->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_collectionsTree, &QTreeWidget::itemClicked,
            this, &MainWindow::onCollectionItemClicked);
    connect(m_collectionsTree, &QWidget::customContextMenuRequested,
            this, [this](const QPoint &pos) {
        auto *item = m_collectionsTree->itemAt(pos);
        if (!item) return;
        QMenu menu(this);
        menu.addAction("Delete", this, &MainWindow::onCollectionDeleteAction);
        menu.exec(m_collectionsTree->viewport()->mapToGlobal(pos));
    });

    dvl->addWidget(m_collectionsTree);
    m_collectionsDock->setWidget(dockContent);
    addDockWidget(Qt::LeftDockWidgetArea, m_collectionsDock);
    m_collectionsDock->setMinimumWidth(200);
    m_collectionsDock->setMaximumWidth(300);

    refreshCollectionsTree();
}

void MainWindow::refreshCollectionsTree()
{
    m_collectionsTree->clear();
    auto groups = m_storage->loadGroups();
    for (const auto &g : groups) {
        auto *groupItem = new QTreeWidgetItem(m_collectionsTree);
        groupItem->setText(0, "📁  " + g.name);
        groupItem->setData(0, Qt::UserRole, QVariant::fromValue(QPoint(g.id, 0)));  // type=group

        auto requests = m_storage->loadRequests(g.id);
        for (const auto &r : requests) {
            auto *reqItem = new QTreeWidgetItem(groupItem);
            reqItem->setText(0, "📄  " + r.name);
            reqItem->setData(0, Qt::UserRole, QVariant::fromValue(QPoint(r.id, 1)));  // type=request
        }
        groupItem->setExpanded(true);
    }
}

// ──────────────────────────────────────────────────────────────────────────────
//  Helper: add a row to the history table
// ──────────────────────────────────────────────────────────────────────────────
void MainWindow::addHistoryRow(const TrafficRecord &rec)
{
    m_historyTable->setSortingEnabled(false);
    int row = m_historyTable->rowCount();
    m_historyTable->insertRow(row);

    m_historyTable->setItem(row, 0, new QTableWidgetItem(QString::number(rec.id)));
    m_historyTable->setItem(row, 1, new QTableWidgetItem(
        rec.time.toString("hh:mm:ss")));
    m_historyTable->setItem(row, 2, new QTableWidgetItem(rec.method));
    m_historyTable->setItem(row, 3, new QTableWidgetItem(rec.host));
    m_historyTable->setItem(row, 4, new QTableWidgetItem(rec.path));

    auto *statusItem = new QTableWidgetItem(
        rec.status > 0 ? QString::number(rec.status) : "—");
    if      (rec.status >= 500) statusItem->setForeground(QColor("#c62828"));
    else if (rec.status >= 400) statusItem->setForeground(QColor("#e65100"));
    else if (rec.status >= 300) statusItem->setForeground(QColor("#1565c0"));
    else if (rec.status >= 200) statusItem->setForeground(QColor("#2e7d32"));
    m_historyTable->setItem(row, 5, statusItem);
    m_historyTable->setItem(row, 6, new QTableWidgetItem(
        rec.size > 0 ? QString::number(rec.size) : "—"));

    m_historyTable->setSortingEnabled(true);
    m_historyTable->scrollToBottom();
}

// ──────────────────────────────────────────────────────────────────────────────
//  Helper: add a row to the HTTP Log table
// ──────────────────────────────────────────────────────────────────────────────
void MainWindow::addLogRow(const TrafficRecord &rec)
{
    if (m_logPaused) return;
    m_logTable->setSortingEnabled(false);
    int row = m_logTable->rowCount();
    m_logTable->insertRow(row);

    m_logTable->setItem(row, 0, new QTableWidgetItem(
        rec.time.toString("hh:mm:ss.zzz")));
    m_logTable->setItem(row, 1, new QTableWidgetItem(rec.method));
    m_logTable->setItem(row, 2, new QTableWidgetItem(rec.host));
    m_logTable->setItem(row, 3, new QTableWidgetItem(rec.path));

    auto *statusItem = new QTableWidgetItem(
        rec.status > 0 ? QString::number(rec.status) : "—");
    if      (rec.status >= 500) statusItem->setForeground(QColor("#c62828"));
    else if (rec.status >= 400) statusItem->setForeground(QColor("#e65100"));
    else if (rec.status >= 300) statusItem->setForeground(QColor("#1565c0"));
    else if (rec.status >= 200) statusItem->setForeground(QColor("#2e7d32"));
    m_logTable->setItem(row, 4, statusItem);
    m_logTable->setItem(row, 5, new QTableWidgetItem(
        rec.size > 0 ? QString::number(rec.size) : "—"));
    m_logTable->setItem(row, 6, new QTableWidgetItem(QString::number(rec.id)));

    m_logTable->setSortingEnabled(true);
    m_logTable->scrollToBottom();
    statusBar()->showMessage(
        QString("  HTTP Log – %1 requests captured").arg(m_logTable->rowCount()));
}

// ──────────────────────────────────────────────────────────────────────────────
//  Slots — Proxy tab
// ──────────────────────────────────────────────────────────────────────────────
void MainWindow::onInterceptToggled(bool checked)
{
    m_interceptOn = checked;
    m_server->setIntercept(checked);
    m_forwardBtn->setEnabled(checked && m_pendingConnId >= 0);
    m_dropBtn->setEnabled(checked && m_pendingConnId >= 0);
    // sendRepeaterBtn is NOT tied to intercept — it depends on having content
    m_interceptCheck->setText(checked ? "Intercept is ON" : "Intercept is OFF");

    if (checked) {
        m_interceptStatus->setText("INTERCEPT: ON");
        m_interceptStatus->setStyleSheet(
            "color: #4caf50; font-weight: bold; font-size: 9pt; "
            "background:#0b3010; padding: 3px 10px; border-radius: 3px; border: 1px solid #4caf50;");
    } else {
        m_interceptStatus->setText("INTERCEPT: OFF");
        m_interceptStatus->setStyleSheet(
            "color: #ff5252; font-weight: bold; font-size: 9pt; "
            "background:#2a1b24; padding: 3px 10px; border-radius: 3px; border: 1px solid #ff5252;");

        // The server's setIntercept(false) already called releaseIfHeld() on
        // the active connection and every queued one — they are all forwarded.
        // Reset the UI to idle: clear the intercept panel and counters.
        m_pendingConnId  = -1;
        m_awaitingRespId = -1;
        m_forwardBtn->setEnabled(false);
        m_dropBtn->setEnabled(false);
        m_rawRequest->clear();
        m_rawResponse->clear();
        m_rawResponse->setPlaceholderText("Response will appear here after forwarding…");
        m_queueLabel->setText("Queue: 0");
        statusBar()->showMessage("  Intercept OFF — all queued requests released");
    }
}

void MainWindow::onForwardClicked()
{
    if (m_pendingConnId < 0) return;

    QByteArray modified = m_rawRequest->toPlainText().toLatin1();

    // Always track the most-recently forwarded request so we can display its
    // response. If a previous response is still in-flight it will simply not
    // match m_awaitingRespId and be silently ignored in the log/history.
    int forwardedId  = m_pendingConnId;
    m_awaitingRespId = forwardedId;
    m_pendingConnId  = -1;

    // Disable the action buttons now; onRequestIntercepted will re-enable them
    // immediately if the server promotes a queued request synchronously.
    m_forwardBtn->setEnabled(false);
    m_dropBtn->setEnabled(false);
    m_sendRepeaterBtn->setEnabled(false);

    // Clear response area while we wait
    m_rawResponse->clear();
    m_rawResponse->setPlaceholderText("Waiting for response…");

    // This may call promoteNext() → emit requestIntercepted() synchronously,
    // which will call onRequestIntercepted() and re-enable the buttons before
    // we return — that is correct and expected.
    m_server->forwardPendingRequest(modified);

    // If no new request was promoted the status bar shows "waiting"; if one was
    // promoted onRequestIntercepted() already updated it.
    if (m_pendingConnId < 0) {
        statusBar()->showMessage(
            QString("  Request #%1 forwarded — waiting for response…").arg(forwardedId));
    }
}

void MainWindow::onDropClicked()
{
    if (m_pendingConnId < 0) return;

    m_pendingConnId = -1;
    m_server->dropPendingRequest();

    m_rawRequest->clear();
    m_rawResponse->clear();
    m_rawResponse->setPlaceholderText("Request dropped.");
    m_forwardBtn->setEnabled(false);
    m_dropBtn->setEnabled(false);
    m_sendRepeaterBtn->setEnabled(false);
    statusBar()->showMessage("  Request dropped");
}

void MainWindow::onSendToRepeater()
{
    QString raw;

    // Prefer live intercepted request
    if (m_pendingConnId >= 0)
        raw = m_rawRequest->toPlainText();

    // Fall back to selected history record
    if (raw.isEmpty() && m_selectedHistoryId >= 0) {
        for (const auto &rec : m_traffic) {
            if (rec.id == m_selectedHistoryId) {
                raw = rec.rawRequest;
                break;
            }
        }
    }

    if (raw.isEmpty()) {
        statusBar()->showMessage("  Nothing to send — select a request first");
        return;
    }

    m_repeaterRequest->setPlainText(raw);

    // Auto-populate target host from Host header
    int hostLine = raw.indexOf("Host:");
    if (hostLine < 0) hostLine = raw.indexOf("host:");
    if (hostLine >= 0) {
        int end = raw.indexOf('\n', hostLine);
        QString hostVal = raw.mid(hostLine + 5, end - hostLine - 5).trimmed();
        // Strip \r if present
        if (hostVal.endsWith('\r')) hostVal.chop(1);
        m_targetHost->setText(hostVal);
        m_sslCheck->setChecked(raw.startsWith("CONNECT") || hostVal.contains(":443"));
    }

    m_mainTabs->setCurrentIndex(1);
    statusBar()->showMessage("  Request sent to Repeater");
}

void MainWindow::onHistoryRowClicked(int row, int)
{
    auto *idItem = m_historyTable->item(row, 0);  // column 0 = ID
    if (!idItem) return;
    int id = idItem->text().toInt();

    for (const auto &rec : m_traffic) {
        if (rec.id == id) {
            m_selectedHistoryId = rec.id;
            m_sendRepeaterBtn->setEnabled(true);
            // Only show in proxy panels when no live intercepted request is pending
            // and we're not waiting on a forwarded response
            if (m_pendingConnId < 0 && m_awaitingRespId < 0) {
                m_rawRequest->setPlainText(rec.rawRequest);
                m_rawResponse->setPlainText(rec.rawResponse);
            }
            break;
        }
    }
}

// ──────────────────────────────────────────────────────────────────────────────
//  Slots — Log tab
// ──────────────────────────────────────────────────────────────────────────────
void MainWindow::onClearLog()
{
    m_logTable->setRowCount(0);
    m_logInspectReq->clear();
    m_logInspectResp->clear();
    statusBar()->showMessage("  Log cleared");
}

void MainWindow::onLogRowClicked(int row, int)
{
    auto *idItem = m_logTable->item(row, 6);
    if (!idItem) return;
    int id = idItem->text().toInt();
    for (const auto &rec : m_traffic) {
        if (rec.id == id) {
            m_logInspectReq->setPlainText(rec.rawRequest);
            m_logInspectResp->setPlainText(rec.rawResponse);
            break;
        }
    }
}

// ──────────────────────────────────────────────────────────────────────────────
//  Slots — Repeater tab
// ──────────────────────────────────────────────────────────────────────────────
void MainWindow::onRepeatRequest()
{
    QString raw = m_repeaterRequest->toPlainText().trimmed();
    if (raw.isEmpty()) {
        statusBar()->showMessage("  Repeater: nothing to send");
        return;
    }

    // If target field looks like placeholder, try to extract host from request
    QString hostPort = m_targetHost->text().trimmed();
    if (hostPort.isEmpty() || hostPort == "target.example.com:443" || hostPort == "target.example.com") {
        int hl = raw.indexOf("Host:");
        if (hl < 0) hl = raw.indexOf("host:");
        if (hl >= 0) {
            int end = raw.indexOf('\n', hl);
            hostPort = raw.mid(hl + 5, end - hl - 5).trimmed();
            if (hostPort.endsWith('\r')) hostPort.chop(1);
            m_targetHost->setText(hostPort);
        }
    }

    bool useSsl = m_sslCheck->isChecked();

    QString host;
    int port = useSsl ? 443 : 80;
    HttpParser::splitHostPort(hostPort, host, port, useSsl);

    if (host.isEmpty()) {
        statusBar()->showMessage("  Repeater: set Target host first");
        m_sendBtn->setEnabled(true);
        m_sendBtn->setText("▶  Send");
        return;
    }

    m_sendBtn->setEnabled(false);
    m_sendBtn->setText("Sending…");
    m_repeaterResponse->clear();
    m_repeaterResponse->setPlaceholderText("Waiting for response…");

    m_repeaterClient->send(raw.toLatin1(), host, port, useSsl);
}

void MainWindow::onRepeaterResponse(QByteArray raw)
{
    m_repeaterResponse->setPlainText(QString::fromLatin1(raw));
    m_sendBtn->setEnabled(true);
    m_sendBtn->setText("▶  Send");

    // Extract status for status bar
    int sp1 = raw.indexOf(' ');
    int sp2 = sp1 >= 0 ? raw.indexOf(' ', sp1 + 1) : -1;
    QString status = sp1 >= 0 && sp2 >= 0
                         ? raw.mid(sp1 + 1, sp2 - sp1 - 1)
                         : QByteArray("?"); // Исправлено: привели к общему типу QByteArray
    statusBar()->showMessage(
        QString("  Repeater: %1 bytes  status %2").arg(raw.size()).arg(status));
}


void MainWindow::onRepeaterError(QString msg)
{
    m_repeaterResponse->setPlainText("ERROR: " + msg);
    m_sendBtn->setEnabled(true);
    m_sendBtn->setText("▶  Send");
    statusBar()->showMessage("  Repeater error: " + msg);
}

// ──────────────────────────────────────────────────────────────────────────────
//  Slots — Proxy server callbacks
// ──────────────────────────────────────────────────────────────────────────────
void MainWindow::onRequestIntercepted(int connId, QByteArray rawRequest)
{
    m_pendingConnId = connId;
    // Do NOT reset m_awaitingRespId here — the previous forwarded connection
    // may still come back with a response. The UI shows the new intercept but
    // also accepts the response when it arrives.

    m_rawRequest->setPlainText(QString::fromLatin1(rawRequest));
    m_rawResponse->clear();
    m_rawResponse->setPlaceholderText("Request intercepted — edit above, then Forward or Drop");
    m_mainTabs->setCurrentIndex(0);  // show Proxy tab

    // Always enable action buttons for the newly intercepted request
    m_forwardBtn->setEnabled(true);
    m_dropBtn->setEnabled(true);
    m_sendRepeaterBtn->setEnabled(true);

    // Update queue label: 1 (currently displayed) + items still waiting
    int total = 1 + m_server->queueDepth();
    m_queueLabel->setText(QString("Queue: %1").arg(total));

    statusBar()->showMessage(
        QString("  ⏸  Request intercepted  (conn #%1)  —  %2 in queue")
            .arg(connId).arg(total));
}

void MainWindow::onNextIntercepted(int connId, QByteArray rawRequest)
{
    if (connId == -1) {
        // Queue drained
        m_queueLabel->setText("Queue: 0");
        return;
    }
    if (connId == -2) {
        // A new request arrived but another is already shown — just update counter
        int total = 1 + m_server->queueDepth();
        m_queueLabel->setText(QString("Queue: %1").arg(total));
        statusBar()->showMessage(
            QString("  ⏸  %1 request(s) waiting — forward or drop current").arg(total));
        return;
    }
    // Normal promote: handled by onRequestIntercepted re-emit from server
    Q_UNUSED(rawRequest)
}

void MainWindow::onRequestFinished(TrafficRecord record)
{
    m_traffic.append(record);
    m_storage->saveRecord(record);
    addHistoryRow(record);
    addLogRow(record);

    // Show response in proxy panel if this was the request we forwarded
    if (record.id == m_awaitingRespId) {
        m_rawResponse->setPlainText(record.rawResponse);
        m_awaitingRespId = -1;

        // If a new intercept has already taken over the panel, leave buttons
        // as onRequestIntercepted set them. Otherwise we are idle — re-enable
        // "Send to Repeater" so the user can push the response to Repeater.
        if (m_pendingConnId < 0) {
            m_sendRepeaterBtn->setEnabled(true);
            // Forward/Drop stay disabled — there is nothing pending to act on.
            m_forwardBtn->setEnabled(false);
            m_dropBtn->setEnabled(false);
        }

        statusBar()->showMessage(
            QString("  ✓  Response received  (conn #%1)  %2 bytes  status %3")
                .arg(record.id)
                .arg(record.size)
                .arg(record.status > 0 ? QString::number(record.status) : "?"));
    }
}


// ──────────────────────────────────────────────────────────────────────────────
//  Slots — Collections
// ──────────────────────────────────────────────────────────────────────────────
void MainWindow::onNewGroup()
{
    bool ok;
    QString name = QInputDialog::getText(
        this, "New Collection Group", "Group name:", QLineEdit::Normal, "Group 1", &ok);
    if (!ok || name.trimmed().isEmpty()) return;
    m_storage->createGroup(name.trimmed());
    refreshCollectionsTree();
    statusBar()->showMessage("  Collection group '" + name + "' created");
}

void MainWindow::onSaveToGroup()
{
    // Gather groups
    auto groups = m_storage->loadGroups();
    if (groups.isEmpty()) {
        QMessageBox::information(this, "No Groups",
                                  "Create a collection group first (Project → New Collection Group).");
        return;
    }

    QStringList groupNames;
    for (const auto &g : groups) groupNames << g.name;

    bool ok;
    QString groupName = QInputDialog::getItem(
        this, "Save to Collection", "Choose group:", groupNames, 0, false, &ok);
    if (!ok) return;

    int groupId = -1;
    for (const auto &g : groups)
        if (g.name == groupName) { groupId = g.id; break; }
    if (groupId < 0) return;

    QString reqName = QInputDialog::getText(
        this, "Request Name", "Name:", QLineEdit::Normal, "Request", &ok);
    if (!ok || reqName.trimmed().isEmpty()) return;

    // Get raw request from whichever tab is active
    QString raw;
    int tab = m_mainTabs->currentIndex();
    if (tab == 0) raw = m_rawRequest->toPlainText();
    else if (tab == 1) raw = m_repeaterRequest->toPlainText();
    if (raw.isEmpty()) {
        // Fall back to selected history row
        int row = m_historyTable->currentRow();
        if (row >= 0 && row < m_traffic.size())
            raw = m_traffic[row].rawRequest;
    }
    if (raw.isEmpty()) {
        statusBar()->showMessage("  Nothing to save — no request selected");
        return;
    }

    QString host   = m_targetHost->text().trimmed();
    bool    useSsl = m_sslCheck->isChecked();
    QString h; int p = useSsl ? 443 : 80;
    HttpParser::splitHostPort(host, h, p, useSsl);

    m_storage->saveRequest(groupId, reqName.trimmed(), raw, h, p, useSsl);
    refreshCollectionsTree();
    statusBar()->showMessage("  Saved '" + reqName + "' to '" + groupName + "'");
}

void MainWindow::onCollectionItemClicked(QTreeWidgetItem *item, int)
{
    if (!item) return;
    QPoint data = item->data(0, Qt::UserRole).value<QPoint>();
    int type = data.y();  // 0=group, 1=request
    if (type != 1) return;

    int reqId = data.x();
    SavedRequest r = m_storage->loadRequest(reqId);
    m_repeaterRequest->setPlainText(r.rawRequest);
    m_targetHost->setText(r.host + ":" + QString::number(r.port));
    m_sslCheck->setChecked(r.useSsl);
    m_mainTabs->setCurrentIndex(1);
    statusBar()->showMessage("  Loaded '" + r.name + "' into Repeater");
}

void MainWindow::onCollectionDeleteAction()
{
    auto *item = m_collectionsTree->currentItem();
    if (!item) return;
    QPoint data = item->data(0, Qt::UserRole).value<QPoint>();
    int id   = data.x();
    int type = data.y();

    auto res = QMessageBox::question(this, "Delete",
        QString("Delete this %1?").arg(type == 0 ? "group and all its requests" : "request"),
        QMessageBox::Yes | QMessageBox::No);
    if (res != QMessageBox::Yes) return;

    if (type == 0) m_storage->deleteGroup(id);
    else            m_storage->deleteRequest(id);
    refreshCollectionsTree();
}

void MainWindow::onClearHistory()
{
    auto res = QMessageBox::question(this, "Clear History",
        "Clear all captured traffic? This also removes saved DB entries.",
        QMessageBox::Yes | QMessageBox::No);
    if (res != QMessageBox::Yes) return;
    m_traffic.clear();
    m_historyTable->setRowCount(0);
    m_logTable->setRowCount(0);
    m_logInspectReq->clear();
    m_logInspectResp->clear();
    m_rawRequest->clear();
    m_rawResponse->clear();
    m_pendingConnId  = -1;
    m_awaitingRespId = -1;
    m_selectedHistoryId = -1;
    m_forwardBtn->setEnabled(false);
    m_dropBtn->setEnabled(false);
    m_sendRepeaterBtn->setEnabled(false);
    m_storage->clearHistory();
    statusBar()->showMessage("  History cleared");
}

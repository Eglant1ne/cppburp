#include "mainwindow.h"
#include <QApplication>
#include <QDateTime>
#include <QStringList>

// ──────────────────────────────────────────────
//  Stylesheet  (white / blue palette)
// ──────────────────────────────────────────────
static const char *STYLESHEET = R"(
/* ── Base ── */
QMainWindow, QWidget {
    background-color: #f0f4fa;
    color: #1a2a3a;
    font-family: "Segoe UI";
    font-size: 9pt;
}

/* ── Menu bar ── */
QMenuBar {
    background-color: #1565c0;
    color: #ffffff;
    padding: 2px 4px;
    font-size: 9pt;
}
QMenuBar::item:selected { background-color: #1976d2; border-radius: 3px; }
QMenu {
    background-color: #ffffff;
    border: 1px solid #90caf9;
    color: #1a2a3a;
}
QMenu::item:selected { background-color: #e3f2fd; }

/* ── Tab widget ── */
QTabWidget::pane {
    border: 1px solid #90caf9;
    border-top: none;
    background: #ffffff;
    border-radius: 0 0 4px 4px;
}
QTabBar::tab {
    background: #dce8f8;
    color: #1a2a3a;
    padding: 6px 18px;
    border: 1px solid #90caf9;
    border-bottom: none;
    margin-right: 2px;
    border-radius: 4px 4px 0 0;
    font-size: 9pt;
}
QTabBar::tab:selected {
    background: #1565c0;
    color: #ffffff;
    font-weight: bold;
}
QTabBar::tab:hover:!selected { background: #bbdefb; }

/* inner tabs (e.g. Request/Response) */
QTabWidget#innerTabs::pane {
    border: 1px solid #bbdefb;
    background: #fafdff;
}
QTabBar#innerBar::tab {
    background: #e8f0fe;
    color: #1565c0;
    padding: 4px 14px;
    border: 1px solid #bbdefb;
    border-bottom: none;
    margin-right: 1px;
    border-radius: 4px 4px 0 0;
    font-size: 8.5pt;
}
QTabBar#innerBar::tab:selected {
    background: #1565c0;
    color: #ffffff;
}

/* ── Buttons ── */
QPushButton {
    background-color: #1565c0;
    color: #ffffff;
    border: none;
    padding: 5px 16px;
    border-radius: 4px;
    font-size: 9pt;
}
QPushButton:hover  { background-color: #1976d2; }
QPushButton:pressed { background-color: #0d47a1; }
QPushButton:disabled { background-color: #90caf9; color: #e3f2fd; }

QPushButton#dangerBtn {
    background-color: #c62828;
}
QPushButton#dangerBtn:hover { background-color: #e53935; }

QPushButton#successBtn {
    background-color: #2e7d32;
}
QPushButton#successBtn:hover { background-color: #388e3c; }

/* ── CheckBox ── */
QCheckBox { spacing: 6px; }
QCheckBox::indicator {
    width: 16px; height: 16px;
    border: 2px solid #1565c0;
    border-radius: 3px;
    background: #ffffff;
}
QCheckBox::indicator:checked {
    background-color: #1565c0;
    image: none;
}

/* ── LineEdit ── */
QLineEdit {
    background: #ffffff;
    border: 1px solid #90caf9;
    border-radius: 4px;
    padding: 4px 8px;
    color: #1a2a3a;
}
QLineEdit:focus { border-color: #1565c0; }

/* ── PlainTextEdit / TextEdit ── */
QPlainTextEdit, QTextEdit {
    background: #ffffff;
    border: 1px solid #90caf9;
    border-radius: 4px;
    color: #1a2a3a;
    font-family: "Consolas", "Courier New", monospace;
    font-size: 9pt;
    selection-background-color: #bbdefb;
}
QPlainTextEdit:focus, QTextEdit:focus { border-color: #1565c0; }

/* ── Table ── */
QTableWidget {
    background-color: #ffffff;
    alternate-background-color: #e8f0fe;
    gridline-color: #c5d8f7;
    border: 1px solid #90caf9;
    border-radius: 4px;
    selection-background-color: #bbdefb;
    selection-color: #0d47a1;
}
QHeaderView::section {
    background-color: #1565c0;
    color: #ffffff;
    padding: 5px 8px;
    border: none;
    border-right: 1px solid #1976d2;
    font-weight: bold;
    font-size: 8.5pt;
}
QHeaderView::section:last { border-right: none; }
QTableWidget::item { padding: 2px 6px; }

/* ── TreeWidget ── */
QTreeWidget {
    background: #ffffff;
    border: 1px solid #90caf9;
    alternate-background-color: #e8f0fe;
}
QTreeWidget::item:selected { background: #bbdefb; color: #0d47a1; }

/* ── Splitter ── */
QSplitter::handle { background-color: #90caf9; }
QSplitter::handle:horizontal { width: 3px; }
QSplitter::handle:vertical   { height: 3px; }

/* ── GroupBox ── */
QGroupBox {
    border: 1px solid #90caf9;
    border-radius: 5px;
    margin-top: 8px;
    padding-top: 6px;
    font-weight: bold;
    color: #1565c0;
}
QGroupBox::title {
    subcontrol-origin: margin;
    left: 10px;
    padding: 0 4px;
}

/* ── ScrollBar ── */
QScrollBar:vertical {
    background: #e8f0fe;
    width: 10px;
    border-radius: 5px;
}
QScrollBar::handle:vertical {
    background: #90caf9;
    border-radius: 5px;
    min-height: 20px;
}
QScrollBar::handle:vertical:hover { background: #1565c0; }
QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }
QScrollBar:horizontal {
    background: #e8f0fe;
    height: 10px;
    border-radius: 5px;
}
QScrollBar::handle:horizontal {
    background: #90caf9;
    border-radius: 5px;
    min-width: 20px;
}
QScrollBar::handle:horizontal:hover { background: #1565c0; }
QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width: 0; }

/* ── Status bar ── */
QStatusBar {
    background: #1565c0;
    color: #ffffff;
    border-top: 1px solid #0d47a1;
    font-size: 8.5pt;
}
QStatusBar::item { border: none; }

/* ── ComboBox ── */
QComboBox {
    background: #ffffff;
    border: 1px solid #90caf9;
    border-radius: 4px;
    padding: 4px 8px;
    color: #1a2a3a;
}
QComboBox::drop-down { border: none; width: 20px; }
QComboBox QAbstractItemView {
    background: #ffffff;
    border: 1px solid #90caf9;
    selection-background-color: #bbdefb;
}

/* ── Label ── */
QLabel#titleLabel {
    font-size: 13pt;
    font-weight: bold;
    color: #1565c0;
}
QLabel#interceptON  { color: #2e7d32; font-weight: bold; }
QLabel#interceptOFF { color: #c62828; font-weight: bold; }
)";

// ──────────────────────────────────────────────
//  Helpers
// ──────────────────────────────────────────────
static const QString SAMPLE_REQUEST =
    "GET /api/v1/users HTTP/1.1\r\n"
    "Host: target.example.com\r\n"
    "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64)\r\n"
    "Accept: application/json\r\n"
    "Authorization: Bearer eyJhbGciOiJIUzI1NiJ9.eyJzdWIiOiJ1c2VyMSJ9.xxx\r\n"
    "Connection: keep-alive\r\n"
    "\r\n";

static const QString SAMPLE_RESPONSE =
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: application/json\r\n"
    "Content-Length: 142\r\n"
    "Server: nginx/1.24.0\r\n"
    "X-Frame-Options: DENY\r\n"
    "\r\n"
    "{\r\n"
    "  \"users\": [\r\n"
    "    {\"id\": 1, \"name\": \"Alice\", \"role\": \"admin\"},\r\n"
    "    {\"id\": 2, \"name\": \"Bob\",   \"role\": \"user\"}\r\n"
    "  ]\r\n"
    "}\r\n";

// ──────────────────────────────────────────────
//  Constructor
// ──────────────────────────────────────────────
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("ProxyLab – HTTP Interception Suite");
    resize(1200, 780);
    applyStyleSheet();
    setupUI();

}

void MainWindow::applyStyleSheet()
{
    setStyleSheet(STYLESHEET);
}

// ──────────────────────────────────────────────
//  setupUI
// ──────────────────────────────────────────────
void MainWindow::setupUI()
{
    // ── Menu bar ──
    auto *menuBar = new QMenuBar(this);
    setMenuBar(menuBar);
    auto *fileMenu    = menuBar->addMenu("File");
    auto *projectMenu = menuBar->addMenu("Project");
    auto *toolsMenu   = menuBar->addMenu("Tools");
    auto *helpMenu    = menuBar->addMenu("Help");
    fileMenu->addAction("New Project");
    fileMenu->addAction("Open…");
    fileMenu->addSeparator();
    fileMenu->addAction("Exit", qApp, &QApplication::quit);
    projectMenu->addAction("Settings");
    toolsMenu->addAction("Decoder");
    toolsMenu->addAction("Comparer");
    helpMenu->addAction("About");

    // ── Central widget ──
    auto *central = new QWidget(this);
    setCentralWidget(central);
    auto *mainLayout = new QVBoxLayout(central);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    setupTopBar();
    mainLayout->addWidget(m_statusLabel->parentWidget()); // top bar widget

    m_mainTabs = new QTabWidget(this);
    m_mainTabs->setTabPosition(QTabWidget::North);
    mainLayout->addWidget(m_mainTabs);

    setupProxyTab();
    setupRepeaterTab();
    setupScannerTab();
    setupLogTab();

    m_mainTabs->addTab(m_proxyTab,    "🔒  Proxy");
    m_mainTabs->addTab(m_repeaterTab, "🔁  Repeater");
    m_mainTabs->addTab(m_scannerTab,  "🔍  Scanner");
    m_mainTabs->addTab(m_logTab,      "📋  HTTP Log");

    // ── Status bar ──
    statusBar()->showMessage("  ProxyLab ready  |  Listening on 127.0.0.1:8080  |  No target scope set");
}

// ──────────────────────────────────────────────
//  Top bar
// ──────────────────────────────────────────────
void MainWindow::setupTopBar()
{
    auto *bar = new QWidget(this);
    bar->setFixedHeight(42);
    bar->setStyleSheet("background-color: #1565c0;");

    auto *hl = new QHBoxLayout(bar);
    hl->setContentsMargins(12, 4, 12, 4);

    auto *title = new QLabel("⚡  ProxyLab", bar);
    title->setObjectName("titleLabel");
    title->setStyleSheet("font-size: 13pt; font-weight: bold; color: #ffffff;");

    m_statusLabel = new QLabel("Proxy: 127.0.0.1:8080", bar);
    m_statusLabel->setStyleSheet("color: #bbdefb; font-size: 9pt;");

    m_interceptStatus = new QLabel("INTERCEPT: OFF", bar);
    m_interceptStatus->setObjectName("interceptOFF");
    m_interceptStatus->setStyleSheet("color: #ef9a9a; font-weight: bold; font-size: 9pt; "
                                     "background:#0d47a1; padding: 3px 10px; border-radius: 3px;");

    auto *spacer = new QWidget(bar);
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    hl->addWidget(title);
    hl->addSpacing(20);
    hl->addWidget(m_statusLabel);
    hl->addWidget(spacer);
    hl->addWidget(m_interceptStatus);
}

// ──────────────────────────────────────────────
//  Proxy Tab
// ──────────────────────────────────────────────
void MainWindow::setupProxyTab()
{
    m_proxyTab = new QWidget;
    auto *vl = new QVBoxLayout(m_proxyTab);
    vl->setContentsMargins(8, 8, 8, 8);
    vl->setSpacing(6);

    // ── Controls bar ──
    auto *ctrlBar = new QWidget;
    ctrlBar->setFixedHeight(40);
    ctrlBar->setStyleSheet("background:#dce8f8; border-radius:5px;");
    auto *hl = new QHBoxLayout(ctrlBar);
    hl->setContentsMargins(10, 4, 10, 4);

    m_interceptCheck = new QCheckBox("Intercept is ON");
    m_interceptCheck->setChecked(false);
    connect(m_interceptCheck, &QCheckBox::toggled, this, &MainWindow::onInterceptToggled);

    m_forwardBtn = new QPushButton("Forward");
    m_forwardBtn->setObjectName("successBtn");
    m_forwardBtn->setEnabled(false);
    connect(m_forwardBtn, &QPushButton::clicked, this, &MainWindow::onForwardClicked);

    m_dropBtn = new QPushButton("Drop");
    m_dropBtn->setObjectName("dangerBtn");
    m_dropBtn->setEnabled(false);
    connect(m_dropBtn, &QPushButton::clicked, this, &MainWindow::onDropClicked);

    m_sendRepeaterBtn = new QPushButton("Send to Repeater");
    m_sendRepeaterBtn->setEnabled(false);
    connect(m_sendRepeaterBtn, &QPushButton::clicked, this, &MainWindow::onSendToRepeater);

    auto *filterLbl = new QLabel("Filter:");
    auto *filterEdit = new QLineEdit;
    filterEdit->setPlaceholderText("host, path, status…");
    filterEdit->setFixedWidth(180);

    hl->addWidget(m_interceptCheck);
    hl->addSpacing(12);
    hl->addWidget(m_forwardBtn);
    hl->addWidget(m_dropBtn);
    hl->addSpacing(12);
    hl->addWidget(m_sendRepeaterBtn);
    hl->addStretch();
    hl->addWidget(filterLbl);
    hl->addWidget(filterEdit);

    vl->addWidget(ctrlBar);

    // ── Vertical splitter: history / request+response ──
    auto *vsplit = new QSplitter(Qt::Vertical);

    // History table
    m_historyTable = new QTableWidget(0, 6);
    m_historyTable->setHorizontalHeaderLabels({"#", "Method", "Host", "Path", "Status", "Length"});
    m_historyTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    m_historyTable->horizontalHeader()->setStretchLastSection(true);
    m_historyTable->setColumnWidth(0, 45);
    m_historyTable->setColumnWidth(1, 65);
    m_historyTable->setColumnWidth(2, 220);
    m_historyTable->setColumnWidth(3, 300);
    m_historyTable->setColumnWidth(4, 60);
    m_historyTable->setAlternatingRowColors(true);
    m_historyTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_historyTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_historyTable->verticalHeader()->setVisible(false);

    connect(m_historyTable, &QTableWidget::cellClicked, this, [this](int row, int) {
        m_rawRequest->setPlainText(SAMPLE_REQUEST);
        m_rawResponse->setPlainText(SAMPLE_RESPONSE);
    });

    vsplit->addWidget(m_historyTable);

    // Request / Response split
    auto *hsplit = new QSplitter(Qt::Horizontal);

    auto *reqGroup = new QGroupBox("Request");
    auto *reqVl = new QVBoxLayout(reqGroup);
    m_rawRequest = new QPlainTextEdit;
    m_rawRequest->setPlainText(SAMPLE_REQUEST);
    reqVl->addWidget(m_rawRequest);

    auto *resGroup = new QGroupBox("Response");
    auto *resVl = new QVBoxLayout(resGroup);
    m_rawResponse = new QPlainTextEdit;
    m_rawResponse->setPlainText(SAMPLE_RESPONSE);
    m_rawResponse->setReadOnly(true);
    resVl->addWidget(m_rawResponse);

    hsplit->addWidget(reqGroup);
    hsplit->addWidget(resGroup);
    hsplit->setSizes({550, 550});

    vsplit->addWidget(hsplit);
    vsplit->setSizes({260, 340});

    vl->addWidget(vsplit);
}

// ──────────────────────────────────────────────
//  Repeater Tab
// ──────────────────────────────────────────────
void MainWindow::setupRepeaterTab()
{
    m_repeaterTab = new QWidget;
    auto *vl = new QVBoxLayout(m_repeaterTab);
    vl->setContentsMargins(8, 8, 8, 8);
    vl->setSpacing(6);

    // Top bar
    auto *topBar = new QWidget;
    topBar->setFixedHeight(40);
    topBar->setStyleSheet("background:#dce8f8; border-radius:5px;");
    auto *hl = new QHBoxLayout(topBar);
    hl->setContentsMargins(10, 4, 10, 4);

    auto *hostLbl = new QLabel("Target:");
    m_targetHost = new QLineEdit("target.example.com:443");
    m_targetHost->setFixedWidth(260);

    auto *sslCheck = new QCheckBox("HTTPS");
    sslCheck->setChecked(true);

    m_sendBtn = new QPushButton("▶  Send");
    m_sendBtn->setObjectName("successBtn");
    m_sendBtn->setFixedWidth(90);
    connect(m_sendBtn, &QPushButton::clicked, this, &MainWindow::onRepeatRequest);

    hl->addWidget(hostLbl);
    hl->addWidget(m_targetHost);
    hl->addWidget(sslCheck);
    hl->addSpacing(12);
    hl->addWidget(m_sendBtn);
    hl->addStretch();

    vl->addWidget(topBar);

    // Split
    auto *hsplit = new QSplitter(Qt::Horizontal);

    auto *reqGroup = new QGroupBox("Request");
    auto *rvl = new QVBoxLayout(reqGroup);
    m_repeaterRequest = new QPlainTextEdit;
    m_repeaterRequest->setPlainText(SAMPLE_REQUEST);
    rvl->addWidget(m_repeaterRequest);

    auto *resGroup = new QGroupBox("Response");
    auto *resvl = new QVBoxLayout(resGroup);
    m_repeaterResponse = new QPlainTextEdit;
    m_repeaterResponse->setReadOnly(true);
    m_repeaterResponse->setPlaceholderText("Response will appear here after sending…");
    resvl->addWidget(m_repeaterResponse);

    hsplit->addWidget(reqGroup);
    hsplit->addWidget(resGroup);
    hsplit->setSizes({560, 560});

    vl->addWidget(hsplit);
}

// ──────────────────────────────────────────────
//  Scanner Tab
// ──────────────────────────────────────────────
void MainWindow::setupScannerTab()
{
    m_scannerTab = new QWidget;
    auto *vl = new QVBoxLayout(m_scannerTab);
    vl->setContentsMargins(8, 8, 8, 8);
    vl->setSpacing(6);

    // Controls
    auto *ctrlBar = new QWidget;
    ctrlBar->setFixedHeight(40);
    ctrlBar->setStyleSheet("background:#dce8f8; border-radius:5px;");
    auto *hl = new QHBoxLayout(ctrlBar);
    hl->setContentsMargins(10, 4, 10, 4);

    auto *urlLbl = new QLabel("Scan URL:");
    auto *urlEdit = new QLineEdit("https://target.example.com");
    urlEdit->setFixedWidth(320);

    auto *scopeCombo = new QComboBox;
    scopeCombo->addItems({"Active Scan", "Passive Scan", "Crawl Only"});

    m_startScanBtn = new QPushButton("▶  Start Scan");
    m_startScanBtn->setObjectName("successBtn");

    connect(m_startScanBtn, &QPushButton::clicked, this, [this]() {
        // Add mock findings
        static const QStringList issueTitles = {
            "SQL Injection", "XSS (Reflected)", "Missing HSTS Header",
            "Weak TLS Cipher", "Open Redirect", "CSRF Token Absent",
            "Directory Listing", "Information Disclosure"
        };
        static const QStringList severities = {
            "🔴 High", "🔴 High", "🟡 Medium",
            "🟡 Medium", "🟠 High", "🟡 Medium",
            "🟢 Low", "🟡 Medium"
        };
        static const QStringList paths = {
            "/api/login", "/search?q=", "/", "/", "/redirect?url=",
            "/account/update", "/uploads/", "/api/debug"
        };
        m_scanResults->clear();
        for (int i = 0; i < issueTitles.size(); ++i) {
            auto *item = new QTreeWidgetItem(m_scanResults);
            item->setText(0, severities[i]);
            item->setText(1, issueTitles[i]);
            item->setText(2, "target.example.com");
            item->setText(3, paths[i]);
        }
        m_scanDetail->setHtml(
            "<p><b>SQL Injection</b></p>"
            "<p><b>Severity:</b> High &nbsp; <b>Confidence:</b> Certain</p>"
            "<p><b>URL:</b> https://target.example.com/api/login</p>"
            "<p><b>Parameter:</b> username</p>"
            "<hr>"
            "<p>The application appears to be vulnerable to SQL injection. "
            "The payload <code>' OR '1'='1</code> caused a database error.</p>"
            "<p><b>Remediation:</b> Use parameterised queries / prepared statements.</p>"
        );
        statusBar()->showMessage("  Scan complete – 8 issues found");
    });

    hl->addWidget(urlLbl);
    hl->addWidget(urlEdit);
    hl->addWidget(scopeCombo);
    hl->addSpacing(12);
    hl->addWidget(m_startScanBtn);
    hl->addStretch();
    vl->addWidget(ctrlBar);

    // Split: tree / detail
    auto *hsplit = new QSplitter(Qt::Horizontal);

    auto *treeGroup = new QGroupBox("Issues");
    auto *tvl = new QVBoxLayout(treeGroup);
    m_scanResults = new QTreeWidget;
    m_scanResults->setHeaderLabels({"Severity", "Issue", "Host", "Path"});
    m_scanResults->setColumnWidth(0, 90);
    m_scanResults->setColumnWidth(1, 200);
    m_scanResults->setColumnWidth(2, 180);
    m_scanResults->setAlternatingRowColors(true);
    connect(m_scanResults, &QTreeWidget::itemClicked, this, [this](QTreeWidgetItem *item, int) {
        m_scanDetail->setHtml(
            "<p><b>" + item->text(1) + "</b></p>"
            "<p><b>Severity:</b> " + item->text(0) + " &nbsp; <b>Host:</b> " + item->text(2) + "</p>"
            "<p><b>Path:</b> " + item->text(3) + "</p>"
            "<hr>"
            "<p>Click <i>Start Scan</i> for a full description and remediation advice.</p>"
        );
    });
    tvl->addWidget(m_scanResults);

    auto *detailGroup = new QGroupBox("Advisory");
    auto *dvl = new QVBoxLayout(detailGroup);
    m_scanDetail = new QTextEdit;
    m_scanDetail->setReadOnly(true);
    m_scanDetail->setPlaceholderText("Select an issue to see details…");
    dvl->addWidget(m_scanDetail);

    hsplit->addWidget(treeGroup);
    hsplit->addWidget(detailGroup);
    hsplit->setSizes({500, 580});
    vl->addWidget(hsplit);
}

// ──────────────────────────────────────────────
//  HTTP Log Tab
// ──────────────────────────────────────────────
void MainWindow::setupLogTab()
{
    m_logTab = new QWidget;
    auto *vl = new QVBoxLayout(m_logTab);
    vl->setContentsMargins(8, 8, 8, 8);
    vl->setSpacing(6);

    auto *ctrlBar = new QWidget;
    ctrlBar->setFixedHeight(40);
    ctrlBar->setStyleSheet("background:#dce8f8; border-radius:5px;");
    auto *hl = new QHBoxLayout(ctrlBar);
    hl->setContentsMargins(10, 4, 10, 4);

    auto *searchLbl = new QLabel("Search:");
    auto *searchEdit = new QLineEdit;
    searchEdit->setPlaceholderText("filter by host / path / status…");
    searchEdit->setFixedWidth(260);

    m_clearLogBtn = new QPushButton("Clear");
    m_clearLogBtn->setObjectName("dangerBtn");
    m_clearLogBtn->setFixedWidth(80);
    connect(m_clearLogBtn, &QPushButton::clicked, this, &MainWindow::onClearLog);

    auto *pauseBtn = new QPushButton("⏸ Pause");
    pauseBtn->setFixedWidth(80);
    pauseBtn->setCheckable(true);

    hl->addWidget(searchLbl);
    hl->addWidget(searchEdit);
    hl->addSpacing(12);
    hl->addWidget(pauseBtn);
    hl->addWidget(m_clearLogBtn);
    hl->addStretch();
    vl->addWidget(ctrlBar);

    m_logTable = new QTableWidget(0, 7);
    m_logTable->setHorizontalHeaderLabels({"Time", "Method", "Host", "Path", "Status", "Length", "Type"});
    m_logTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    m_logTable->horizontalHeader()->setStretchLastSection(false);
    m_logTable->setColumnWidth(0, 82);
    m_logTable->setColumnWidth(1, 60);
    m_logTable->setColumnWidth(2, 210);
    m_logTable->setColumnWidth(3, 290);
    m_logTable->setColumnWidth(4, 55);
    m_logTable->setColumnWidth(5, 70);
    m_logTable->setColumnWidth(6, 120);
    m_logTable->setAlternatingRowColors(true);
    m_logTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_logTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_logTable->verticalHeader()->setVisible(false);
    m_logTable->setSortingEnabled(true);
    vl->addWidget(m_logTable);
}

// ──────────────────────────────────────────────
//  Slots
// ──────────────────────────────────────────────
void MainWindow::onInterceptToggled(bool checked)
{
    m_interceptOn = checked;
    m_forwardBtn->setEnabled(checked);
    m_dropBtn->setEnabled(checked);
    m_sendRepeaterBtn->setEnabled(checked);
    m_interceptCheck->setText(checked ? "Intercept is ON" : "Intercept is OFF");

    if (checked) {
        m_interceptStatus->setText("INTERCEPT: ON");
        m_interceptStatus->setStyleSheet("color : #a5d6a7; font-weight: bold; font-size: 9pt; "
                                         "background:#0d47a1; padding: 3px 10px; border-radius: 3px;");
        statusBar()->showMessage("  Intercept enabled – traffic paused");
    } else {
        m_interceptStatus->setText("INTERCEPT: OFF");
        m_interceptStatus->setStyleSheet("color: #ef9a9a; font-weight: bold; font-size: 9pt; "
                                         "background:#0d47a1; padding: 3px 10px; border-radius: 3px;");
        statusBar()->showMessage("  Intercept disabled – passing traffic");
    }
}

void MainWindow::onForwardClicked()
{
    statusBar()->showMessage("  Request forwarded");
    m_rawResponse->setPlainText(SAMPLE_RESPONSE);
}

void MainWindow::onDropClicked()
{
    m_rawRequest->clear();
    m_rawResponse->clear();
    statusBar()->showMessage("  Request dropped");
}

void MainWindow::onSendToRepeater()
{
    m_repeaterRequest->setPlainText(m_rawRequest->toPlainText());
    m_targetHost->setText("target.example.com:443");
    m_mainTabs->setCurrentIndex(1);
    statusBar()->showMessage("  Request sent to Repeater");
}

void MainWindow::onRepeatRequest()
{
    m_sendBtn->setEnabled(false);
    m_sendBtn->setText("Sending…");

    QTimer::singleShot(600, this, [this]() {
        m_repeaterResponse->setPlainText(SAMPLE_RESPONSE);
        m_sendBtn->setEnabled(true);
        m_sendBtn->setText("▶  Send");
        statusBar()->showMessage("  Repeater: response received  200 OK  142 bytes");
    });
}

void MainWindow::onClearLog()
{
    m_logTable->setRowCount(0);
    statusBar()->showMessage("  Log cleared");
}

// ──────────────────────────────────────────────
//  Traffic simulation
// ──────────────────────────────────────────────

void MainWindow::addLogRow(const QString &method, const QString &host,
                            const QString &path, int status, int length,
                            const QString &type)
{
    int row = m_logTable->rowCount();
    m_logTable->insertRow(row);

    QString ts = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    m_logTable->setItem(row, 0, new QTableWidgetItem(ts));
    m_logTable->setItem(row, 1, new QTableWidgetItem(method));
    m_logTable->setItem(row, 2, new QTableWidgetItem(host));
    m_logTable->setItem(row, 3, new QTableWidgetItem(path));

    auto *statusItem = new QTableWidgetItem(QString::number(status));
    if (status >= 500)      statusItem->setForeground(QColor("#c62828"));
    else if (status >= 400) statusItem->setForeground(QColor("#e65100"));
    else if (status >= 300) statusItem->setForeground(QColor("#1565c0"));
    else                    statusItem->setForeground(QColor("#2e7d32"));
    m_logTable->setItem(row, 4, statusItem);

    m_logTable->setItem(row, 5, new QTableWidgetItem(QString::number(length)));
    m_logTable->setItem(row, 6, new QTableWidgetItem(type));

    m_logTable->scrollToBottom();
    statusBar()->showMessage("  HTTP Log – " + QString::number(row + 1) + " requests captured");
}

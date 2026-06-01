// SPDX-License-Identifier: MIT
// Encoding: UTF-8
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
#include <QDockWidget>
#include <QInputDialog>
#include <QMessageBox>
#include <QStandardPaths>
#include <QVector>

#include "trafficrecord.h"
#include "proxyserver.h"
#include "repeaterclient.h"
#include "storagemanager.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow() = default;

private slots:
    // Proxy tab
    void onInterceptToggled(bool checked);
    void onForwardClicked();
    void onDropClicked();
    void onSendToRepeater();
    void onHistoryRowClicked(int row, int col);

    // Log tab
    void onClearLog();
    void onLogRowClicked(int row, int col);

    // Repeater tab
    void onRepeatRequest();
    void onRepeaterResponse(QByteArray raw);
    void onRepeaterError(QString msg);

    // Collections dock
    void onNewGroup();
    void onSaveToGroup();
    void onCollectionItemClicked(QTreeWidgetItem *item, int col);
    void onCollectionDeleteAction();

    // Proxy server callbacks
    void onRequestIntercepted(int connId, QByteArray rawRequest);
    void onNextIntercepted(int connId, QByteArray rawRequest);
    void onRequestFinished(TrafficRecord record);

    // General
    void onClearHistory();

private:
    void setupUI();
    void applyStyleSheet();
    void setupProxyTab();
    void setupRepeaterTab();
    void setupLogTab();
    void setupTopBar();
    void setupCollectionsDock();
    void refreshCollectionsTree();

    void addHistoryRow(const TrafficRecord &rec);
    void addLogRow(const TrafficRecord &rec);

    // ── Tabs ──────────────────────────────────────────────────────────────────
    QTabWidget      *m_mainTabs     = nullptr;

    // Proxy tab
    QWidget         *m_proxyTab     = nullptr;
    QCheckBox       *m_interceptCheck = nullptr;
    QPushButton     *m_forwardBtn   = nullptr;
    QPushButton     *m_dropBtn      = nullptr;
    QPushButton     *m_sendRepeaterBtn = nullptr;
    QPlainTextEdit  *m_rawRequest   = nullptr;
    QPlainTextEdit  *m_rawResponse  = nullptr;
    QTableWidget    *m_historyTable = nullptr;

    // Repeater tab
    QWidget         *m_repeaterTab  = nullptr;
    QLineEdit       *m_targetHost   = nullptr;
    QCheckBox       *m_sslCheck     = nullptr;
    QPlainTextEdit  *m_repeaterRequest  = nullptr;
    QPlainTextEdit  *m_repeaterResponse = nullptr;
    QPushButton     *m_sendBtn      = nullptr;

    // Log tab
    QWidget         *m_logTab       = nullptr;
    QTableWidget    *m_logTable     = nullptr;
    QPushButton     *m_clearLogBtn  = nullptr;
    QPlainTextEdit  *m_logInspectReq  = nullptr;
    QPlainTextEdit  *m_logInspectResp = nullptr;

    // Top bar labels
    QLabel          *m_statusLabel      = nullptr;
    QLabel          *m_interceptStatus  = nullptr;
    QLabel          *m_connCountLabel   = nullptr;

    // Collections dock
    QDockWidget     *m_collectionsDock  = nullptr;
    QTreeWidget     *m_collectionsTree  = nullptr;

    // ── Backend ───────────────────────────────────────────────────────────────
    ProxyServer     *m_server           = nullptr;
    RepeaterClient  *m_repeaterClient   = nullptr;
    StorageManager  *m_storage          = nullptr;

    // In-memory traffic store (session)
    QVector<TrafficRecord> m_traffic;

    // Intercept state
    int              m_pendingConnId     = -1;  // ID currently shown in the panel
    int              m_awaitingRespId    = -1;  // ID forwarded, waiting for response
    int              m_queuedCount       = 0;   // items sitting in server queue
    QLabel          *m_queueLabel        = nullptr; // shows queue depth in ctrl bar

    int              m_selectedHistoryId = -1;
    bool             m_interceptOn       = false;
    int              m_activeConns       = 0;
    bool             m_logPaused         = false;
};

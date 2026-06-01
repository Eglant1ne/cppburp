#pragma once

#include <QObject>
#include <QTcpSocket>
#include <QSslSocket>
#include <QByteArray>
#include <QTimer>
#include "httpparser.h"
#include "trafficrecord.h"

class ProxyConnection : public QObject
{
    Q_OBJECT
public:
    explicit ProxyConnection(qintptr socketDescriptor, bool interceptEnabled, QObject *parent = nullptr);
    ~ProxyConnection();

    // Called by ProxyServer when intercept state changes
    void setIntercept(bool on) { m_interceptOn = on; }

    // If this connection is currently held waiting for user action (intercept),
    // forward it immediately using the original unmodified request bytes.
    void releaseIfHeld();

    int id() const { return m_id; }

public slots:
    // UI actions
    void forwardRequest(const QByteArray &modifiedRequest);
    void dropRequest();
    void dropConnection();
signals:
    // Emitted when a complete request is intercepted (needs user action)
    void intercepted(int id, QByteArray rawRequest);

    // Emitted when a full request+response cycle completes
    void finished(TrafficRecord record);

    // Tell server we are done
    void done(int id);

private slots:
    void onClientData();
    void onTargetConnected();
    void onTargetData();
    void onClientDisconnected();
    void onTargetDisconnected();
    void onSslEstablished();

private:
    void connectToTarget(const QString &host, int port, bool tls);
    void flushToTarget(const QByteArray &data);
    void startTunnel();          // transparent TCP/TLS tunnel after CONNECT handshake

    static int s_idCounter;

    int         m_id;
    qintptr     m_descriptor;
    bool        m_interceptOn;

    QTcpSocket *m_clientSocket  = nullptr;
    QTcpSocket *m_targetSocket  = nullptr;  // plain or SSL

    QByteArray  m_clientBuffer;
    QByteArray  m_responseBuffer;

    HttpRequest m_parsedRequest;
    bool        m_requestParsed = false;
    bool        m_tunnelMode    = false;    // transparent after CONNECT

    QDateTime   m_startTime;
};

#pragma once

#include <QTcpServer>
#include <QMap>
#include <QQueue>
#include "proxyconnection.h"
#include "trafficrecord.h"

class ProxyServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit ProxyServer(QObject *parent = nullptr);

    bool startListening(const QHostAddress &addr, quint16 port);
    void setIntercept(bool on);
    bool isIntercepting() const { return m_intercept; }
    quint16 port() const { return m_port; }
    int queueDepth() const { return m_interceptQueue.size(); }

public slots:
    void forwardPendingRequest(const QByteArray &rawRequest);
    void dropPendingRequest();
    void clearQueue();   // drop active + all queued requests immediately

signals:
    void requestIntercepted(int connId, QByteArray rawRequest);
    void nextIntercepted(int connId, QByteArray rawRequest);
    void requestFinished(TrafficRecord record);
    void serverError(const QString &message);
    void queueChanged(int totalPending);

protected:
    void incomingConnection(qintptr handle) override;

private:
    void promoteNext();

    bool    m_intercept = false;
    quint16 m_port      = 8080;
    int     m_activeId  = -1;

    struct PendingItem { int connId; QByteArray raw; };
    QQueue<PendingItem>         m_interceptQueue;
    QMap<int, ProxyConnection*> m_connections;
};

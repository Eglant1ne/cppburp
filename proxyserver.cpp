#include "proxyserver.h"
#include <QHostAddress>

ProxyServer::ProxyServer(QObject *parent)
    : QTcpServer(parent)
{}

bool ProxyServer::startListening(const QHostAddress &addr, quint16 port)
{
    m_port = port;
    if (!listen(addr, port)) {
        emit serverError(tr("Cannot bind to %1:%2 — %3")
                             .arg(addr.toString())
                             .arg(port)
                             .arg(errorString()));
        return false;
    }
    return true;
}

void ProxyServer::setIntercept(bool on)
{
    m_intercept = on;
    for (auto *conn : m_connections)
        conn->setIntercept(on);
}

void ProxyServer::incomingConnection(qintptr handle)
{
    auto *conn = new ProxyConnection(handle, m_intercept, this);

    connect(conn, &ProxyConnection::intercepted, this,
            [this](int id, QByteArray raw) {
                if (m_activeId < 0) {
                    // No request currently shown — show this one immediately
                    m_activeId = id;
                    emit requestIntercepted(id, raw);
                } else {
                    // Another request is being reviewed — queue it
                    m_interceptQueue.enqueue({id, raw});
                    // nextIntercepted with a queue-depth hint so UI can show counter
                    emit nextIntercepted(-2, raw);  // -2 = "queued, not yet shown"
                }
            });

    connect(conn, &ProxyConnection::finished, this,
            [this](TrafficRecord rec) {
                emit requestFinished(rec);
            });

    connect(conn, &ProxyConnection::done, this,
            [this](int id) {
                m_connections.remove(id);
                if (m_activeId == id) {
                    m_activeId = -1;
                    promoteNext();
                }
                // Also scrub from queue if the connection died on its own
                QQueue<PendingItem> cleaned;
                for (auto &item : m_interceptQueue)
                    if (item.connId != id) cleaned.enqueue(item);
                m_interceptQueue = cleaned;
            });

    m_connections.insert(conn->id(), conn);
}

// ── Called after forward/drop to show the next queued item ──────────────────
void ProxyServer::promoteNext()
{
    if (m_interceptQueue.isEmpty()) {
        emit nextIntercepted(-1, {});   // -1 = queue empty
        return;
    }
    auto item = m_interceptQueue.dequeue();
    m_activeId = item.connId;
    emit requestIntercepted(item.connId, item.raw);
}

void ProxyServer::forwardPendingRequest(const QByteArray &rawRequest)
{
    if (m_activeId < 0) return;
    auto *conn = m_connections.value(m_activeId, nullptr);

    // Clear active slot BEFORE calling promoteNext so it can assign the next id
    m_activeId = -1;

    if (conn) conn->forwardRequest(rawRequest);

    promoteNext();

    emit queueChanged(m_interceptQueue.size() + (m_activeId >= 0 ? 1 : 0));
}

void ProxyServer::dropPendingRequest()
{
    if (m_activeId < 0) return;
    auto *conn = m_connections.value(m_activeId, nullptr);

    m_activeId = -1;

    if (conn) conn->dropRequest();

    promoteNext();

    emit queueChanged(m_interceptQueue.size() + (m_activeId >= 0 ? 1 : 0));
}

void ProxyServer::clearQueue()
{
    // Drop the currently shown request
    if (m_activeId >= 0) {
        auto *conn = m_connections.value(m_activeId, nullptr);
        if (conn) conn->dropRequest();
        m_activeId = -1;
    }
    // Drop everything still in the queue
    while (!m_interceptQueue.isEmpty()) {
        auto item = m_interceptQueue.dequeue();
        auto *conn = m_connections.value(item.connId, nullptr);
        if (conn) conn->dropRequest();
    }
    emit queueChanged(0);
    emit nextIntercepted(-1, {});
}

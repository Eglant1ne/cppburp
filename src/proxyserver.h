// SPDX-License-Identifier: MIT
// Encoding: UTF-8
#pragma once

#include <QTcpServer>
#include <QMap>
#include <QQueue>
#include "proxyconnection.h"
#include "trafficrecord.h"

/**
 * @brief TCP server that accepts browser connections and manages the intercept queue.
 *
 * ProxyServer listens on a configurable address/port (default 127.0.0.1:8080)
 * and creates a ProxyConnection for every accepted socket. When interception is
 * enabled it serialises incoming requests through an internal queue so that the UI
 * can inspect and modify them one at a time.
 */
class ProxyServer : public QTcpServer {
    Q_OBJECT
public:
    /**
     * @brief Construct the server; does not start listening yet.
     * @param parent Qt parent object.
     */
    explicit ProxyServer(QObject* parent = nullptr);

    /**
     * @brief Bind to @p addr : @p port and start accepting connections.
     * @param addr  Local address to bind (use QHostAddress::LocalHost for loopback).
     * @param port  TCP port number (1–65535).
     * @return true on success, false if the port is already in use.
     * @throws std::runtime_error if the underlying socket cannot be created.
     */
    bool startListening(const QHostAddress& addr, quint16 port);

    /**
     * @brief Enable or disable request interception.
     *
     * When turned off all queued and currently-held connections are released
     * immediately so that pending browser requests are not left hanging.
     *
     * @param on true to enable interception, false to pass requests through.
     */
    void setIntercept(bool on);

    /** @return true if interception is currently enabled. */
    bool isIntercepting() const { return m_intercept; }

    /** @return The port number this server is listening on. */
    quint16 port() const { return m_port; }

    /** @return Number of requests currently waiting in the intercept queue. */
    int queueDepth() const { return m_interceptQueue.size(); }

public slots:
    /**
     * @brief Forward the currently displayed intercepted request to the origin server.
     * @param rawRequest  Possibly-edited raw HTTP bytes to send.
     */
    void forwardPendingRequest(const QByteArray& rawRequest);

    /**
     * @brief Drop (block) the currently displayed intercepted request.
     *
     * Sends HTTP 403 to the browser and promotes the next queued item.
     */
    void dropPendingRequest();

    /**
     * @brief Drop the active request and all queued requests immediately.
     */
    void clearQueue();

signals:
    /**
     * @brief Emitted when a new request should be shown in the intercept panel.
     * @param connId     Internal connection identifier.
     * @param rawRequest Raw HTTP bytes of the captured request.
     */
    void requestIntercepted(int connId, QByteArray rawRequest);

    /**
     * @brief Emitted when the queue state changes (item added or removed).
     *
     * Special values for @p connId: -1 = queue empty, -2 = item queued but
     * another request is already shown.
     *
     * @param connId     Connection ID of the next item, or a negative sentinel.
     * @param rawRequest Raw bytes of the next item (empty when connId < 0).
     */
    void nextIntercepted(int connId, QByteArray rawRequest);

    /**
     * @brief Emitted when a full request/response cycle completes.
     * @param record  Populated TrafficRecord for the finished exchange.
     */
    void requestFinished(TrafficRecord record);

    /**
     * @brief Emitted when the server encounters a fatal error (e.g. bind failure).
     * @param message Human-readable error description.
     */
    void serverError(const QString& message);

    /**
     * @brief Emitted whenever the total number of pending requests changes.
     * @param totalPending  Current queue depth (0 when idle).
     */
    void queueChanged(int totalPending);

protected:
    /** @brief Overridden to create a ProxyConnection for each accepted socket. */
    void incomingConnection(qintptr handle) override;

private:
    /** @brief Promote the next queued item to the active intercept slot. */
    void promoteNext();

    bool m_intercept = false;  ///< Current interception state.
    quint16 m_port = 8080;     ///< Listening port.
    int m_activeId = -1;       ///< ID of the connection currently shown in the UI.

    /** @brief One entry in the pending-intercept queue. */
    struct PendingItem {
        int connId;        ///< Connection ID.
        QByteArray raw;    ///< Captured raw request bytes.
    };

    QQueue<PendingItem> m_interceptQueue;          ///< Requests waiting to be shown.
    QMap<int, ProxyConnection*> m_connections;     ///< All live connections keyed by ID.
};

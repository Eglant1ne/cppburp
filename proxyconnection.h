// SPDX-License-Identifier: MIT
// Encoding: UTF-8
#pragma once

#include <QObject>
#include <QTcpSocket>
#include <QSslSocket>
#include <QByteArray>
#include <QTimer>
#include "httpparser.h"
#include "trafficrecord.h"

/**
 * @brief Handles one client↔origin-server connection through the proxy.
 *
 * Created by ProxyServer for every accepted TCP socket. Reads the client
 * request, optionally holds it for the user (intercept mode), then relays
 * it to the origin server and pipes the response back. Supports both plain
 * HTTP and TLS tunnels established via the CONNECT method.
 *
 * Lifetime: the object deletes itself (deleteLater) when both sides close.
 */
class ProxyConnection : public QObject {
    Q_OBJECT
public:
    /**
     * @brief Construct and attach to an already-accepted socket descriptor.
     * @param socketDescriptor  Native socket handle from QTcpServer::incomingConnection.
     * @param interceptEnabled  Initial interception state.
     * @param parent            Qt parent (usually the ProxyServer).
     */
    explicit ProxyConnection(qintptr socketDescriptor, bool interceptEnabled,
                             QObject* parent = nullptr);

    /** @brief Destructor; sockets are cleaned up by Qt's parent/child mechanism. */
    ~ProxyConnection();

    /**
     * @brief Update interception state for this connection.
     * @param on true to intercept subsequent requests on this connection.
     */
    void setIntercept(bool on) { m_interceptOn = on; }

    /**
     * @brief Forward the original (unmodified) request if the connection is on hold.
     *
     * A connection is "held" when intercept fired but the user has not yet
     * pressed Forward or Drop. Calling this method resumes normal proxy flow.
     * Has no effect if the connection is not currently held.
     */
    void releaseIfHeld();

    /** @return The unique session ID assigned to this connection. */
    int id() const { return m_id; }

public slots:
    /**
     * @brief Forward a (potentially modified) request to the origin server.
     *
     * Normalises line endings, strips absolute URLs from the request line,
     * forces Connection: close, and recalculates Content-Length before sending.
     *
     * @param modifiedRequest  Raw HTTP bytes as edited by the user.
     */
    void forwardRequest(const QByteArray& modifiedRequest);

    /**
     * @brief Drop this request and send HTTP 403 to the browser.
     */
    void dropRequest();

    /**
     * @brief Close both the client and target sockets immediately.
     */
    void dropConnection();

signals:
    /**
     * @brief Emitted when a complete request header is available and intercept is on.
     * @param id         This connection's ID.
     * @param rawRequest Full raw request bytes (headers + body so far).
     */
    void intercepted(int id, QByteArray rawRequest);

    /**
     * @brief Emitted when a complete request/response cycle finishes.
     * @param record  Populated record ready for storage and UI display.
     */
    void finished(TrafficRecord record);

    /**
     * @brief Emitted when this connection is fully closed and can be removed.
     * @param id  This connection's ID.
     */
    void done(int id);

private slots:
    void onClientData();
    void onTargetConnected();
    void onTargetData();
    void onClientDisconnected();
    void onTargetDisconnected();
    void onSslEstablished();

private:
    /**
     * @brief Open a connection to the origin server.
     * @param host  Hostname or IP address.
     * @param port  Port number.
     * @param tls   If true, perform a TLS handshake (QSslSocket).
     */
    void connectToTarget(const QString& host, int port, bool tls);

    /**
     * @brief Write @p data to the target socket if it is open.
     * @param data  Bytes to send.
     */
    void flushToTarget(const QByteArray& data);

    /** @brief Switch the connection into transparent tunnel mode. */
    void startTunnel();

    static int s_idCounter;  ///< Monotonically increasing ID counter.

    int m_id;            ///< Unique ID for this connection.
    qintptr m_descriptor;  ///< Original socket descriptor.
    bool m_interceptOn;    ///< Whether interception is active.

    QTcpSocket* m_clientSocket = nullptr;  ///< Socket to the browser.
    QTcpSocket* m_targetSocket = nullptr;  ///< Socket to the origin (plain or SSL).

    QByteArray m_clientBuffer;    ///< Buffered bytes from the client.
    QByteArray m_responseBuffer;  ///< Accumulated response bytes for TrafficRecord.

    HttpRequest m_parsedRequest;     ///< Parsed request (valid when m_requestParsed).
    bool m_requestParsed = false;    ///< True once HttpParser::parse succeeded.
    bool m_tunnelMode = false;       ///< True after CONNECT tunnel is established.

    QDateTime m_startTime;  ///< When the connection was accepted.
};

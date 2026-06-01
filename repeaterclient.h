// SPDX-License-Identifier: MIT
// Encoding: UTF-8
#pragma once

#include <QObject>
#include <QTcpSocket>
#include <QSslSocket>
#include <QByteArray>
#include <QTimer>
#include "httpparser.h"

/**
 * @brief Standalone HTTP/HTTPS client for the Repeater tab.
 *
 * RepeaterClient connects directly to an origin server (bypassing the proxy),
 * sends a single HTTP request, and emits the raw response. It handles both
 * plain TCP and TLS connections, normalises line endings and headers before
 * sending, and enforces a 20-second hard timeout.
 *
 * Only one request can be in flight at a time; calling send() while a
 * previous request is pending cancels the previous one silently.
 */
class RepeaterClient : public QObject {
    Q_OBJECT
public:
    /**
     * @brief Construct the client and initialise the timeout timer.
     * @param parent Qt parent object.
     */
    explicit RepeaterClient(QObject* parent = nullptr);

    /**
     * @brief Destructor; aborts any in-flight connection.
     */
    ~RepeaterClient();

    /**
     * @brief Send a raw HTTP request to the specified host.
     *
     * Cancels any previous in-flight request before starting.
     * The request bytes are normalised (line endings, Connection: close,
     * Content-Length) before transmission.
     *
     * @param rawRequest  Raw HTTP/1.x request bytes (may use \\n or \\r\\n line endings).
     * @param host        Target hostname or IP address.
     * @param port        Target TCP port.
     * @param useSsl      If true, use a TLS connection (QSslSocket).
     */
    void send(const QByteArray& rawRequest, const QString& host, int port, bool useSsl);

signals:
    /**
     * @brief Emitted when a complete response has been received.
     * @param rawResponse  Full raw response bytes including headers and body.
     */
    void responseReceived(QByteArray rawResponse);

    /**
     * @brief Emitted when a network or timeout error occurs.
     * @param errorMessage  Human-readable error description.
     */
    void errorOccurred(QString errorMessage);

private slots:
    void onConnected();
    void onSslConnected();
    void onReadyRead();
    void onDisconnected();
    void onSocketError(QAbstractSocket::SocketError err);
    void onOverallTimeout();

private:
    /**
     * @brief Normalise and write the request to the open socket.
     *
     * Fixes line endings, strips absolute URL from the request line,
     * forces Connection: close, and recalculates Content-Length.
     */
    void doSend();

    /**
     * @brief Emit responseReceived with the buffered data and clean up.
     *
     * Safe to call multiple times; subsequent calls are no-ops.
     */
    void finish();

    /**
     * @brief Stop the timer and abort the socket without emitting any signal.
     */
    void hardReset();

    QByteArray m_request;   ///< Request bytes passed to send().
    QByteArray m_response;  ///< Accumulated response bytes.
    QString m_host;         ///< Target host.
    int m_port = 80;        ///< Target port.
    bool m_useSsl = false;  ///< Whether TLS is in use.

    QTcpSocket* m_socket = nullptr;       ///< Active socket (plain or SSL).
    QTimer* m_overallTimer = nullptr;     ///< 20-second hard limit timer.
    bool m_finished = false;              ///< Guards against double-emit.
};

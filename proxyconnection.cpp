// SPDX-License-Identifier: MIT
// Encoding: UTF-8
#include "proxyconnection.h"
#include <QSslSocket>
#include <QHostAddress>
#include <stdexcept>

int ProxyConnection::s_idCounter = 0;

ProxyConnection::ProxyConnection(qintptr descriptor, bool interceptEnabled, QObject *parent)
    : QObject(parent)
    , m_id(++s_idCounter)
    , m_descriptor(descriptor)
    , m_interceptOn(interceptEnabled)
    , m_startTime(QDateTime::currentDateTime())
{
    // ── Client socket ──
    m_clientSocket = new QTcpSocket(this);
    if (!m_clientSocket->setSocketDescriptor(m_descriptor)) {
        deleteLater();
        return;
    }

    connect(m_clientSocket, &QTcpSocket::readyRead,
            this, &ProxyConnection::onClientData);
    connect(m_clientSocket, &QTcpSocket::disconnected,
            this, &ProxyConnection::onClientDisconnected);
}

ProxyConnection::~ProxyConnection() = default;

// ─────────────────────────────────────────────────────────────────────────────
//  Client → Proxy  (incoming data from browser)
// ─────────────────────────────────────────────────────────────────────────────
void ProxyConnection::onClientData()
{
    m_clientBuffer += m_clientSocket->readAll();

    // In tunnel mode just forward bytes to target
    if (m_tunnelMode) {
        if (m_targetSocket && m_targetSocket->isOpen())
            m_targetSocket->write(m_clientBuffer);
        m_clientBuffer.clear();
        return;
    }

    // Try to parse HTTP header
    if (!m_requestParsed) {
        try {
            if (!HttpParser::parse(m_clientBuffer, m_parsedRequest))
                return;  // need more data
        } catch (const std::invalid_argument& ex) {
            qWarning("ProxyConnection: bad request — %s", ex.what());
            dropConnection();
            return;
        }
        m_requestParsed = true;
    }

    // CONNECT → set up TLS tunnel
    if (m_parsedRequest.isConnect) {
        if (m_clientSocket->isOpen())
            m_clientSocket->write("HTTP/1.1 200 Connection established\r\n\r\n");
        m_clientBuffer.clear();
        connectToTarget(m_parsedRequest.host, m_parsedRequest.port, /*tls=*/true);
        return;
    }

    // Normal HTTP request
    if (m_interceptOn) {
        emit intercepted(m_id, m_clientBuffer);
        return;
    }

    connectToTarget(m_parsedRequest.host, m_parsedRequest.port, m_parsedRequest.isTls);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Connect to origin server
// ─────────────────────────────────────────────────────────────────────────────
void ProxyConnection::connectToTarget(const QString &host, int port, bool tls)
{
    if (tls) {
        auto *ssl = new QSslSocket(this);
        ssl->setPeerVerifyMode(QSslSocket::VerifyNone);
        m_targetSocket = ssl;
        connect(ssl, &QSslSocket::encrypted,
                this, &ProxyConnection::onSslEstablished);
    } else {
        m_targetSocket = new QTcpSocket(this);
    }

    connect(m_targetSocket, &QTcpSocket::connected,
            this, &ProxyConnection::onTargetConnected);
    connect(m_targetSocket, &QTcpSocket::readyRead,
            this, &ProxyConnection::onTargetData);
    connect(m_targetSocket, &QTcpSocket::disconnected,
            this, &ProxyConnection::onTargetDisconnected);

    if (tls) {
        auto *ssl = qobject_cast<QSslSocket*>(m_targetSocket);
        ssl->connectToHostEncrypted(host, static_cast<quint16>(port));
    } else {
        m_targetSocket->connectToHost(host, static_cast<quint16>(port));
    }
}

void ProxyConnection::onTargetConnected()
{
    if (m_parsedRequest.isConnect) {
        m_tunnelMode = true;
        return;
    }
    flushToTarget(m_clientBuffer);
    m_clientBuffer.clear();
}

void ProxyConnection::onSslEstablished()
{
    if (m_parsedRequest.isConnect) {
        m_tunnelMode = true;
        return;
    }
    flushToTarget(m_clientBuffer);
    m_clientBuffer.clear();
}

void ProxyConnection::flushToTarget(const QByteArray &data)
{
    if (m_targetSocket && m_targetSocket->isOpen())
        m_targetSocket->write(data);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Target → Proxy → Client
// ─────────────────────────────────────────────────────────────────────────────
void ProxyConnection::onTargetData()
{
    QByteArray chunk = m_targetSocket->readAll();

    if (m_clientSocket && m_clientSocket->isOpen())
        m_clientSocket->write(chunk);

    if (!m_tunnelMode)
        m_responseBuffer += chunk;
}

void ProxyConnection::onTargetDisconnected()
{
    if (!m_tunnelMode && m_requestParsed) {
        TrafficRecord rec;
        rec.id     = m_id;
        rec.time   = m_startTime;
        rec.method = m_parsedRequest.method;
        rec.host   = m_parsedRequest.host;
        rec.size   = m_responseBuffer.size();

        QString fullUrl = m_parsedRequest.url;
        if (fullUrl.startsWith("http://") || fullUrl.startsWith("https://")) {
            int slashPos = fullUrl.indexOf('/', 8);
            rec.path = slashPos >= 0 ? fullUrl.mid(slashPos) : "/";
        } else {
            rec.path = fullUrl;
        }

        rec.rawRequest  = QString::fromLatin1(m_parsedRequest.raw);
        rec.rawResponse = QString::fromLatin1(m_responseBuffer);

        int statusStart = m_responseBuffer.indexOf("HTTP/");
        if (statusStart >= 0) {
            int sp1 = m_responseBuffer.indexOf(' ', statusStart);
            if (sp1 >= 0) {
                int sp2 = m_responseBuffer.indexOf(' ', sp1 + 1);
                rec.status = m_responseBuffer.mid(sp1 + 1, sp2 - sp1 - 1).toInt();
            }
        }

        emit finished(rec);
    }

    if (m_clientSocket && m_clientSocket->isOpen())
        m_clientSocket->disconnectFromHost();

    emit done(m_id);
    deleteLater();
}

void ProxyConnection::onClientDisconnected()
{
    if (m_targetSocket && m_targetSocket->isOpen())
        m_targetSocket->disconnectFromHost();
    emit done(m_id);
    deleteLater();
}

// ─────────────────────────────────────────────────────────────────────────────
//  Intercept actions (called from UI)
// ─────────────────────────────────────────────────────────────────────────────

// Normalise raw bytes coming from QPlainTextEdit before sending to origin server.
// Fixes:
//   1. \n → \r\n  (QPlainTextEdit strips \r)
//   2. Absolute URL → relative path  (browsers send "GET http://host/path" to
//      proxies, but origin servers only accept "GET /path")
//   3. Connection: close  (prevents keep-alive stalls)
//   4. Content-Length recalculated after body edits
static QByteArray normalizeRequest(const QByteArray &raw)
{
    QByteArray req = raw;

    // 1. Normalize line endings
    req.replace("\r\n", "\n");
    req.replace("\n", "\r\n");

    // Ensure header/body separator
    if (!req.contains("\r\n\r\n")) {
        if (!req.endsWith("\r\n")) req.append("\r\n");
        req.append("\r\n");
    }

    int blankIdx = req.indexOf("\r\n\r\n");
    QByteArray hdrs = req.left(blankIdx);
    QByteArray body = req.mid(blankIdx + 4);

    // 2. Strip absolute URL in the request line → relative path
    //    "GET http://example.com/path?q=1 HTTP/1.1"
    //    becomes "GET /path?q=1 HTTP/1.1"
    {
        int sp1 = hdrs.indexOf(' ');          // after method
        int sp2 = hdrs.indexOf(' ', sp1 + 1); // after URL
        if (sp1 > 0 && sp2 > sp1) {
            QByteArray url = hdrs.mid(sp1 + 1, sp2 - sp1 - 1);
            if (url.startsWith("http://") || url.startsWith("https://")) {
                int schemeEnd = url.indexOf("://") + 3;
                int pathStart = url.indexOf('/', schemeEnd);
                QByteArray rel = (pathStart >= 0) ? url.mid(pathStart) : QByteArray("/");
                hdrs.replace(sp1 + 1, url.size(), rel);
            }
        }
    }

    QByteArray lowerHdrs = hdrs.toLower();

    // 3. Force Connection: close
    int connIdx = lowerHdrs.indexOf("\nconnection:");
    if (connIdx >= 0) {
        connIdx++;
        int lineEnd = hdrs.indexOf("\r\n", connIdx);
        if (lineEnd >= 0)
            hdrs.replace(connIdx, lineEnd - connIdx, "Connection: close");
    } else {
        hdrs.append("\r\nConnection: close");
    }

    // 4. Recalculate Content-Length
    lowerHdrs = hdrs.toLower();
    int clIdx = lowerHdrs.indexOf("\ncontent-length:");
    if (clIdx >= 0) {
        clIdx++;
        int lineEnd = hdrs.indexOf("\r\n", clIdx);
        if (lineEnd >= 0)
            hdrs.replace(clIdx, lineEnd - clIdx,
                         "Content-Length: " + QByteArray::number(body.size()));
    }

    return hdrs + "\r\n\r\n" + body;
}

void ProxyConnection::forwardRequest(const QByteArray &modifiedRequest)
{
    QByteArray req = normalizeRequest(modifiedRequest);

    m_clientBuffer = req;

    // Re-parse to get up-to-date host/port from the edited request.
    HttpRequest parsed;
    try {
        if (HttpParser::parse(req, parsed))
            m_parsedRequest = parsed;
    } catch (const std::invalid_argument& ex) {
        qWarning("ProxyConnection::forwardRequest — bad request: %s", ex.what());
        dropConnection();
        return;
    }

    // Destroy any previous target socket so we don't leak/double-connect
    if (m_targetSocket) {
        m_targetSocket->disconnect();
        m_targetSocket->deleteLater();
        m_targetSocket = nullptr;
    }

    m_responseBuffer.clear();
    connectToTarget(m_parsedRequest.host, m_parsedRequest.port, m_parsedRequest.isTls);
}

void ProxyConnection::releaseIfHeld()
{
    // A connection is "held" when intercept fired but the user has not yet
    // forwarded or dropped it.  m_clientBuffer still holds the original raw
    // request at that point and no target socket has been created yet.
    if (m_targetSocket || m_tunnelMode || !m_requestParsed)
        return;  // already connecting/connected, or nothing parsed yet

    // Forward using the bytes we already have
    connectToTarget(m_parsedRequest.host, m_parsedRequest.port, m_parsedRequest.isTls);
}


void ProxyConnection::dropConnection()
{
    if (m_clientSocket && m_clientSocket->isOpen()) {
        m_clientSocket->write(
            "HTTP/1.1 403 Forbidden\r\nContent-Length: 0\r\nConnection: close\r\n\r\n");
        m_clientSocket->disconnectFromHost();
    }
    emit done(m_id);
    deleteLater();
}
void ProxyConnection::dropRequest()
{
    if (m_clientSocket && m_clientSocket->isOpen()) {
        m_clientSocket->write(
            "HTTP/1.1 403 Forbidden\r\nContent-Length: 0\r\nConnection: close\r\n\r\n");
        m_clientSocket->disconnectFromHost();
    }
    emit done(m_id);
    deleteLater();
}
void ProxyConnection::startTunnel()
{
    m_tunnelMode = true;
}

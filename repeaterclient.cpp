// SPDX-License-Identifier: MIT
// Encoding: UTF-8
#include "repeaterclient.h"
#include <QSslSocket>


RepeaterClient::RepeaterClient(QObject *parent)
    : QObject(parent)
{
    m_overallTimer = new QTimer(this);
    m_overallTimer->setSingleShot(true);
    connect(m_overallTimer, &QTimer::timeout, this, &RepeaterClient::onOverallTimeout);
}

RepeaterClient::~RepeaterClient()
{
    hardReset();
}


void RepeaterClient::send(const QByteArray &rawRequest, const QString &host, int port, bool useSsl)
{
    hardReset();

    m_request  = rawRequest;
    m_response.clear();
    m_host     = host;
    m_port     = port;
    m_useSsl   = useSsl;
    m_finished = false;

    if (useSsl) {
        auto *ssl = new QSslSocket(this);
        ssl->setPeerVerifyMode(QSslSocket::VerifyNone);
        m_socket = ssl;
        connect(ssl, &QSslSocket::encrypted, this, &RepeaterClient::onSslConnected);
    } else {
        m_socket = new QTcpSocket(this);
        connect(m_socket, &QTcpSocket::connected, this, &RepeaterClient::onConnected);
    }

    connect(m_socket, &QTcpSocket::readyRead,    this, &RepeaterClient::onReadyRead);
    connect(m_socket, &QTcpSocket::disconnected, this, &RepeaterClient::onDisconnected);
    connect(m_socket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::errorOccurred),
            this, &RepeaterClient::onSocketError);

    m_overallTimer->start(20000);

    if (useSsl) {
        qobject_cast<QSslSocket*>(m_socket)->connectToHostEncrypted(host, static_cast<quint16>(port));
    } else {
        m_socket->connectToHost(host, static_cast<quint16>(port));
    }
}


void RepeaterClient::doSend()
{
    QByteArray req = m_request;

    req.replace("\r\n", "\n"); 
    req.replace("\n", "\r\n"); 

    if (!req.contains("\r\n\r\n")) {
        if (!req.endsWith("\r\n")) req.append("\r\n");
        req.append("\r\n");
    }

    int blankIdx = req.indexOf("\r\n\r\n");
    QByteArray hdrs = req.left(blankIdx);
    QByteArray body = req.mid(blankIdx + 4);

    QByteArray lowerHdrs = hdrs.toLower();

    int connIdx = lowerHdrs.indexOf("\nconnection:");
    if (connIdx >= 0) {
        connIdx++;
        int lineEnd = hdrs.indexOf("\r\n", connIdx);
        if (lineEnd >= 0) {
            // Connection: is a mid-block header — replace up to the \r\n
            hdrs.replace(connIdx, lineEnd - connIdx, "Connection: close");
        } else {
            // Connection: is the last header — no trailing \r\n, replace to end
            hdrs.replace(connIdx, hdrs.size() - connIdx, "Connection: close");
        }
    } else {
        hdrs.append("\r\nConnection: close");
    }

    lowerHdrs = hdrs.toLower(); 
    int clIdx = lowerHdrs.indexOf("\ncontent-length:");
    if (clIdx >= 0) {
        clIdx++;
        int lineEnd = hdrs.indexOf("\r\n", clIdx);
        if (lineEnd >= 0) {
            hdrs.replace(clIdx, lineEnd - clIdx, "Content-Length: " + QByteArray::number(body.size()));
        }
    }

    req = hdrs + "\r\n\r\n" + body;

    m_socket->write(req);
    m_socket->flush();
}

void RepeaterClient::finish()
{
    if (m_finished) return;
    m_finished = true;

    m_overallTimer->stop();

    if (m_socket) {
        m_response += m_socket->readAll();
    }

    QByteArray resp = m_response;

    if (m_socket) {
        m_socket->disconnect(this);
        m_socket->abort();
        m_socket->deleteLater();
        m_socket = nullptr;
    }

    emit responseReceived(resp);
}

void RepeaterClient::hardReset()
{
    m_overallTimer->stop();
    m_finished = true;   

    if (m_socket) {
        m_socket->disconnect(this);
        m_socket->abort();
        m_socket->deleteLater();
        m_socket = nullptr;
    }
}


void RepeaterClient::onConnected()    { doSend(); }
void RepeaterClient::onSslConnected() { doSend(); }

void RepeaterClient::onReadyRead()
{
    if (!m_socket) return;
    m_response += m_socket->readAll();

    int headerEnd = m_response.indexOf("\r\n\r\n");
    if (headerEnd < 0) return;

    QByteArray hdrs = m_response.left(headerEnd).toLower();

    if (hdrs.contains("transfer-encoding: chunked")) {
        if (m_response.endsWith("0\r\n\r\n")) {
            finish();
        }
        return;
    }

    int clPos = hdrs.indexOf("content-length:");
    if (clPos >= 0) {
        int eol = hdrs.indexOf('\n', clPos);
        if (eol > clPos) {
            QByteArray clLine = hdrs.mid(clPos, eol - clPos);
            QByteArray clValue = clLine.mid(15).trimmed(); 
            
            bool ok = false;
            int cl = clValue.toInt(&ok);
            if (ok) {
                int bodyReceived = m_response.size() - (headerEnd + 4);
                if (bodyReceived >= cl) {
                    finish();
                }
            }
        }
        return;
    }
}

void RepeaterClient::onDisconnected()
{
    finish();
}

void RepeaterClient::onSocketError(QAbstractSocket::SocketError err)
{
    if (err == QAbstractSocket::RemoteHostClosedError) {
        finish();
        return;
    }

    if (m_finished) return;
    m_finished = true;
    m_overallTimer->stop();

    QString msg = m_socket ? m_socket->errorString() : "Socket error";
    
    if (m_socket) {
        m_socket->disconnect(this);
        m_socket->abort();
        m_socket->deleteLater();
        m_socket = nullptr;
    }
    emit errorOccurred(msg);
}

void RepeaterClient::onOverallTimeout()
{
    if (m_finished) return;
    if (!m_response.isEmpty()) {
        finish();
    } else {
        m_finished = true;
        hardReset();
        emit errorOccurred("Timed out — no response in 20 s");
    }
}
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

// ─────────────────────────────────────────────────────────────────────────────
//  Public API
// ─────────────────────────────────────────────────────────────────────────────

void RepeaterClient::send(const QByteArray &rawRequest,
                           const QString &host, int port, bool useSsl)
{
    hardReset();   // kill any previous socket / timers cleanly

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

    connect(m_socket, &QTcpSocket::readyRead, this, &RepeaterClient::onReadyRead);
    connect(m_socket, &QTcpSocket::disconnected, this, &RepeaterClient::onDisconnected);
    connect(m_socket,
            QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::errorOccurred),
            this, &RepeaterClient::onSocketError);

    m_overallTimer->start(20000);

    if (useSsl)
        qobject_cast<QSslSocket*>(m_socket)->connectToHostEncrypted(host, static_cast<quint16>(port));
    else
        m_socket->connectToHost(host, static_cast<quint16>(port));
}

// ─────────────────────────────────────────────────────────────────────────────
//  Internal helpers
// ─────────────────────────────────────────────────────────────────────────────

void RepeaterClient::doSend()
{
    QByteArray req = m_request;

    // 1. Нормализация переносов строк (ОЧЕНЬ ВАЖНО для HTTP)
    // QPlainTextEdit отдает \n, а стандарт HTTP требует \r\n
    req.replace("\r\n", "\n"); // Убираем старые \r\n, чтобы не задвоить
    req.replace("\n", "\r\n"); // Делаем все переносы по стандарту

    // Убеждаемся, что пустая строка (окончание заголовков) вообще есть
    if (!req.contains("\r\n\r\n")) {
        if (!req.endsWith("\r\n")) req.append("\r\n");
        req.append("\r\n");
    }

    // 2. Разделяем запрос на заголовки и тело для безопасной модификации
    int blankIdx = req.indexOf("\r\n\r\n");
    QByteArray hdrs = req.left(blankIdx);
    QByteArray body = req.mid(blankIdx + 4);

    QByteArray lowerHdrs = hdrs.toLower();

    // 3. Форсируем Connection: close
    int connIdx = lowerHdrs.indexOf("\nconnection:");
    if (connIdx >= 0) {
        connIdx++; // сдвигаемся на начало слова 'c'
        int lineEnd = hdrs.indexOf("\r\n", connIdx);
        if (lineEnd >= 0) {
            hdrs.replace(connIdx, lineEnd - connIdx, "Connection: close");
        }
    } else {
        hdrs.append("\r\nConnection: close");
    }

    // 4. Авто-пересчет Content-Length
    // Если ты стер пару символов в теле через UI, сервер больше не будет висеть в ожидании
    lowerHdrs = hdrs.toLower(); // Обновляем после возможной вставки Connection
    int clIdx = lowerHdrs.indexOf("\ncontent-length:");
    if (clIdx >= 0) {
        clIdx++;
        int lineEnd = hdrs.indexOf("\r\n", clIdx);
        if (lineEnd >= 0) {
            hdrs.replace(clIdx, lineEnd - clIdx, "Content-Length: " + QByteArray::number(body.size()));
        }
    }

    // Собираем идеальный HTTP-запрос обратно
    req = hdrs + "\r\n\r\n" + body;

    // Отправляем и проталкиваем буфер
    m_socket->write(req);
    m_socket->flush();
}
void RepeaterClient::finish()
{
    if (m_finished) return;
    m_finished = true;

    m_overallTimer->stop();

    // Drain any remaining bytes before killing the socket
    if (m_socket)
        m_response += m_socket->readAll();

    QByteArray resp = m_response;

    // Destroy socket cleanly without triggering more signals
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
    m_finished = true;   // prevent any in-flight slot from emitting

    if (m_socket) {
        m_socket->disconnect(this);
        m_socket->abort();
        m_socket->deleteLater();
        m_socket = nullptr;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  Slots
// ─────────────────────────────────────────────────────────────────────────────

void RepeaterClient::onConnected()    { doSend(); }
void RepeaterClient::onSslConnected() { doSend(); }

void RepeaterClient::onReadyRead()
{
    if (!m_socket) return;
    m_response += m_socket->readAll();

    // Try to detect end of response via Content-Length
    int headerEnd = m_response.indexOf("\r\n\r\n");
    if (headerEnd < 0) return;

    QByteArray hdrs = m_response.left(headerEnd).toLower();

    // Chunked: wait for terminal chunk
    if (hdrs.contains("transfer-encoding: chunked")) {
        // Улучшенная проверка конца чанка
        if (m_response.endsWith("0\r\n\r\n"))
            finish();
        return;
    }

    // Content-Length
    int clPos = hdrs.indexOf("content-length:");
    if (clPos >= 0) {
        int eol = hdrs.indexOf('\n', clPos);
        bool ok = false;
        int cl = hdrs.mid(clPos + 15, eol - clPos - 15).trimmed().toInt(&ok);
        if (ok) {
            int bodyReceived = m_response.size() - (headerEnd + 4);
            if (bodyReceived >= cl)
                finish();
        }
        return;
    }

    // Unknown length — rely on disconnect
}

void RepeaterClient::onDisconnected()
{
    // Server closed the connection — whatever is buffered is the full response
    finish();
}

void RepeaterClient::onSocketError(QAbstractSocket::SocketError err)
{
    // RemoteHostClosedError fires before disconnected on many platforms
    if (err == QAbstractSocket::RemoteHostClosedError) {
        finish();
        return;
    }

    if (m_finished) return;
    m_finished = true;
    m_overallTimer->stop();

    QString msg = m_socket ? m_socket->errorString() : "socket error";
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
    if (!m_response.isEmpty())
        finish();   // return partial response
    else {
        m_finished = true;
        hardReset();
        emit errorOccurred("Timed out — no response in 20 s");
    }
}
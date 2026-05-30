#pragma once

#include <QObject>
#include <QTcpSocket>
#include <QSslSocket>
#include <QByteArray>
#include <QTimer>
#include "httpparser.h"

class RepeaterClient : public QObject
{
    Q_OBJECT
public:
    explicit RepeaterClient(QObject *parent = nullptr);
    ~RepeaterClient();

    void send(const QByteArray &rawRequest, const QString &host, int port, bool useSsl);

signals:
    void responseReceived(QByteArray rawResponse);
    void errorOccurred(QString errorMessage);

private slots:
    void onConnected();
    void onSslConnected();
    void onReadyRead();
    void onDisconnected();
    void onSocketError(QAbstractSocket::SocketError err);
    void onOverallTimeout();

private:
    void doSend();
    void finish();
    void hardReset();

    QByteArray  m_request;
    QByteArray  m_response;
    QString     m_host;
    int         m_port    = 80;
    bool        m_useSsl  = false;

    QTcpSocket *m_socket        = nullptr;
    QTimer     *m_overallTimer  = nullptr;   // 20s hard limit
    bool        m_finished      = false;
};
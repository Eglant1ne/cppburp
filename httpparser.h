#pragma once
#include <QString>
#include <QByteArray>
#include <QMap>

struct HttpRequest {
    QString method;
    QString url;        // full URL or path
    QString host;
    int     port      = 80;
    bool    isTls     = false;
    bool    isConnect = false;
    QString version;
    QMap<QString, QString> headers;
    QByteArray body;
    QByteArray raw;    // original bytes
};

class HttpParser {
public:
    // Parse raw bytes into HttpRequest.
    // Returns true if a complete request header was found.
    static bool parse(const QByteArray &data, HttpRequest &out);

    // Extract Host + Port from a "host:port" string.
    static void splitHostPort(const QString &hostPort, QString &host, int &port, bool isTls);
};

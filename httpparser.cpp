// SPDX-License-Identifier: MIT
// Encoding: UTF-8
#include "httpparser.h"
#include <QStringList>
#include <stdexcept>

bool HttpParser::parse(const QByteArray& data, HttpRequest& out)
{
    out.raw = data;

    // Find end of headers (\r\n\r\n)
    int headerEnd = data.indexOf("\r\n\r\n");
    if (headerEnd < 0) return false;

    QByteArray headerPart = data.left(headerEnd);
    out.body = data.mid(headerEnd + 4);

    QList<QByteArray> lines = headerPart.split('\n');
    if (lines.isEmpty()) return false;

    // Request line
    QString requestLine = QString::fromLatin1(lines[0]).trimmed();
    QStringList parts = requestLine.split(' ');
    if (parts.size() < 3)
        throw std::invalid_argument(
            "Malformed HTTP request line: fewer than three tokens");

    out.method = parts[0].toUpper();
    out.url = parts[1];
    out.version = parts[2];
    out.isConnect = (out.method == "CONNECT");

    // Headers
    for (int i = 1; i < lines.size(); ++i) {
        QString line = QString::fromLatin1(lines[i]).trimmed();
        int colonIdx = line.indexOf(':');
        if (colonIdx < 0) continue;
        QString key = line.left(colonIdx).trimmed().toLower();
        QString value = line.mid(colonIdx + 1).trimmed();
        out.headers[key] = value;
    }

    // Resolve host/port
    QString hostHeader = out.headers.value("host");
    if (out.isConnect) {
        // CONNECT host:port  →  TLS tunnel
        splitHostPort(out.url, out.host, out.port, /*isTls=*/true);
        out.isTls = true;
    } else if (!hostHeader.isEmpty()) {
        bool defaultTls = out.url.startsWith("https://");
        splitHostPort(hostHeader, out.host, out.port, defaultTls);
        out.isTls = defaultTls;
    }

    return true;
}

void HttpParser::splitHostPort(const QString& hostPort, QString& host, int& port, bool isTls)
{
    // Handle IPv6 [::1]:8080
    if (hostPort.startsWith('[')) {
        int closeBracket = hostPort.indexOf(']');
        if (closeBracket < 0) {
            host = hostPort;
            port = isTls ? 443 : 80;
            return;
        }
        host = hostPort.mid(1, closeBracket - 1);
        QString rest = hostPort.mid(closeBracket + 1);
        if (rest.startsWith(':'))
            port = rest.mid(1).toInt();
        else
            port = isTls ? 443 : 80;
        return;
    }

    int colonIdx = hostPort.lastIndexOf(':');
    if (colonIdx > 0) {
        bool ok = false;
        int p = hostPort.mid(colonIdx + 1).toInt(&ok);
        if (ok) {
            host = hostPort.left(colonIdx);
            port = p;
            return;
        }
    }
    host = hostPort;
    port = isTls ? 443 : 80;
}

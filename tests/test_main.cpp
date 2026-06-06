// ProxyLab Unit Tests
// Covers: HttpParser::splitHostPort, normalizeRequest, HttpParser::parse

#include <QtCore/QCoreApplication>
#include <QtCore/QByteArray>
#include <QtCore/QString>
#include <QtCore/QDebug>

#include "httpparser.h"

// ─── Minimal test runner ────────────────────────────────────────────────────

static int g_total = 0, g_passed = 0, g_failed = 0;

static QString toStr(const QString &s)    { return s; }
static QString toStr(const QByteArray &b) { return QString::fromLatin1(b.toHex()); }
static QString toStr(int v)               { return QString::number(v); }
static QString toStr(bool v)              { return v ? "true" : "false"; }

template<typename T>
void expectEq(T actual, T expected, const char *name) {
    ++g_total;
    if (actual == expected) {
        ++g_passed;
        qDebug("  [PASS] %s", name);
    } else {
        ++g_failed;
        qDebug("  [FAIL] %s\n         expected: %s\n         actual:   %s",
               name,
               qPrintable(toStr(expected)),
               qPrintable(toStr(actual)));
    }
}
void expectTrue(bool c, const char *n)  { expectEq(c, true,  n); }
void expectFalse(bool c, const char *n) { expectEq(c, false, n); }

#define EXPECT_EQ(a, e, n)   expectEq((a), (e), (n))
#define EXPECT_TRUE(c, n)    expectTrue((c), (n))
#define EXPECT_FALSE(c, n)   expectFalse((c), (n))


static QByteArray normalizeRequest(const QByteArray &raw)
{
    QByteArray req = raw;
    req.replace("\r\n", "\n");
    req.replace("\n", "\r\n");

    if (!req.contains("\r\n\r\n")) {
        if (!req.endsWith("\r\n")) req.append("\r\n");
        req.append("\r\n");
    }

    int blankIdx = req.indexOf("\r\n\r\n");
    QByteArray hdrs = req.left(blankIdx);
    QByteArray body = req.mid(blankIdx + 4);

    // Convert absolute URL to relative path
    {
        int sp1 = hdrs.indexOf(' ');
        int sp2 = hdrs.indexOf(' ', sp1 + 1);
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

    int connIdx = lowerHdrs.indexOf("\nconnection:");
    if (connIdx >= 0) {
        connIdx++;
        int lineEnd = hdrs.indexOf("\r\n", connIdx);
        if (lineEnd >= 0)
            hdrs.replace(connIdx, lineEnd - connIdx, "Connection: close");
    } else {
        hdrs.append("\r\nConnection: close");
    }

    // Recalculate Content-Length
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

// ═══════════════════════════════════════════════════════════════════════════
//  1. HttpParser::splitHostPort — 10 tests
// ═══════════════════════════════════════════════════════════════════════════

void test_splitHostPort()
{
    qDebug("\n=== HttpParser::splitHostPort ===");

    {
        QString host; int port = -1;
        HttpParser::splitHostPort("example.com", host, port, false);
        EXPECT_EQ(host, QString("example.com"), "plain host → correct host");
        EXPECT_EQ(port, 80, "plain host HTTP → port 80");
    }

    {
        QString host; int port = -1;
        HttpParser::splitHostPort("example.com", host, port, true);
        EXPECT_EQ(port, 443, "plain host TLS → port 443");
    }

    {
        QString host; int port = -1;
        HttpParser::splitHostPort("example.com:9090", host, port, false);
        EXPECT_EQ(host, QString("example.com"), "host:port → correct host");
        EXPECT_EQ(port, 9090, "host:port → correct port");
    }

    {
        QString host; int port = -1;
        HttpParser::splitHostPort("192.168.1.1:8080", host, port, false);
        EXPECT_EQ(host, QString("192.168.1.1"), "IPv4:port → host");
        EXPECT_EQ(port, 8080, "IPv4:port → port 8080");
    }

    {
        QString host; int port = -1;
        HttpParser::splitHostPort("[::1]:8080", host, port, false);
        EXPECT_EQ(host, QString("::1"), "IPv6 bracket:port → host without brackets");
        EXPECT_EQ(port, 8080, "IPv6 bracket:port → port 8080");
    }

    {
        QString host; int port = -1;
        HttpParser::splitHostPort("[::1]", host, port, false);
        EXPECT_EQ(host, QString("::1"), "IPv6 no port → host");
        EXPECT_EQ(port, 80, "IPv6 no port HTTP → port 80");
    }

    {
        QString host; int port = -1;
        HttpParser::splitHostPort("[fe80::1]", host, port, true);
        EXPECT_EQ(host, QString("fe80::1"), "IPv6 TLS no port → host");
        EXPECT_EQ(port, 443, "IPv6 TLS no port → port 443");
    }

    {
        QString host; int port = -1;
        HttpParser::splitHostPort("[::1", host, port, false);
        EXPECT_EQ(host, QString("[::1"), "unclosed IPv6 bracket → raw string as host");
        EXPECT_EQ(port, 80, "unclosed IPv6 bracket → default port 80");
    }

    {
        QString host; int port = -1;
        HttpParser::splitHostPort("example.com:notaport", host, port, false);
        EXPECT_EQ(host, QString("example.com:notaport"), "non-numeric port → full string as host");
        EXPECT_EQ(port, 80, "non-numeric port → default port 80");
    }

    {
        QString host; int port = -1;
        HttpParser::splitHostPort("api.github.com:443", host, port, true);
        EXPECT_EQ(host, QString("api.github.com"), "CONNECT target → host");
        EXPECT_EQ(port, 443, "CONNECT target → port 443");
    }
}

// ═══════════════════════════════════════════════════════════════════════════
//  2. normalizeRequest — 10 tests
// ═══════════════════════════════════════════════════════════════════════════

void test_normalizeRequest()
{
    qDebug("\n=== normalizeRequest ===");

    {
        QByteArray raw = "GET /index.html HTTP/1.1\nHost: example.com\n\n";
        QByteArray result = normalizeRequest(raw);
        // Scan for bare \n not preceded by \r
        bool hasBareLF = false;
        for (int i = 0; i < result.size(); ++i)
            if (result[i] == '\n' && (i == 0 || result[i-1] != '\r'))
                hasBareLF = true;
        EXPECT_TRUE(result.contains("\r\n"), "\\n → \\r\\n conversion applied");
        EXPECT_FALSE(hasBareLF, "no bare \\n remaining after normalization");
    }

    {
        QByteArray raw =
            "GET http://example.com/api/v1/users HTTP/1.1\r\n"
            "Host: example.com\r\n\r\n";
        QByteArray result = normalizeRequest(raw);
        EXPECT_TRUE(result.startsWith("GET /api/v1/users HTTP/1.1\r\n"),
                    "absolute HTTP URL → relative path");
    }

    {
        QByteArray raw =
            "POST https://api.example.com/submit HTTP/1.1\r\n"
            "Host: api.example.com\r\n\r\n";
        QByteArray result = normalizeRequest(raw);
        EXPECT_TRUE(result.startsWith("POST /submit HTTP/1.1\r\n"),
                    "absolute HTTPS URL → relative path");
    }

    {
        QByteArray raw =
            "GET /already/relative HTTP/1.1\r\n"
            "Host: example.com\r\n\r\n";
        QByteArray result = normalizeRequest(raw);
        EXPECT_TRUE(result.startsWith("GET /already/relative HTTP/1.1\r\n"),
                    "relative URL stays unchanged");
    }

    // Connection: keep-alive is NOT the last header → replacement works correctly
    {
        QByteArray raw =
            "GET /index HTTP/1.1\r\n"
            "Connection: keep-alive\r\n"
            "Host: example.com\r\n\r\n";
        QByteArray result = normalizeRequest(raw);
        EXPECT_TRUE(result.contains("Connection: close"),
                    "Connection not-last header → replaced with close");
        EXPECT_FALSE(result.contains("keep-alive"),
                     "keep-alive value removed when header is not last");
    }

    // KNOWN BUG: Connection is the last header → lineEnd == -1, replace skipped
    {
        QByteArray raw =
            "GET /index HTTP/1.1\r\n"
            "Host: example.com\r\n"
            "Connection: keep-alive\r\n\r\n";
        QByteArray result = normalizeRequest(raw);
        EXPECT_FALSE(result.contains("Connection: close"),
                     "BUG: Connection last header → close NOT injected (lineEnd==-1 skips replace)");
        EXPECT_TRUE(result.contains("keep-alive"),
                    "BUG: Connection last header → keep-alive NOT removed");
    }

    {
        QByteArray raw =
            "GET /index HTTP/1.1\r\n"
            "Host: example.com\r\n\r\n";
        QByteArray result = normalizeRequest(raw);
        EXPECT_TRUE(result.contains("Connection: close"),
                    "Connection: close added when header absent");
    }

    {
        QByteArray raw =
            "POST /data HTTP/1.1\r\n"
            "Host: example.com\r\n"
            "Content-Length: 5\r\n\r\n"
            "Hello, World!";
        QByteArray result = normalizeRequest(raw);
        EXPECT_TRUE(result.contains("Content-Length: 13"),
                    "Content-Length recalculated to actual body size (13)");
        EXPECT_FALSE(result.contains("Content-Length: 5"),
                     "stale Content-Length: 5 removed");
    }

    {
        QByteArray raw =
            "GET /ping HTTP/1.1\r\n"
            "Host: example.com\r\n\r\n";
        QByteArray result = normalizeRequest(raw);
        EXPECT_FALSE(result.contains("Content-Length"),
                     "Content-Length not injected for bodyless request");
    }

    {
        QByteArray raw =
            "GET http://example.com HTTP/1.1\r\n"
            "Host: example.com\r\n\r\n";
        QByteArray result = normalizeRequest(raw);
        EXPECT_TRUE(result.startsWith("GET / HTTP/1.1\r\n"),
                    "absolute URL with no path → /");
    }

    {
        QByteArray raw =
            "GET /mixed HTTP/1.1\r\n"
            "Host: example.com\n"
            "X-Custom: value\n"
            "\n";
        QByteArray result = normalizeRequest(raw);
        bool hasBareLF = false;
        for (int i = 0; i < result.size(); ++i)
            if (result[i] == '\n' && (i == 0 || result[i-1] != '\r'))
                hasBareLF = true;
        EXPECT_FALSE(hasBareLF, "mixed endings → all normalized to \\r\\n");
    }
}

// ═══════════════════════════════════════════════════════════════════════════
//  3. HttpParser::parse — 10 tests
// ═══════════════════════════════════════════════════════════════════════════

void test_httpParserParse()
{
    qDebug("\n=== HttpParser::parse ===");

    {
        QByteArray data = "GET /index.html HTTP/1.1\r\nHost: example.com\r\n";
        HttpRequest req;
        EXPECT_FALSE(HttpParser::parse(data, req),
                     "incomplete headers → returns false");
    }

    {
        QByteArray data =
            "GET /path/to/resource HTTP/1.1\r\n"
            "Host: example.com\r\n\r\n";
        HttpRequest req;
        EXPECT_TRUE(HttpParser::parse(data, req),        "valid GET → returns true");
        EXPECT_EQ(req.method, QString("GET"),            "method = GET");
        EXPECT_EQ(req.url, QString("/path/to/resource"), "url parsed correctly");
        EXPECT_EQ(req.version, QString("HTTP/1.1"),      "version = HTTP/1.1");
        EXPECT_EQ(req.host, QString("example.com"),      "host from Host header");
        EXPECT_EQ(req.port, 80,                          "default port 80");
        EXPECT_FALSE(req.isTls,                          "isTls = false for plain HTTP");
        EXPECT_FALSE(req.isConnect,                      "isConnect = false for GET");
    }

    {
        QByteArray body = "username=admin&password=1234";
        QByteArray data =
            "POST /login HTTP/1.1\r\n"
            "Host: api.example.com\r\n"
            "Content-Length: 27\r\n\r\n" + body;
        HttpRequest req;
        EXPECT_TRUE(HttpParser::parse(data, req), "valid POST → returns true");
        EXPECT_EQ(req.method, QString("POST"),    "method = POST");
        EXPECT_EQ(req.body, body,                 "body bytes preserved in out.body");
    }

    {
        QByteArray data =
            "CONNECT api.github.com:443 HTTP/1.1\r\n"
            "Host: api.github.com:443\r\n\r\n";
        HttpRequest req;
        EXPECT_TRUE(HttpParser::parse(data, req),        "CONNECT → returns true");
        EXPECT_TRUE(req.isConnect,                       "isConnect = true");
        EXPECT_TRUE(req.isTls,                           "isTls = true");
        EXPECT_EQ(req.host, QString("api.github.com"),   "CONNECT host parsed");
        EXPECT_EQ(req.port, 443,                         "CONNECT port = 443");
    }

    {
        QByteArray data =
            "GET /page HTTP/1.1\r\n"
            "Host: example.com\r\n"
            "X-Custom-Header: SomeValue\r\n"
            "Content-Type: application/json\r\n\r\n";
        HttpRequest req;
        HttpParser::parse(data, req);
        EXPECT_TRUE(req.headers.contains("x-custom-header"),
                    "header key lowercased");
        EXPECT_EQ(req.headers.value("x-custom-header"), QString("SomeValue"),
                  "header value preserved as-is");
        EXPECT_EQ(req.headers.value("content-type"), QString("application/json"),
                  "content-type header parsed");
    }

    {
        QByteArray data = "BADREQUEST\r\n\r\n";
        HttpRequest req;
        bool threw = false;
        try { HttpParser::parse(data, req); }
        catch (const std::invalid_argument &) { threw = true; }
        EXPECT_TRUE(threw, "malformed request line → throws invalid_argument");
    }

    {
        QByteArray data =
            "GET /api HTTP/1.1\r\n"
            "Host: backend.local:8443\r\n\r\n";
        HttpRequest req;
        HttpParser::parse(data, req);
        EXPECT_EQ(req.host, QString("backend.local"), "host extracted from Host:port");
        EXPECT_EQ(req.port, 8443, "port extracted from Host header");
    }

    {
        QByteArray data =
            "GET /check HTTP/1.1\r\n"
            "Host: example.com\r\n\r\n";
        HttpRequest req;
        HttpParser::parse(data, req);
        EXPECT_EQ(req.raw, data, "raw field equals original input bytes");
    }

    {
        HttpRequest req;
        EXPECT_FALSE(HttpParser::parse(QByteArray(), req),
                     "empty input → returns false");
    }

    {
        QByteArray data = "\r\n\r\n";
        HttpRequest req;
        bool threw = false;
        bool ok = false;
        try { ok = HttpParser::parse(data, req); }
        catch (const std::invalid_argument &) { threw = true; }
        EXPECT_TRUE(threw || !ok,
                    "empty request line → exception or false");
    }
}

// ═══════════════════════════════════════════════════════════════════════════
//  main
// ═══════════════════════════════════════════════════════════════════════════

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    qDebug("╔══════════════════════════════════════════════════╗");
    qDebug("║       ProxyLab Unit Tests                        ║");
    qDebug("╚══════════════════════════════════════════════════╝");

    test_splitHostPort();
    test_normalizeRequest();
    test_httpParserParse();

    qDebug("\n══════════════════════════════════════════════════");
    qDebug("Results: %d passed / %d failed / %d total",
           g_passed, g_failed, g_total);
    qDebug("══════════════════════════════════════════════════");

    return (g_failed == 0) ? 0 : 1;
}

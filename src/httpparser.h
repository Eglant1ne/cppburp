// SPDX-License-Identifier: MIT
// Encoding: UTF-8
#pragma once
#include <QString>
#include <QByteArray>
#include <QMap>
#include <stdexcept>

/**
 * @brief Parsed representation of an HTTP/1.x request.
 *
 * Filled in by HttpParser::parse(). All fields are left at their
 * default values if parsing fails.
 */
struct HttpRequest {
    QString method;   ///< HTTP method in upper-case (e.g. "GET", "POST", "CONNECT").
    QString url;      ///< Full URL or path as it appears in the request line.
    QString host;     ///< Resolved target hostname (from Host header or CONNECT target).
    int port = 80;    ///< Resolved target port (default 80 for HTTP, 443 for HTTPS).
    bool isTls = false;     ///< True when the connection should use TLS.
    bool isConnect = false; ///< True when the method is CONNECT (tunnel setup).
    QString version;        ///< HTTP version string (e.g. "HTTP/1.1").
    QMap<QString, QString> headers; ///< Header map; keys are lower-cased.
    QByteArray body;  ///< Request body (bytes after the blank line).
    QByteArray raw;   ///< Original unmodified bytes passed to parse().
};

/**
 * @brief Lightweight HTTP/1.x request parser.
 *
 * Parses only the request headers and the first line; does not
 * handle chunked bodies or HTTP/2. Designed for proxy interception
 * where the full stream is already buffered.
 */
class HttpParser {
public:
    /**
     * @brief Parse raw bytes into an HttpRequest structure.
     *
     * Looks for the end-of-headers marker (\\r\\n\\r\\n). If it is not
     * yet present in @p data the function returns false and the caller
     * should buffer more data before retrying.
     *
     * @param data  Raw bytes received from the client socket.
     * @param out   Output structure; overwritten on success.
     * @return true  A complete header section was found and parsed.
     * @return false The header section is incomplete; @p out is unchanged.
     * @throws std::invalid_argument if the request line contains fewer than
     *         three whitespace-separated tokens.
     */
    static bool parse(const QByteArray& data, HttpRequest& out);

    /**
     * @brief Split a "host:port" string into separate host and port values.
     *
     * Handles IPv6 addresses in bracket notation (e.g. "[::1]:8080").
     * If no port is found the default is chosen based on @p isTls.
     *
     * @param hostPort  Input string in "host", "host:port", or "[ipv6]:port" form.
     * @param host      Output: hostname without brackets or port.
     * @param port      Output: port number (defaults to 443 when @p isTls is true,
     *                  80 otherwise).
     * @param isTls     Whether to apply the HTTPS default port (443).
     */
    static void splitHostPort(const QString& hostPort, QString& host, int& port, bool isTls);
};

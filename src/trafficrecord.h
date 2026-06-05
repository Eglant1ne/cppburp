// SPDX-License-Identifier: MIT
// Encoding: UTF-8
#pragma once
#include <QString>
#include <QDateTime>

/**
 * @brief Represents a single captured HTTP traffic entry.
 *
 * Stores all metadata and raw content for one request/response cycle
 * captured by the proxy server.
 */
struct TrafficRecord {
    int id = 0;           ///< Unique session-local identifier.
    QDateTime time;       ///< Timestamp when the connection was initiated.
    QString method;       ///< HTTP method (GET, POST, etc.).
    QString host;         ///< Target hostname extracted from Host header.
    QString path;         ///< Request path (e.g. "/api/v1/users").
    int status = 0;       ///< HTTP response status code (0 if unknown).
    int size = 0;         ///< Response body size in bytes.
    QString rawRequest;   ///< Full raw HTTP request as text.
    QString rawResponse;  ///< Full raw HTTP response as text.
    bool intercepted = false; ///< True if the request was intercepted by the user.
};

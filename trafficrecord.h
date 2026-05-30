#pragma once
#include <QString>
#include <QDateTime>

struct TrafficRecord {
    int       id       = 0;
    QDateTime time;
    QString   method;
    QString   host;
    QString   path;
    int       status   = 0;
    int       size     = 0;
    QString   rawRequest;
    QString   rawResponse;
    bool      intercepted = false;
};

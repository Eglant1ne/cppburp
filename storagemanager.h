#pragma once

#include <QObject>
#include <QSqlDatabase>
#include <QVector>
#include "trafficrecord.h"

struct SavedGroup {
    int     id   = 0;
    QString name;
};

struct SavedRequest {
    int     id      = 0;
    int     groupId = 0;
    QString name;
    QString rawRequest;
    QString host;
    int     port   = 80;
    bool    useSsl = false;
};

class StorageManager : public QObject
{
    Q_OBJECT
public:
    explicit StorageManager(QObject *parent = nullptr);
    ~StorageManager();

    bool open(const QString &filePath);

    // Traffic history
    void        saveRecord(const TrafficRecord &rec);
    QVector<TrafficRecord> loadHistory(int limit = 500);
    void        clearHistory();

    // Collections / Groups
    int         createGroup(const QString &name);
    QVector<SavedGroup>   loadGroups();
    void        deleteGroup(int groupId);

    // Saved requests
    int         saveRequest(int groupId, const QString &name,
                             const QString &rawRequest,
                             const QString &host, int port, bool useSsl);
    QVector<SavedRequest> loadRequests(int groupId);
    SavedRequest          loadRequest(int requestId);
    void        deleteRequest(int requestId);

private:
    void createSchema();
    QSqlDatabase m_db;
};

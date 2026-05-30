#include "storagemanager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>

StorageManager::StorageManager(QObject *parent)
    : QObject(parent)
{}

StorageManager::~StorageManager()
{
    if (m_db.isOpen())
        m_db.close();
}

bool StorageManager::open(const QString &filePath)
{
    m_db = QSqlDatabase::addDatabase("QSQLITE", "proxylab_conn");
    m_db.setDatabaseName(filePath);
    if (!m_db.open()) {
        qWarning() << "StorageManager: cannot open DB:" << m_db.lastError().text();
        return false;
    }
    createSchema();
    return true;
}

void StorageManager::createSchema()
{
    QSqlQuery q(m_db);
    q.exec("PRAGMA journal_mode=WAL;");
    q.exec(R"(
        CREATE TABLE IF NOT EXISTS traffic (
            id           INTEGER PRIMARY KEY AUTOINCREMENT,
            time         TEXT,
            method       TEXT,
            host         TEXT,
            path         TEXT,
            status       INTEGER,
            size         INTEGER,
            raw_request  TEXT,
            raw_response TEXT
        )
    )");
    q.exec(R"(
        CREATE TABLE IF NOT EXISTS groups (
            id   INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL
        )
    )");
    q.exec(R"(
        CREATE TABLE IF NOT EXISTS saved_requests (
            id          INTEGER PRIMARY KEY AUTOINCREMENT,
            group_id    INTEGER REFERENCES groups(id) ON DELETE CASCADE,
            name        TEXT,
            raw_request TEXT,
            host        TEXT,
            port        INTEGER,
            use_ssl     INTEGER
        )
    )");
}

// ── Traffic history ──────────────────────────────────────────────────────────

void StorageManager::saveRecord(const TrafficRecord &rec)
{
    QSqlQuery q(m_db);
    q.prepare(R"(
        INSERT INTO traffic (time,method,host,path,status,size,raw_request,raw_response)
        VALUES (:time,:method,:host,:path,:status,:size,:req,:resp)
    )");
    q.bindValue(":time",   rec.time.toString(Qt::ISODate));
    q.bindValue(":method", rec.method);
    q.bindValue(":host",   rec.host);
    q.bindValue(":path",   rec.path);
    q.bindValue(":status", rec.status);
    q.bindValue(":size",   rec.size);
    q.bindValue(":req",    rec.rawRequest);
    q.bindValue(":resp",   rec.rawResponse);
    if (!q.exec())
        qWarning() << "saveRecord:" << q.lastError().text();
}

QVector<TrafficRecord> StorageManager::loadHistory(int limit)
{
    QVector<TrafficRecord> result;
    QSqlQuery q(m_db);
    q.prepare("SELECT id,time,method,host,path,status,size,raw_request,raw_response "
              "FROM traffic ORDER BY id DESC LIMIT :lim");
    q.bindValue(":lim", limit);
    q.exec();
    while (q.next()) {
        TrafficRecord r;
        r.id          = q.value(0).toInt();
        r.time        = QDateTime::fromString(q.value(1).toString(), Qt::ISODate);
        r.method      = q.value(2).toString();
        r.host        = q.value(3).toString();
        r.path        = q.value(4).toString();
        r.status      = q.value(5).toInt();
        r.size        = q.value(6).toInt();
        r.rawRequest  = q.value(7).toString();
        r.rawResponse = q.value(8).toString();
        result.append(r);
    }
    return result;
}

void StorageManager::clearHistory()
{
    QSqlQuery q(m_db);
    q.exec("DELETE FROM traffic");
}

// ── Groups ───────────────────────────────────────────────────────────────────

int StorageManager::createGroup(const QString &name)
{
    QSqlQuery q(m_db);
    q.prepare("INSERT INTO groups (name) VALUES (:name)");
    q.bindValue(":name", name);
    if (!q.exec()) return -1;
    return static_cast<int>(q.lastInsertId().toLongLong());
}

QVector<SavedGroup> StorageManager::loadGroups()
{
    QVector<SavedGroup> result;
    QSqlQuery q("SELECT id, name FROM groups ORDER BY id", m_db);
    while (q.next()) {
        SavedGroup g;
        g.id   = q.value(0).toInt();
        g.name = q.value(1).toString();
        result.append(g);
    }
    return result;
}

void StorageManager::deleteGroup(int groupId)
{
    QSqlQuery q(m_db);
    q.prepare("DELETE FROM groups WHERE id=:id");
    q.bindValue(":id", groupId);
    q.exec();
}

// ── Saved requests ───────────────────────────────────────────────────────────

int StorageManager::saveRequest(int groupId, const QString &name,
                                  const QString &rawRequest,
                                  const QString &host, int port, bool useSsl)
{
    QSqlQuery q(m_db);
    q.prepare("INSERT INTO saved_requests (group_id,name,raw_request,host,port,use_ssl) "
              "VALUES (:gid,:name,:req,:host,:port,:ssl)");
    q.bindValue(":gid",  groupId);
    q.bindValue(":name", name);
    q.bindValue(":req",  rawRequest);
    q.bindValue(":host", host);
    q.bindValue(":port", port);
    q.bindValue(":ssl",  useSsl ? 1 : 0);
    if (!q.exec()) return -1;
    return static_cast<int>(q.lastInsertId().toLongLong());
}

QVector<SavedRequest> StorageManager::loadRequests(int groupId)
{
    QVector<SavedRequest> result;
    QSqlQuery q(m_db);
    q.prepare("SELECT id,group_id,name,raw_request,host,port,use_ssl "
              "FROM saved_requests WHERE group_id=:gid ORDER BY id");
    q.bindValue(":gid", groupId);
    q.exec();
    while (q.next()) {
        SavedRequest r;
        r.id         = q.value(0).toInt();
        r.groupId    = q.value(1).toInt();
        r.name       = q.value(2).toString();
        r.rawRequest = q.value(3).toString();
        r.host       = q.value(4).toString();
        r.port       = q.value(5).toInt();
        r.useSsl     = q.value(6).toInt() != 0;
        result.append(r);
    }
    return result;
}

SavedRequest StorageManager::loadRequest(int requestId)
{
    SavedRequest r;
    QSqlQuery q(m_db);
    q.prepare("SELECT id,group_id,name,raw_request,host,port,use_ssl "
              "FROM saved_requests WHERE id=:id");
    q.bindValue(":id", requestId);
    if (q.exec() && q.next()) {
        r.id         = q.value(0).toInt();
        r.groupId    = q.value(1).toInt();
        r.name       = q.value(2).toString();
        r.rawRequest = q.value(3).toString();
        r.host       = q.value(4).toString();
        r.port       = q.value(5).toInt();
        r.useSsl     = q.value(6).toInt() != 0;
    }
    return r;
}

void StorageManager::deleteRequest(int requestId)
{
    QSqlQuery q(m_db);
    q.prepare("DELETE FROM saved_requests WHERE id=:id");
    q.bindValue(":id", requestId);
    q.exec();
}

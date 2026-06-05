// SPDX-License-Identifier: MIT
// Encoding: UTF-8
#pragma once

#include <QObject>
#include <QSqlDatabase>
#include <QVector>
#include "trafficrecord.h"

/**
 * @brief A named group (folder) for saved requests in the Collections panel.
 */
struct SavedGroup {
    int id = 0;    ///< Primary key in the groups table.
    QString name;  ///< Display name chosen by the user.
};

/**
 * @brief A single saved HTTP request stored inside a SavedGroup.
 */
struct SavedRequest {
    int id = 0;          ///< Primary key in the saved_requests table.
    int groupId = 0;     ///< Foreign key to the parent SavedGroup.
    QString name;        ///< User-defined display name.
    QString rawRequest;  ///< Full raw HTTP request text.
    QString host;        ///< Target hostname.
    int port = 80;       ///< Target port.
    bool useSsl = false; ///< Whether TLS should be used when replaying.
};

/**
 * @brief SQLite persistence layer for traffic history and request collections.
 *
 * Uses a named QSqlDatabase connection ("proxylab_conn") to avoid conflicts
 * with other database connections in the application. The schema is created
 * automatically on the first open().
 *
 * All methods are synchronous and execute on the calling thread.
 */
class StorageManager : public QObject {
    Q_OBJECT
public:
    /**
     * @brief Construct the manager; does not open the database yet.
     * @param parent Qt parent object.
     */
    explicit StorageManager(QObject* parent = nullptr);

    /**
     * @brief Close the database connection if open.
     */
    ~StorageManager();

    /**
     * @brief Open (or create) the SQLite database at @p filePath.
     *
     * Creates the schema tables if they do not exist. The directory must
     * already exist; use QDir::mkpath() before calling this method.
     *
     * @param filePath  Absolute path to the .sqlite file.
     * @return true on success.
     * @throws std::runtime_error if the database cannot be opened.
     */
    bool open(const QString& filePath);

    // ── Traffic history ────────────────────────────────────────────────────

    /**
     * @brief Persist a completed traffic record to the database.
     * @param rec  Record to save; its @c id field is ignored (auto-assigned).
     */
    void saveRecord(const TrafficRecord& rec);

    /**
     * @brief Load the most recent traffic records from the database.
     * @param limit  Maximum number of records to return (most recent first).
     * @return Vector of records ordered by descending insertion ID.
     */
    QVector<TrafficRecord> loadHistory(int limit = 500);

    /**
     * @brief Delete all rows from the traffic table.
     */
    void clearHistory();

    // ── Groups ─────────────────────────────────────────────────────────────

    /**
     * @brief Create a new collection group.
     * @param name  Display name for the group.
     * @return The new group's primary key, or -1 on failure.
     */
    int createGroup(const QString& name);

    /**
     * @brief Load all collection groups ordered by creation time.
     * @return Vector of SavedGroup structs.
     */
    QVector<SavedGroup> loadGroups();

    /**
     * @brief Delete a group and all its saved requests (CASCADE).
     * @param groupId  Primary key of the group to delete.
     */
    void deleteGroup(int groupId);

    // ── Saved requests ─────────────────────────────────────────────────────

    /**
     * @brief Save a request into an existing group.
     * @param groupId     Target group's primary key.
     * @param name        Display name for the saved request.
     * @param rawRequest  Raw HTTP request text.
     * @param host        Target hostname.
     * @param port        Target port.
     * @param useSsl      Whether TLS should be used.
     * @return The new request's primary key, or -1 on failure.
     */
    int saveRequest(int groupId, const QString& name, const QString& rawRequest,
                    const QString& host, int port, bool useSsl);

    /**
     * @brief Load all requests belonging to a group.
     * @param groupId  Parent group's primary key.
     * @return Vector of SavedRequest structs ordered by insertion ID.
     */
    QVector<SavedRequest> loadRequests(int groupId);

    /**
     * @brief Load a single saved request by its primary key.
     * @param requestId  Primary key of the request to load.
     * @return Populated SavedRequest; all fields are default if not found.
     */
    SavedRequest loadRequest(int requestId);

    /**
     * @brief Delete a single saved request by its primary key.
     * @param requestId  Primary key of the request to delete.
     */
    void deleteRequest(int requestId);

private:
    /**
     * @brief Create the schema tables if they do not already exist.
     */
    void createSchema();

    QSqlDatabase m_db;  ///< Named database connection.
};

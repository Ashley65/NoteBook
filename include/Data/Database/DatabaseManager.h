//
// Created by DevAccount on 09/09/2026.
//

#ifndef TASKHELPER_DATABASEMANAGER_H
#define TASKHELPER_DATABASEMANAGER_H
#pragma once

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QUuid>
#include <QList>
#include <optional>
#include "DataSchema.h"

/**
 * @file DatabaseManager.h
 * @brief Low-level SQLite database persistence manager.
 */

/**
 * @class DatabaseManager
 * @brief Handles SQLite connectivity, schema initialization, migrations, and CRUD operations.
 *
 * DatabaseManager encapsulates low-level SQL queries and transactions for Workspaces, Projects,
 * Tasks, Subtasks, Notes, and FileAttachments using Qt's `QSqlDatabase` framework. Foreign keys
 * and cascade deletions are enforced directly via SQLite schema definitions.
 */
class DatabaseManager
{
public:
    /**
     * @brief Constructs a DatabaseManager with a specific unique connection name.
     * @param connectionName Name used by QSqlDatabase to identify this SQLite connection.
     */
    explicit DatabaseManager(const QString& connectionName = "flow_db_connection");

    /**
     * @brief Destructor. Closes and removes the database connection.
     */
    ~DatabaseManager();

    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    /**
     * @brief Opens the SQLite database file at the given filesystem path.
     * @param dbPath Absolute or relative path to the .db file.
     * @return True if opened successfully and foreign keys were enabled; false otherwise.
     */
    bool open(const QString& dbPath);

    /**
     * @brief Closes the active database connection.
     */
    void close();

    /**
     * @brief Checks if the database connection is currently open and valid.
     * @return True if connected and open, false otherwise.
     */
    bool isOpen() const;

    /**
     * @brief Initializes the SQLite tables, indices, and triggers from schema.sql.
     * @return True if schema setup succeeded without errors.
     */
    bool initializeSchema();

    /**
     * @brief Returns the last encountered error message string.
     */
    QString lastError() const;

    /**
     * @brief Begins an explicit database transaction.
     * @return True if transaction began successfully.
     */
    bool beginTransaction();

    /**
     * @brief Commits the currently active database transaction.
     * @return True if transaction was committed successfully.
     */
    bool commitTransaction();

    /**
     * @brief Rolls back the currently active database transaction.
     * @return True if rollback completed.
     */
    bool rollbackTransaction();

    // =========================================================================
    // Workspace Operations
    // =========================================================================

    /**
     * @brief Inserts a new workspace into the database.
     * @param ws Workspace domain model with initial properties.
     * @return Generated or existing workspace QUuid, or null QUuid on failure.
     */
    QUuid addWorkspace(const Workspace& ws);

    /**
     * @brief Retrieves a workspace by its unique identifier.
     * @param id Workspace UUID.
     * @return std::optional containing the Workspace if found, or std::nullopt.
     */
    std::optional<Workspace> getWorkspace(const QUuid& id) const;

    /**
     * @brief Returns all workspaces in the database.
     * @param includeArchived Whether to include archived workspaces in the result.
     */
    QList<Workspace> getAllWorkspaces(bool includeArchived = false) const;

    /**
     * @brief Updates an existing workspace record.
     * @param ws Workspace containing updated fields matching `ws.id`.
     * @return True on success, false on error.
     */
    bool updateWorkspace(const Workspace& ws);

    /**
     * @brief Deletes a workspace and cascades deletion to child projects, tasks, and notes.
     * @param id Identifier of the workspace to delete.
     * @return True if deleted successfully.
     */
    bool deleteWorkspace(const QUuid& id);

    // =========================================================================
    // Project Operations
    // =========================================================================

    /**
     * @brief Inserts a new project associated with a workspace.
     * @param proj Project domain model.
     * @return Project QUuid, or null QUuid on failure.
     */
    QUuid addProject(const Project& proj);

    /**
     * @brief Fetches a single project by its unique ID.
     * @param id Project UUID.
     * @return std::optional with Project if found, std::nullopt otherwise.
     */
    std::optional<Project> getProject(const QUuid& id) const;

    /**
     * @brief Returns all projects belonging to a given workspace.
     * @param workspaceId Parent workspace UUID.
     */
    QList<Project> getProjectsByWorkspace(const QUuid& workspaceId) const;

    /**
     * @brief Updates an existing project record.
     * @param proj Project with updated properties matching `proj.id`.
     * @return True on success.
     */
    bool updateProject(const Project& proj);

    /**
     * @brief Deletes a project by ID (cascading to tasks and project notes).
     * @param id Project UUID.
     */
    bool deleteProject(const QUuid& id);

    // =========================================================================
    // Task & Subtask Operations
    // =========================================================================

    /**
     * @brief Adds a task to the database.
     * @param task Task domain object.
     * @return Task UUID, or null QUuid on failure.
     */
    QUuid addTask(const Task& task);

    /**
     * @brief Fetches a single task along with its nested subtasks.
     * @param id Task UUID.
     */
    std::optional<Task> getTask(const QUuid& id) const;

    /**
     * @brief Retrieves all tasks belonging to a workspace.
     * @param workspaceId Workspace UUID.
     */
    QList<Task> getTasksByWorkspace(const QUuid& workspaceId) const;

    /**
     * @brief Retrieves all tasks assigned to a specific project.
     * @param projectId Project UUID.
     */
    QList<Task> getTasksByProject(const QUuid& projectId) const;

    /**
     * @brief Updates an existing task record.
     * @param task Task with modified fields.
     */
    bool updateTask(const Task& task);

    /**
     * @brief Deletes a task by its ID (cascades to child subtasks).
     * @param id Task UUID.
     */
    bool deleteTask(const QUuid& id);

    /**
     * @brief Adds a child subtask linked to a parent task.
     * @param subtask Subtask model.
     * @return Subtask UUID, or null QUuid on failure.
     */
    QUuid addSubtask(const SubTask& subtask);

    /**
     * @brief Retrieves all subtasks for a parent task.
     * @param taskId Parent task UUID.
     */
    QList<SubTask> getSubtasks(const QUuid& taskId) const;

    /**
     * @brief Updates a subtask's title or completion status.
     * @param subtask Subtask model with updated fields.
     */
    bool updateSubtask(const SubTask& subtask);

    /**
     * @brief Deletes a subtask by ID.
     * @param id Subtask UUID.
     */
    bool deleteSubtask(const QUuid& id);

    // =========================================================================
    // Note Operations
    // =========================================================================

    /**
     * @brief Adds a markdown note metadata record to the database.
     * @param note Note domain model.
     * @return Note UUID, or null QUuid on failure.
     */
    QUuid addNote(const Note& note);

    /**
     * @brief Retrieves a note by its unique identifier.
     * @param id Note UUID.
     */
    std::optional<Note> getNote(const QUuid& id) const;

    /**
     * @brief Returns all notes in a given workspace.
     * @param workspaceId Workspace UUID.
     */
    QList<Note> getNotesByWorkspace(const QUuid& workspaceId) const;

    /**
     * @brief Returns all notes assigned to a project.
     * @param projectId Project UUID.
     */
    QList<Note> getNotesByProject(const QUuid& projectId) const;

    /**
     * @brief Updates note properties (title, pin, archive, preview).
     * @param note Note domain object.
     */
    bool updateNote(const Note& note);

    /**
     * @brief Deletes a note record by ID.
     * @param id Note UUID.
     */
    bool deleteNote(const QUuid& id);

    // =========================================================================
    // Attachment Operations
    // =========================================================================

    /**
     * @brief Inserts an attachment metadata record.
     * @param att FileAttachment domain model.
     * @return Attachment UUID, or null QUuid on failure.
     */
    QUuid addAttachment(const FileAttachment& att);

    /**
     * @brief Retrieves attachment metadata by ID.
     * @param id Attachment UUID.
     */
    std::optional<FileAttachment> getAttachment(const QUuid& id) const;

    /**
     * @brief Retrieves attachments attached to a specific entity type and ID.
     * @param entityType Linked entity type enum (Workspace, Project, Note, Task).
     * @param entityId Target entity UUID.
     */
    QList<FileAttachment> getAttachmentsForEntity(AttachmentEntityType entityType, const QUuid& entityId) const;

    /**
     * @brief Returns all attachments within a given workspace.
     * @param workspaceId Workspace UUID.
     */
    QList<FileAttachment> getAttachmentsByWorkspace(const QUuid& workspaceId) const;

    /**
     * @brief Returns all attachments associated with a project.
     * @param projectId Project UUID.
     */
    QList<FileAttachment> getAttachmentsByProject(const QUuid& projectId) const;

    /**
     * @brief Returns all attachments associated with a note.
     * @param noteId Note UUID.
     */
    QList<FileAttachment> getAttachmentsByNote(const QUuid& noteId) const;

    /**
     * @brief Updates an attachment metadata record.
     * @param att FileAttachment domain model.
     */
    bool updateAttachment(const FileAttachment& att);

    /**
     * @brief Deletes an attachment metadata record by ID.
     * @param id Attachment UUID.
     */
    bool deleteAttachment(const QUuid& id);

private:
    QString m_connectionName;
    mutable QString m_lastError;

    QSqlDatabase db() const;
    bool runMigrations();
    int currentSchemaVersion() const;
    bool setSchemaVersion(int version);

    static QString uuidToDb(const QUuid& id) {
        return id.isNull() ? QString() : id.toString(QUuid::WithoutBraces);
    }
    static QUuid dbToUuid(const QString& str) {
        return str.isEmpty() ? QUuid() : QUuid::fromString(str);
    }
};

#endif //TASKHELPER_DATABASEMANAGER_H

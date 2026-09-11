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

class DatabaseManager
{
public:
    explicit DatabaseManager(const QString& connectionName = "flow_db_connection");
    ~DatabaseManager();

    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    bool open(const QString& dbPath);
    void close();
    bool isOpen() const;
    bool initializeSchema();
    QString lastError() const;

    bool beginTransaction();
    bool commitTransaction();
    bool rollbackTransaction();

    // Workspaces
    QUuid addWorkspace(const Workspace& ws);
    std::optional<Workspace> getWorkspace(const QUuid& id) const;
    QList<Workspace> getAllWorkspaces(bool includeArchived = false) const;
    bool updateWorkspace(const Workspace& ws);
    bool deleteWorkspace(const QUuid& id);

    // Projects
    QUuid addProject(const Project& proj);
    std::optional<Project> getProject(const QUuid& id) const;
    QList<Project> getProjectsByWorkspace(const QUuid& workspaceId) const;
    bool updateProject(const Project& proj);
    bool deleteProject(const QUuid& id);

    // Tasks & Subtasks
    QUuid addTask(const Task& task);
    std::optional<Task> getTask(const QUuid& id) const;
    QList<Task> getTasksByWorkspace(const QUuid& workspaceId) const;
    QList<Task> getTasksByProject(const QUuid& projectId) const;
    bool updateTask(const Task& task);
    bool deleteTask(const QUuid& id);

    QUuid addSubtask(const SubTask& subtask);
    QList<SubTask> getSubtasks(const QUuid& taskId) const;
    bool updateSubtask(const SubTask& subtask);
    bool deleteSubtask(const QUuid& id);

    // Notes
    QUuid addNote(const Note& note);
    std::optional<Note> getNote(const QUuid& id) const;
    QList<Note> getNotesByWorkspace(const QUuid& workspaceId) const;
    QList<Note> getNotesByProject(const QUuid& projectId) const;
    bool updateNote(const Note& note);
    bool deleteNote(const QUuid& id);

    // Attachments
    QUuid addAttachment(const FileAttachment& att);
    std::optional<FileAttachment> getAttachment(const QUuid& id) const;
    QList<FileAttachment> getAttachmentsForEntity(AttachmentEntityType entityType, const QUuid& entityId) const;
    QList<FileAttachment> getAttachmentsByWorkspace(const QUuid& workspaceId) const;
    QList<FileAttachment> getAttachmentsByProject(const QUuid& projectId) const;
    QList<FileAttachment> getAttachmentsByNote(const QUuid& noteId) const;
    bool updateAttachment(const FileAttachment& att);
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

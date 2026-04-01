//
// Created by DevAccount on 28/01/2026.
//

#ifndef TASKHELPER_WORKSPACEREPOSITORY_H
#define TASKHELPER_WORKSPACEREPOSITORY_H
#pragma once
#include <QObject>
#include "helpers/Workspace.h"
#include "Structure/Project.h"
#include "Structure/Task.h"
#include "Structure/Note.h"
#include "Structure/FileAttachment.h"



class WorkspaceRepository : public QObject
{
    Q_OBJECT

public:
    explicit WorkspaceRepository(QObject* parent = nullptr);

    // Workspace Management
    [[nodiscard]] QList<Workspace> workspaces() const;
    [[nodiscard]] Workspace getWorkspaceById(const QUuid& id) const;
    QUuid createWorkspace(const QString& name, const QString& type = "custom", const QString& description = "");
    void updateWorkspace(const Workspace& ws);
    void deleteWorkspace(const QUuid& id);

    // Project Management
    [[nodiscard]] QList<Project> getProjectsByWorkspace(const QUuid& workspaceId) const;
    [[nodiscard]] Project getProjectById(const QUuid& id) const;
    QUuid createProject(const Project& project);
    void updateProject(const Project& project);
    void deleteProject(const QUuid& id);

    // Task Management
    [[nodiscard]] QList<Task> getTasksByWorkspace(const QUuid& workspaceId) const;
    [[nodiscard]] QList<Task> getTasksByProject(const QUuid& projectId) const;
    [[nodiscard]] Task getTaskById(const QUuid& id) const;
    QUuid createTask(const Task& task);
    void updateTask(const Task& task);
    void deleteTask(const QUuid& id);

    // Note Management
    [[nodiscard]] QList<Note> getNotesByWorkspace(const QUuid& workspaceId) const;
    [[nodiscard]] QList<Note> getNotesByProject(const QUuid& projectId) const;
    [[nodiscard]] Note getNoteById(const QUuid& id) const;
    QUuid createNote(const Note& note);
    void updateNote(const Note& note);
    void deleteNote(const QUuid& id);

    // FileAttachment Management
    [[nodiscard]] QList<FileAttachment> getAttachmentsByWorkspace(const QUuid& workspaceId) const;
    [[nodiscard]] QList<FileAttachment> getAttachmentsByProject(const QUuid& projectId) const;
    [[nodiscard]] QList<FileAttachment> getAttachmentsByNote(const QUuid& noteId) const;
    [[nodiscard]] FileAttachment getAttachmentById(const QUuid& id) const;
    QUuid createAttachment(const FileAttachment& attachment);
    void deleteAttachment(const QUuid& id);

signals:
    void workspaceAdded(const Workspace& ws);
    void workspaceUpdated(const Workspace& ws);
    void projectAdded(const Project& project);
    void projectUpdated(const Project& project);
    void projectDeleted(const QUuid& projectId);
    void taskAdded(const Task& task);
    void taskUpdated(const Task& task);
    void taskDeleted(const QUuid& taskId);
    void noteAdded(const Note& note);
    void noteUpdated(const Note& note);
    void noteDeleted(const QUuid& noteId);
    void attachmentAdded(const FileAttachment& attachment);
    void attachmentDeleted(const QUuid& attachmentId);

private:
    QList<Workspace> workspaces_;
    QList<Project> projects_;
    QList<Task> tasks_;
    QList<Note> notes_;
    QList<FileAttachment> attachments_;

    // Persistence
    void saveWorkspaces();
    void loadWorkspaces();
    void saveProjects();
    void loadProjects();
    void saveTasks();
    void loadTasks();
    void saveNotes();
    void loadNotes();
    void saveAttachments();
    void loadAttachments();
    void ensureProjectStructure();
    [[nodiscard]] QUuid defaultProjectForWorkspace(const QUuid& workspaceId) const;

    static QString uuidKey (const QUuid& id) { return id.toString(QUuid::WithoutBraces);}

    [[nodiscard]] QString dataRootPath() const;

    [[nodiscard]] QString projectPath(const QUuid& workspaceId, const QUuid& projectId) const;

    [[nodiscard]] bool ensureProjectDir(const QUuid& workspaceId, const QUuid& projectId) const;

    // File-based persistence helpers
    void saveNoteToFile(const Note& note) const;
    void removeNoteFromFile(const Note& note) const;
    QString storeAttachmentFile(const FileAttachment& attachment) const;
    void removeAttachmentFile(const FileAttachment& attachment) const;


};

#endif //TASKHELPER_WORKSPACEREPOSITORY_H
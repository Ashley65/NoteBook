//
// Created by DevAccount on 28/01/2026.
//

#ifndef TASKHELPER_WORKSPACEREPOSITORY_H
#define TASKHELPER_WORKSPACEREPOSITORY_H
#pragma once
#include <QObject>
#include "helpers/Workspace.h"
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
    [[nodiscard]] Workspace getWorkspaceById(const QString& id) const;
    QString createWorkspace(const QString& name, const QString& type = "custom", const QString& description = "");
    void updateWorkspace(const Workspace& ws);
    void deleteWorkspace(const QString& id);

    // Task Management
    [[nodiscard]] QList<Task> getTasksByWorkspace(const QString& workspaceId) const;
    [[nodiscard]] Task getTaskById(const QString& id) const;
    QString createTask(const Task& task);
    void updateTask(const Task& task);
    void deleteTask(const QString& id);

    // Note Management
    [[nodiscard]] QList<Note> getNotesByWorkspace(const QString& workspaceId) const;
    [[nodiscard]] Note getNoteById(const QString& id) const;
    QString createNote(const Note& note);
    void updateNote(const Note& note);
    void deleteNote(const QString& id);

    // FileAttachment Management
    [[nodiscard]] QList<FileAttachment> getAttachmentsByWorkspace(const QString& workspaceId) const;
    [[nodiscard]] QList<FileAttachment> getAttachmentsByNote(const QString& noteId) const;
    [[nodiscard]] FileAttachment getAttachmentById(const QString& id) const;
    QString createAttachment(const FileAttachment& attachment);
    void deleteAttachment(const QString& id);

signals:
    void workspaceAdded(const Workspace& ws);
    void workspaceUpdated(const Workspace& ws);
    void taskAdded(const Task& task);
    void taskUpdated(const Task& task);
    void taskDeleted(const QString& taskId);
    void noteAdded(const Note& note);
    void noteUpdated(const Note& note);
    void noteDeleted(const QString& noteId);
    void attachmentAdded(const FileAttachment& attachment);
    void attachmentDeleted(const QString& attachmentId);

private:
    QList<Workspace> workspaces_;
    QList<Task> tasks_;
    QList<Note> notes_;
    QList<FileAttachment> attachments_;

    // Persistence
    void saveWorkspaces();
    void loadWorkspaces();
    void saveTasks();
    void loadTasks();
    void saveNotes();
    void loadNotes();
    void saveAttachments();
    void loadAttachments();


};

#endif //TASKHELPER_WORKSPACEREPOSITORY_H
//
// Created by DevAccount on 28/01/2026.
//

#ifndef TASKHELPER_WORKSPACEREPOSITORY_H
#define TASKHELPER_WORKSPACEREPOSITORY_H
#pragma once
#include <QObject>
#include "helpers/Workspace.h"
#include "helpers/Task.h"



class WorkspaceRepository : public QObject
{
    Q_OBJECT

public:
    explicit WorkspaceRepository(QObject* parent = nullptr);

    // Workspace Management
    [[nodiscard]] QList<Workspace> workspaces() const;
    [[nodiscard]] Workspace getWorkspaceById(const QString& id) const;
    QString createWorkspace(const QString& name);
    void deleteWorkspace(const QString& id);

    // Task Management
    [[nodiscard]] QList<Task> getTasksByWorkspace(const QString& workspaceId) const;
    [[nodiscard]] Task getTaskById(const QString& id) const;
    QString createTask(const Task& task);
    void updateTask(const Task& task);
    void deleteTask(const QString& id);

signals:
    void workspaceAdded(const Workspace& ws);
    void taskAdded(const Task& task);
    void taskUpdated(const Task& task);
    void taskDeleted(const QString& taskId);

private:
    QList<Workspace> workspaces_;
    QList<Task> tasks_;

    // Persistence
    void saveWorkspaces();
    void loadWorkspaces();
    void saveTasks();
    void loadTasks();


};

#endif //TASKHELPER_WORKSPACEREPOSITORY_H
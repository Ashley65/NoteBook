//
// Created by DevAccount on 11/04/2026.
//

#ifndef TASKHELPER_TASKMANAGER_H
#define TASKHELPER_TASKMANAGER_H
#pragma once
#include <QObject>
#include <QList>
#include "Data/workspace/WorkspaceRepository.h"
#include "Data/workspace/Structure/Task.h"

struct TaskCreateRequest
{
    QString title;
    QString description;
    QUuid workspaceId;
    QUuid projectId;
    TaskPriority priority = TaskPriority::Medium;
    QDateTime dueDate;
};


class TaskManager : public QObject
{
    Q_OBJECT
public:
    explicit TaskManager(WorkspaceRepository* repo, QObject* parent = nullptr);

    QList<Task> todayTasks(const QUuid& workspaceId) const;
    QList<Task> overdueTasks(const QUuid& workspaceId) const;
    QList<Task> allTasks(const QUuid& workspaceId) const;
    Task getTaskById(const QUuid& taskId) const;

    QUuid createTask(const TaskCreateRequest& request);
    void setCompleted(const QUuid& taskId, bool completed);
    void updateTask(const Task& task);
    void deleteTask(const QUuid& taskId);

signals:
    void tasksChanged(const QUuid& workspaceId);

private:
    WorkspaceRepository* m_repo {nullptr};

};




#endif //TASKHELPER_TASKMANAGER_H

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
    QUuid id;
    QUuid workspaceId;
    QUuid projectId;
    QString title;
    QString description;
    TaskStatus status {TaskStatus::Pending};
    TaskPriority priority {TaskPriority::Medium};
    QDateTime createdAt;
    QDateTime dueDate;
    QDateTime completedAt;
    // Subtasks collection
    QList<SubTask> subtasks;

    int totalSubTask() const { return subtasks.size(); }
    int completedSubTasks() const
    {
        int count = 0;
        for (const auto& subTask : subtasks)
        {
            if (subTask.isCompleted) {
                count++;
            }
        }
        return count;
    }
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
    void setTaskStatus(const QUuid& taskId, TaskStatus status);
    void updateTask(const Task& task);
    void deleteTask(const QUuid& taskId);

    // Subtask actions
    void addSubtask(const QUuid& taskId, const QString& title);
    void toggleSubtask(const QUuid& taskId, const QUuid& subtaskId, bool completed);
    void deleteSubtask(const QUuid& taskId, const QUuid& subtaskId);
    void updateSubtask(const QUuid& taskId, const QUuid& subtaskId, const QString& title, bool completed);

signals:
    void tasksChanged(const QUuid& workspaceId);

private:
    WorkspaceRepository* m_repo {nullptr};
};




#endif //TASKHELPER_TASKMANAGER_H

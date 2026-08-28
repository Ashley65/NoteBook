//
// Created by DevAccount on 16/02/2026.
//

#ifndef TASKHELPER_TASK_H
#define TASKHELPER_TASK_H
#pragma once

#include <QString>
#include <QDateTime>
#include <QUuid>
#include <QList>

enum class TaskStatus
{
    Pending,
    InProgress,
    Completed,
    Archived
};

enum class TaskPriority
{
    Low,
    Medium,
    High,
    Critical
};

struct SubTask
{
    QUuid id;
    QUuid taskId;
    QString title;
    bool isCompleted {false};
};

struct Task
{
    QUuid id;
    QUuid workspaceId;
    QUuid projectId;

    QString title;
    QString description;
    TaskStatus status;

    TaskPriority priority;
    QDateTime createdAt;
    QDateTime dueDate;
    QDateTime completedAt;

    QList<SubTask> subtasks;

    int totalSubTasks() const { return subtasks.size(); }
    int completedSubTasks() const
    {
        int count = 0;
        for (const auto& st : subtasks) {
            if (st.isCompleted) {
                count++;
            }
        }
        return count;
    }
};

#endif //TASKHELPER_TASK_H


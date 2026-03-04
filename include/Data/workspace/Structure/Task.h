//
// Created by DevAccount on 16/02/2026.
//

#ifndef TASKHELPER_TASK_H
#define TASKHELPER_TASK_H
#pragma once

#include <QString>
#include <QDateTime>
#include <QUuid>

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

struct Task
{
    QString id;
    QString workspaceId;
    QString title;
    QString description;
    TaskStatus status;
    TaskPriority priority;
    QDateTime createdAt;
    QDateTime dueDate;
    QDateTime completedAt;
};

#endif //TASKHELPER_TASK_H


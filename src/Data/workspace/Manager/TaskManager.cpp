//
// Created by DevAccount on 11/04/2026.
//

#include "Data/workspace/Manager/TaskManager.h"

#include <algorithm>

TaskManager::TaskManager(WorkspaceRepository* repo, QObject* parent)
    : QObject(parent), m_repo(repo)
{
    if (!m_repo)
    {
        return;
    }

    connect(m_repo, &WorkspaceRepository::taskAdded, this, [this](const Task& task)
    {
        emit tasksChanged(task.workspaceId);
    });

    connect(m_repo, &WorkspaceRepository::taskUpdated, this, [this](const Task& task)
    {
        emit tasksChanged(task.workspaceId);

    });

    connect(m_repo, &WorkspaceRepository::taskDeleted, this, [this](const QUuid&) {
        emit tasksChanged(QUuid{});
    });

}

QList<Task> TaskManager::todayTasks(const QUuid& workspaceId) const
{
    QList<Task> out;
    const QList<Task> tasks = allTasks(workspaceId);
    const QDate today = QDate::currentDate();

    for (const Task& t : tasks)
    {
        if (t.status == TaskStatus::Completed || t.status == TaskStatus::Archived)
        {
            continue;
        }
        if (t.dueDate.isValid() && t.dueDate.date() == today)
        {
            out.append(t);
        }
    }
    return out;
}

QList<Task> TaskManager::overdueTasks(const QUuid& workspaceId) const
{
    QList<Task> out;
    const QList<Task> tasks = allTasks(workspaceId);
    const QDateTime now = QDateTime::currentDateTime();

    for (const Task& t : tasks)
    {
        if (t.status == TaskStatus::Completed || t.status == TaskStatus::Archived)
        {
            continue;
        }
        if (t.dueDate.isValid() && t.dueDate < now)
        {
            out.append(t);
        }

    }
    return out;

}

QList<Task> TaskManager::allTasks(const QUuid& workspaceId) const
{
    if (!m_repo || workspaceId.isNull())
    {
        return {};
    }

    QList<Task> tasks = m_repo->getTasksByWorkspace(workspaceId);

    std::sort(tasks.begin(), tasks.end(), [](const Task& a, const Task& b)
    {
        const bool aHasDue = a.dueDate.isValid();
        const bool bHasDue = b.dueDate.isValid();

        if (aHasDue && bHasDue) return a.dueDate < b.dueDate;
        if (aHasDue != bHasDue) return aHasDue; // Tasks with due dates come first
        return a.createdAt < b.createdAt; // Otherwise sort by creation date

    });
    return tasks;
}

Task TaskManager::getTaskById(const QUuid& taskId) const
{
    if (!m_repo || taskId.isNull())
    {
        return {};
    }
    return m_repo->getTaskById(taskId);
}

QUuid TaskManager::createTask(const TaskCreateRequest& request)
{

    if (!m_repo || request.workspaceId.isNull() || request.title.trimmed().isEmpty()) {
        return {};
    }

    Task task;
    task.workspaceId = request.workspaceId;
    task.projectId = request.projectId; // repo assigns default if null
    task.title = request.title.trimmed();
    task.description = request.description;
    task.status = TaskStatus::Pending;
    task.priority = request.priority;
    task.dueDate = request.dueDate;

    const QUuid id = m_repo->createTask(task);
    if (!id.isNull()) {
        emit tasksChanged(request.workspaceId);
    }
    return id;
}

void TaskManager::setCompleted(const QUuid& taskId, bool completed)
{
    if (!m_repo || taskId.isNull())
    {
        return;
    }


    Task task = m_repo->getTaskById(taskId);
    if (task.id.isNull())
    {
        return;
    }

    const TaskStatus targetStatus = completed ? TaskStatus::Completed : TaskStatus::Pending;
    if (task.status == targetStatus)
    {
        return;
    }

    task.status = targetStatus;
    m_repo->updateTask(task);
    emit tasksChanged(task.workspaceId);
}

void TaskManager::updateTask(const Task& task)
{

    if (!m_repo || task.id.isNull() || task.workspaceId.isNull() || task.title.trimmed().isEmpty()) {
        return;
    }

    Task updated = task;
    updated.title = updated.title.trimmed();

    m_repo->updateTask(updated);
    emit tasksChanged(updated.workspaceId);
}

void TaskManager::deleteTask(const QUuid& taskId)
{
    if (!m_repo || taskId.isNull())
    {
        return;
    }

    const Task existing = m_repo->getTaskById(taskId);
    if (existing.id.isNull())
    {
        return;
    }

    m_repo->deleteTask(taskId);
    emit tasksChanged(existing.workspaceId);
}

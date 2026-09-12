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

/**
 * @file TaskManager.h
 * @brief High-level business logic manager for tasks, deadlines, and subtasks.
 */

/**
 * @struct TaskCreateRequest
 * @brief Parameters for creating a new task, including optional subtasks and metadata.
 */
struct TaskCreateRequest
{
    QUuid id;                           ///< Optional custom ID; generated if null.
    QUuid workspaceId;                  ///< Target workspace UUID.
    QUuid projectId;                    ///< Target project UUID.
    QString title;                      ///< Task headline.
    QString description;                ///< Extended description or markdown notes.
    TaskStatus status {TaskStatus::Pending};   ///< Initial status (Pending, InProgress, Completed).
    TaskPriority priority {TaskPriority::Medium}; ///< Priority (Low, Medium, High, Critical).
    QDateTime createdAt;                ///< Creation timestamp.
    QDateTime dueDate;                  ///< Target completion deadline.
    QDateTime completedAt;              ///< Timestamp when marked completed.
    QList<SubTask> subtasks;            ///< Optional initial list of subtasks.

    /**
     * @brief Returns total count of attached subtasks.
     */
    int totalSubTask() const { return subtasks.size(); }

    /**
     * @brief Computes how many attached subtasks are currently completed.
     */
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

/**
 * @class TaskManager
 * @brief Domain manager for task lifecycles, deadline filtering, and subtask mutations.
 *
 * TaskManager operates directly on the WorkspaceRepository to provide task-related
 * queries (such as tasks due today, overdue tasks, or all tasks in a workspace) and
 * high-level mutation APIs like toggling completion status, transitioning Kanban states,
 * and managing checklist subtasks.
 */
class TaskManager : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief Constructs a TaskManager wired to the specified WorkspaceRepository.
     * @param repo Pointer to the shared WorkspaceRepository instance.
     * @param parent Optional parent QObject for ownership hierarchy.
     */
    explicit TaskManager(WorkspaceRepository* repo, QObject* parent = nullptr);

    /**
     * @brief Filters active tasks due on today's calendar date.
     * @param workspaceId Target workspace UUID.
     */
    QList<Task> todayTasks(const QUuid& workspaceId) const;

    /**
     * @brief Filters active tasks whose due date is in the past.
     * @param workspaceId Target workspace UUID.
     */
    QList<Task> overdueTasks(const QUuid& workspaceId) const;

    /**
     * @brief Returns all tasks within a given workspace.
     * @param workspaceId Target workspace UUID.
     */
    QList<Task> allTasks(const QUuid& workspaceId) const;

    /**
     * @brief Retrieves a task by its unique ID.
     * @param taskId Task UUID.
     */
    Task getTaskById(const QUuid& taskId) const;

    /**
     * @brief Creates a task from a request object and persists it to the repository.
     * @param request Populated TaskCreateRequest descriptor.
     * @return Generated Task UUID.
     */
    QUuid createTask(const TaskCreateRequest& request);

    /**
     * @brief Sets whether a task is completed, updating its status and completedAt timestamp.
     * @param taskId Task UUID.
     * @param completed True to mark Completed; false to reset to Pending.
     */
    void setCompleted(const QUuid& taskId, bool completed);

    /**
     * @brief Transitions a task to an explicit Kanban status state.
     * @param taskId Task UUID.
     * @param status New TaskStatus (Pending, InProgress, Completed, Archived).
     */
    void setTaskStatus(const QUuid& taskId, TaskStatus status);

    /**
     * @brief Updates an existing task's attributes.
     * @param task Task domain object with modified values.
     */
    void updateTask(const Task& task);

    /**
     * @brief Removes a task and its subtasks from the repository.
     * @param taskId Identifier of the task to delete.
     */
    void deleteTask(const QUuid& taskId);

    // =========================================================================
    // Subtask Actions
    // =========================================================================

    /**
     * @brief Adds a new subtask to an existing task.
     * @param taskId Parent task UUID.
     * @param title Subtask description/title.
     */
    void addSubtask(const QUuid& taskId, const QString& title);

    /**
     * @brief Toggles completion status of an individual subtask.
     * @param taskId Parent task UUID.
     * @param subtaskId Subtask UUID.
     * @param completed True if finished, false if open.
     */
    void toggleSubtask(const QUuid& taskId, const QUuid& subtaskId, bool completed);

    /**
     * @brief Deletes a subtask from a task.
     * @param taskId Parent task UUID.
     * @param subtaskId Subtask UUID to remove.
     */
    void deleteSubtask(const QUuid& taskId, const QUuid& subtaskId);

    /**
     * @brief Updates an individual subtask's title and completion state.
     * @param taskId Parent task UUID.
     * @param subtaskId Subtask UUID.
     * @param title Updated title text.
     * @param completed Updated completion status.
     */
    void updateSubtask(const QUuid& taskId, const QUuid& subtaskId, const QString& title, bool completed);

signals:
    /**
     * @brief Emitted whenever tasks within the specified workspace are created, updated, or deleted.
     * @param workspaceId Identifier of the affected workspace.
     */
    void tasksChanged(const QUuid& workspaceId);

private:
    WorkspaceRepository* m_repo {nullptr};
};

#endif //TASKHELPER_TASKMANAGER_H

//
// Created by DevAccount on 26/08/2026.
//

#ifndef TASKHELPER_WSTASKBOARDPAGE_H
#define TASKHELPER_WSTASKBOARDPAGE_H
#pragma once

#include <QQuickWidget>
#include <QVariantList>
#include <QDateTime>
#include "../IWorkspaceView.h"
#include "helpers/Workspace.h"
#include "Data/workspace/Manager/TaskManager.h"
#include "Data/workspace/WorkspaceRepository.h"
class wsTaskBoardPage : public IWorkspaceView
{
    Q_OBJECT
    // Workspace & Project Context
    Q_PROPERTY(QString workspaceName READ workspaceName NOTIFY workspaceNameChanged)
    Q_PROPERTY(QString activeProjectId READ activeProjectId WRITE setActiveProjectId NOTIFY activeProjectChanged)
    Q_PROPERTY(QString activeProjectName READ activeProjectName NOTIFY activeProjectChanged)
    // Kanban Columns (Pre-grouped by Status)
    Q_PROPERTY(QVariantList todoTasks READ todoTasks NOTIFY tasksChanged)
    Q_PROPERTY(QVariantList inProgressTasks READ inProgressTasks NOTIFY tasksChanged)
    Q_PROPERTY(QVariantList completedTasks READ completedTasks NOTIFY tasksChanged)
    // Column Counters (for badges: To Do (1), In Progress (0), Completed (1))
    Q_PROPERTY(int todoCount READ todoCount NOTIFY tasksChanged)
    Q_PROPERTY(int inProgressCount READ inProgressCount NOTIFY tasksChanged)
    Q_PROPERTY(int completedCount READ completedCount NOTIFY tasksChanged)
    // Search, Filter & Sort Controls
    Q_PROPERTY(QString searchQuery READ searchQuery WRITE setSearchQuery NOTIFY filterChanged)
    Q_PROPERTY(int priorityFilter READ priorityFilter WRITE setPriorityFilter NOTIFY filterChanged)
    Q_PROPERTY(int sortMode READ sortMode WRITE setSortMode NOTIFY sortModeChanged)
public:
    explicit wsTaskBoardPage(const Workspace& ws, WorkspaceRepository* repo, QWidget* parent = nullptr);
    // IWorkspaceView Overrides
    void refresh() override;
    void updateWorkspace(const Workspace& ws) override;
    void setActiveProject(const Project& project) override;
    // Property Getters
    QString workspaceName() const;
    QString activeProjectId() const;
    QString activeProjectName() const;
    QVariantList todoTasks() const;
    QVariantList inProgressTasks() const;
    QVariantList completedTasks() const;
    int todoCount() const;
    int inProgressCount() const;
    int completedCount() const;
    QString searchQuery() const;
    int priorityFilter() const;
    int sortMode() const;
    // Property Setters
    void setActiveProjectId(const QString& id);
    void setSearchQuery(const QString& query);
    void setPriorityFilter(int priority);
    void setSortMode(int mode);
    // QML Task Actions
    Q_INVOKABLE void createNewTask(const QString& title, const QString& description = QString(), int priority = 1, const QDateTime& dueDate = QDateTime());
    Q_INVOKABLE void updateTaskStatus(const QString& taskId, int newStatus);
    Q_INVOKABLE void toggleTaskCompletion(const QString& taskId, bool isCompleted);
    Q_INVOKABLE void deleteTask(const QString& taskId);
    Q_INVOKABLE void openTaskDetails(const QString& taskId);
    Q_INVOKABLE void openLinkedNote(const QString& noteId);
    // QML Subtask Actions
    Q_INVOKABLE void addSubtask(const QString& taskId, const QString& title);
    Q_INVOKABLE void toggleSubtask(const QString& taskId, const QString& subtaskId, bool completed);
    Q_INVOKABLE void deleteSubtask(const QString& taskId, const QString& subtaskId);
    Q_INVOKABLE void updateSubtask(const QString& taskId, const QString& subtaskId, const QString& title, bool completed);
signals:
    void workspaceNameChanged();
    void activeProjectChanged();
    void tasksChanged();
    void filterChanged();
    void sortModeChanged();
    void noteOpenRequested(const QString& noteId);
private:
    Workspace m_workspace;
    Project m_activeProject;
    QUuid workspaceId_;
    WorkspaceRepository* m_repo {nullptr};
    TaskManager* m_taskManager {nullptr};
    QQuickWidget* m_TaskQuickView {nullptr};
    // Cached column data for QML
    QVariantList m_todoTasks;
    QVariantList m_inProgressTasks;
    QVariantList m_completedTasks;
    // Filter states
    QString m_searchQuery;
    int m_priorityFilter {-1}; // -1 = All priorities
    int m_sortMode {0};        // 0 = Due Date Earliest, 1 = Priority, etc.
    void setupUi();
    void populateData();
};
#endif //TASKHELPER_WSTASKBOARDPAGE_H

//
// Created by DevAccount on 10/04/2026.
//

#ifndef TASKHELPER_WSHOMEPAGE_H
#define TASKHELPER_WSHOMEPAGE_H
#pragma once
#include <QQuickWidget>

#include "../IWorkspaceView.h"
#include "helpers/Workspace.h"
#include "Data/workspace/Manager/NoteManager.h"
#include "Data/workspace/Manager/TaskManager.h"
#include <QVariantList>


class wsHomePage : public IWorkspaceView
{
    Q_OBJECT

    Q_PROPERTY(QString workspaceName READ workspaceName NOTIFY workspaceNameChanged)

    // Create a property list to populate the three-columns layout of the homepage with some basic info about the workspace and some quick actions
    Q_PROPERTY(QVariantList recentNotes READ recentNotes NOTIFY recentNotesChanged)
    Q_PROPERTY(QVariantList todayTasks READ todayTasks NOTIFY todayTasksChanged)
    Q_PROPERTY(QVariantList overdueTasks READ overdueTasks NOTIFY overdueTasksChanged)
    Q_PROPERTY(QVariantList projectProgress READ projectProgress NOTIFY projectProgressChanged)
    Q_PROPERTY(QString activeProjectName READ activeProjectName NOTIFY activeProjectChanged)

public:
    explicit wsHomePage(const Workspace& ws ,WorkspaceRepository* repo ,QWidget* parent = nullptr);
    void refresh() override;
    void updateWorkspace(const Workspace& ws) override;
    void setActiveProject(const Project& project) override;

    // Top Action Bar Buttons
    Q_INVOKABLE void createNewTask(const QString& title);
    Q_INVOKABLE void createNewNote(const QString& title);
    Q_INVOKABLE void uploadFile(const QString& filePath);
    Q_INVOKABLE void requestUploadFile();

    // Task Interactions
    Q_INVOKABLE void toggleTaskCompletion(const QString& taskId, bool isCompleted);
    Q_INVOKABLE void openNote(const QString& noteId);

    QString workspaceName() const;
    QString activeProjectName() const;
    QVariantList recentNotes() const;
    QVariantList todayTasks() const;
    QVariantList overdueTasks() const;
    QVariantList projectProgress() const;


signals:
    void workspaceNameChanged();
    void recentNotesChanged();
    void todayTasksChanged();
    void overdueTasksChanged();
    void projectProgressChanged();
    void activeProjectChanged();
    void uploadMessage(const QString& message, bool isError);

private:
    Workspace m_workspace;
    Project m_activeProject;
    // Store the workspace ID for data fetching and updates
    QUuid workspaceId_;

    WorkspaceRepository* m_repo {nullptr};
    QVariantList m_recentNotes;
    QVariantList m_todayTasks;
    QVariantList m_overdueTasks;
    QVariantList m_projectProgress;


    void populateData();

    TaskManager* m_taskManager {nullptr};
    NoteManager* m_noteManager {nullptr};

    QQuickWidget*  m_homeQuickView {nullptr};

    void setupUi();
};


#endif //TASKHELPER_WSHOMEPAGE_H

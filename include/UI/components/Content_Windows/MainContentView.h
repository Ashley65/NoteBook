//
// Created by DevAccount on 28/01/2026.
//

#ifndef TASKHELPER_MAINCONTENTVIEW_H
#define TASKHELPER_MAINCONTENTVIEW_H
#pragma once
#include <QStackedWidget>

#include "helpers/Workspace.h"

#include <helpers/AppStateController.h>

#include <QHash>
#include <QUuid>
#include "IWorkspaceView.h"
#include "Data/workspace/WorkspaceRepository.h"

class MainContentView : public QStackedWidget
{
    Q_OBJECT
public:
    explicit MainContentView(WorkspaceRepository* repo, QWidget* parent = nullptr);
    void setActiveWorkspace(const Workspace& ws);
    void setActiveWorkspace(const AppContext& ctx);
    void setActiveProject(const Project& project);
    void discardView(const QUuid& contextId);

private:
    QHash<QUuid, IWorkspaceView*> views_;
    WorkspaceRepository* m_repo {nullptr};
    Project m_activeProject;
};
#endif //TASKHELPER_MAINCONTENTVIEW_H
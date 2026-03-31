//
// Created by DevAccount on 23/01/2026.
//

#ifndef TASKHELPER_WORKSPACEVIEW_H
#define TASKHELPER_WORKSPACEVIEW_H
#pragma once
#include <QWidget>
#include <QUuid>
#include "helpers/Workspace.h"

#include "IWorkspaceView.h"

class WorkspaceView : public IWorkspaceView
{
    Q_OBJECT
public:
    explicit WorkspaceView(const Workspace& ws, QWidget* parent = nullptr);

    // IWorkspaceView interface
    void refresh() override;
    void updateWorkspace(const Workspace& ws) override;

private:
    QUuid workspaceId_;
};
#endif //TASKHELPER_WORKSPACEVIEW_H
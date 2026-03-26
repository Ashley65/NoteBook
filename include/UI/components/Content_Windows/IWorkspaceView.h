//
// Created by DevAccount on 06/03/2026.
//

#ifndef TASKHELPER_IWORKSPACEVIEW_H
#define TASKHELPER_IWORKSPACEVIEW_H
#pragma once

#include <QWidget>
#include "helpers/Workspace.h"


class IWorkspaceView : public QWidget
{
    Q_OBJECT
public:
    explicit IWorkspaceView(const Workspace& ws, QWidget* parent = nullptr) : QWidget(parent) {}
    virtual ~IWorkspaceView() override = default;

    // API for interacting with the workspace view
    virtual void refresh() = 0; // Refresh the view with current workspace data#
    virtual void updateWorkspace(const Workspace& ws) = 0; // Update the view with new workspace data
};

#endif //TASKHELPER_IWORKSPACEVIEW_H
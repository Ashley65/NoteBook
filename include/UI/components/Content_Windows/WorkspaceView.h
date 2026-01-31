//
// Created by DevAccount on 23/01/2026.
//

#ifndef TASKHELPER_WORKSPACEVIEW_H
#define TASKHELPER_WORKSPACEVIEW_H
#pragma once
#include <QWidget>
#include "helpers/Workspace.h"

class WorkspaceView : public QWidget
{
    Q_OBJECT
public:
    explicit WorkspaceView(const Workspace& ws, QWidget* parent = nullptr);


private:
    QString workspaceId_;
};
#endif //TASKHELPER_WORKSPACEVIEW_H
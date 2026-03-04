//
// Created by DevAccount on 28/01/2026.
//

#ifndef TASKHELPER_MAINCONTENTVIEW_H
#define TASKHELPER_MAINCONTENTVIEW_H
#pragma once
#include <QStackedWidget>

#include "helpers/Workspace.h"

#include <helpers/AppStateController.h>

class WorkspaceView;

class MainContentView : public QStackedWidget
{
    Q_OBJECT
public:
    explicit MainContentView(QWidget* parent = nullptr);
    void setActiveWorkspace(const Workspace& ws);
    void setActiveWorkspace(const AppContext& ctx);

    // QML Interface



private:
    QHash<QString, WorkspaceView*> views_;
};
#endif //TASKHELPER_MAINCONTENTVIEW_H
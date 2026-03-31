//
// Created by DevAccount on 18/12/2025.
//
#ifndef TASKHELPER_PRIMARYACTIONSECTION_H
#define TASKHELPER_PRIMARYACTIONSECTION_H
#pragma once

#include <QWidget>
#include <QPushButton>
#include <QToolButton>
#include <QUuid>

#include "helpers/AppStateController.h"
#include "helpers/Workspace.h"
#include "helpers/WorkspaceCommand.h"


class WorkspaceContextSection : public QWidget
{
    Q_OBJECT
public:
    explicit WorkspaceContextSection(QWidget *parent = nullptr);
    void setActiveWorkspace(const Workspace& ws);
protected:
    // Listen for the "compact" property change from SideBar.cpp
    void changeEvent(QEvent *event) override;

signals:
    void workspaceChanged(const QUuid& workspaceId);
    void requestWorkspaceSwitch();
    void requestWorkspaceCreate();
    void requestWorkspaceSettings(const QUuid& workspaceId);



public slots:
    void setContext(const AppContext& ctx);

private slots:
    void onMenuActionTriggered(QAction *action);

private:
    QToolButton* mainBtn;
    QString currentWorkspace_ = "TEST workspace";
    QUuid currentWorkspaceID_;
    QString currentWorkspaceName_;
    void updateStyle(bool compact);
};

#endif //TASKHELPER_PRIMARYACTIONSECTION_H
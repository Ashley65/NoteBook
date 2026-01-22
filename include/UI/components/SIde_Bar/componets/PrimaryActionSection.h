//
// Created by DevAccount on 18/12/2025.
//
#ifndef TASKHELPER_PRIMARYACTIONSECTION_H
#define TASKHELPER_PRIMARYACTIONSECTION_H
#pragma once

#include <QWidget>
#include <QPushButton>
#include <QToolButton>

class WorkspaceContextSection : public QWidget
{
    Q_OBJECT
public:
    explicit WorkspaceContextSection(QWidget *parent = nullptr);

protected:
    // Listen for the "compact" property change from SideBar.cpp
    void changeEvent(QEvent *event) override;

signals:
    void workspaceChanged(const QString& workspaceId);
    void requestWorkspaceSwitch();
    void requestWorkspaceCreate();
    void triggered();
    void requestWorkspaceSettings(const QString& workspaceId);



private slots:
    void onMenuActionTriggered(QAction *action);

private:
    QToolButton* mainBtn;
    QString currentWorkspace_ = "TEST workspace";
    QString currentWorkspaceID_;
    QString currentWorkspaceName_;
    void updateStyle(bool compact);
};

#endif //TASKHELPER_PRIMARYACTIONSECTION_H
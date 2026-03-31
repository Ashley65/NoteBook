//
// Created by DevAccount on 29/01/2026.
//

#ifndef TASKHELPER_WORKSPACESWITCHDIALOG_H
#define TASKHELPER_WORKSPACESWITCHDIALOG_H
#pragma once
#include <QDialog>
#include <QListWidget>
#include <QUuid>

#include "helpers/Workspace.h"

class WorkspaceSwitchDialog : public QDialog
{
    Q_OBJECT
public:
    explicit WorkspaceSwitchDialog(
        const QList<Workspace>& workspaces,
        const QUuid& activeWorkspaceId,
        QWidget* parent = nullptr
    );

    [[nodiscard]] QUuid selectedWorkspaceId() const;

private:
    QListWidget* list_;
    QUuid selectedId_;
};


#endif //TASKHELPER_WORKSPACESWITCHDIALOG_H
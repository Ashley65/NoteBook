//
// Created by DevAccount on 29/01/2026.
//

#ifndef TASKHELPER_WORKSPACESWITCHDIALOG_H
#define TASKHELPER_WORKSPACESWITCHDIALOG_H
#pragma once
#include <QDialog>
#include <QListWidget>
#include <QString>

#include "helpers/Workspace.h"

class WorkspaceSwitchDialog : public QDialog
{
    Q_OBJECT
public:
    explicit WorkspaceSwitchDialog(
        const QList<Workspace>& workspaces,
        const QString& activeWorkspaceId,
        QWidget* parent = nullptr
    );

    [[nodiscard]] QString selectedWorkspaceId() const;

private:
    QListWidget* list_;
    QString selectedId_;
};


#endif //TASKHELPER_WORKSPACESWITCHDIALOG_H
//
// Created by DevAccount on 23/02/2026.
//

#ifndef TASKHELPER_WORKSPACEDELETEDIALOG_H
#define TASKHELPER_WORKSPACEDELETEDIALOG_H
#pragma once
#include <QDialog>
#include <QListWidget>
#include <QString>

#include "helpers/Workspace.h"

class WorkspaceDeleteDialog : public QDialog
{
    Q_OBJECT
public:
    explicit WorkspaceDeleteDialog(
        const QList<Workspace>& workspaces,
        QWidget* parent = nullptr
        );

    [[nodiscard]] QString selectedWorkspaceId() const;

private slots:

    void validateSelection(const QString& name);


private:
    QListWidget* list_;
    QString selectedId_;
};

#endif //TASKHELPER_WORKSPACEDELETEDIALOG_H
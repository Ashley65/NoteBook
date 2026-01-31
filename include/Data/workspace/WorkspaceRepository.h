//
// Created by DevAccount on 28/01/2026.
//

#ifndef TASKHELPER_WORKSPACEREPOSITORY_H
#define TASKHELPER_WORKSPACEREPOSITORY_H
#pragma once
#include <QObject>
#include "helpers/Workspace.h"



class WorkspaceRepository : public QObject
{
    Q_OBJECT

public:
    explicit WorkspaceRepository(QObject* parent = nullptr);
    [[nodiscard]] QList<Workspace> workspaces() const;

    [[nodiscard]] Workspace getWorkspaceById(const QString& id) const;

    QString createWorkspace(const QString& name);

    void deleteWorkspace(const QString& id);

signals:
    void workspaceAdded(const Workspace& ws);

private:
    QList<Workspace> workspaces_;
    void saveWorkspaces();
    void loadWorkspaces();


};

#endif //TASKHELPER_WORKSPACEREPOSITORY_H
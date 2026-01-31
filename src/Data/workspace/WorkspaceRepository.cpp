#include "Data/workspace/WorkspaceRepository.h"
#include <QUuid>
#include <QSettings>

WorkspaceRepository::WorkspaceRepository(QObject* parent) : QObject(parent)
{
    // LINK: Load existing workspaces from persistent storage
    loadWorkspaces();
}

QList<Workspace> WorkspaceRepository::workspaces() const {
    return workspaces_;
}

Workspace WorkspaceRepository::getWorkspaceById(const QString& id) const {
    for (const auto& ws : workspaces_) {
        if (ws.id == id) return ws;
    }
    return {};
}

QString WorkspaceRepository::createWorkspace(const QString& name) {
    Workspace ws;
    ws.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    ws.name = name;
    workspaces_.append(ws);

    // LINK: Persist changes immediately
    saveWorkspaces();

    emit workspaceAdded(ws);
    return ws.id;
}

void WorkspaceRepository::deleteWorkspace(const QString& id)
{

    for (int i = 0; i < workspaces_.size(); ++i) {
        if (workspaces_[i].id == id) {
            workspaces_.removeAt(i);

            // LINK: Persist changes immediately
            saveWorkspaces();
            break;
        }
    }
}

void WorkspaceRepository::saveWorkspaces()
{
    QSettings s;
    s.beginWriteArray("workspaces");

    for (int i = 0; i < workspaces_.size(); ++i) {
        s.setArrayIndex(i);
        s.setValue("id", workspaces_[i].id);
        s.setValue("name", workspaces_[i].name);
    }

    s.endArray();
}

void WorkspaceRepository::loadWorkspaces()
{
    QSettings s;
    int count = s.beginReadArray("workspaces");

    // clear existing list to prevent duplicates
    workspaces_.clear();

    for (int i = 0; i < count; ++i) {
        s.setArrayIndex(i);
        Workspace ws;
        ws.id = s.value("id").toString();
        ws.name = s.value("name").toString();
        workspaces_.append(ws);
    }

    s.endArray();
}





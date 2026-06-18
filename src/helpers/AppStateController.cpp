#include "helpers/AppStateController.h"
#include <QSettings>
#include <QVariantMap>

AppStateController::AppStateController(QObject* parent) : QObject(parent)
{
    QSettings settings;

    // Load persisted state from QSettings
    ctx_.activeWorkspaceId = QUuid::fromString(settings.value("activeWorkspaceId", "").toString());
    ctx_.activeWorkspaceName = settings.value("activeWorkspaceName", "").toString();
    ctx_.isCompact = settings.value("isCompact", false).toBool();
    ctx_.isDarkMode = settings.value("isDarkMode", false).toBool();
    ctx_.selectedTaskId = QUuid::fromString(settings.value("selectedTaskId", "").toString());
    ctx_.selectedFilter = settings.value("selectedFilter", "").toString();

    // Load persisted per-workspace last project mapping
    QVariantMap lastMap = settings.value("lastProjects", QVariantMap()).toMap();
    for (auto it = lastMap.constBegin(); it != lastMap.constEnd(); ++it) {
        const QUuid wsId = QUuid::fromString(it.key());
        const QUuid projId = QUuid::fromString(it.value().toString());
        if (!wsId.isNull() && !projId.isNull()) {
            lastProjects_.insert(wsId, projId);
        }
    }
}

const AppContext& AppStateController::context() const {
    return ctx_;
}

void AppStateController::setActiveWorkspace(const QUuid& id, const QString& name) {
    QSettings settings;
    settings.setValue("activeWorkspaceId", id.toString(QUuid::WithoutBraces));
    settings.setValue("activeWorkspaceName", name);

    if (ctx_.activeWorkspaceId == id && ctx_.activeWorkspaceName == name) return;
    ctx_.activeWorkspaceId = id;
    ctx_.activeWorkspaceName = name;
    emit activeWorkspaceChanged();
}

void AppStateController::setCompact(bool value) {
    if (ctx_.isCompact == value) return;
    ctx_.isCompact = value;
    QSettings settings;
    settings.setValue("isCompact", value);
    emit compactChanged();
}

void AppStateController::setDarkMode(bool value) {
    if (ctx_.isDarkMode == value) return;
    ctx_.isDarkMode = value;
    QSettings settings;
    settings.setValue("isDarkMode", value);
    emit darkModeChanged();
}

void AppStateController::setSelectedTaskId(const QUuid& taskId) {
    if (ctx_.selectedTaskId == taskId) return;
    ctx_.selectedTaskId = taskId;
    QSettings settings;
    settings.setValue("selectedTaskId", taskId.toString(QUuid::WithoutBraces));
    emit selectedTaskChanged();
}

void AppStateController::setSelectedFilter(const QString& filter) {
    if (ctx_.selectedFilter == filter) return;
    ctx_.selectedFilter = filter;
    QSettings settings;
    settings.setValue("selectedFilter", filter);
    emit selectedFilterChanged();
}

void AppStateController::setLastProjectForWorkspace(const QUuid& workspaceId, const QUuid& projectId)
{
    if (workspaceId.isNull()) return;

    if (projectId.isNull()) {
        lastProjects_.remove(workspaceId);
    } else {
        lastProjects_.insert(workspaceId, projectId);
    }

    // Persist mapping into QSettings as a QVariantMap
    QVariantMap map;
    for (auto it = lastProjects_.constBegin(); it != lastProjects_.constEnd(); ++it) {
        map.insert(it.key().toString(QUuid::WithoutBraces), it.value().toString(QUuid::WithoutBraces));
    }
    QSettings settings;
    settings.setValue("lastProjects", map);
}

QUuid AppStateController::lastProjectForWorkspace(const QUuid& workspaceId) const
{
    if (workspaceId.isNull()) return QUuid();
    return lastProjects_.value(workspaceId, QUuid());
}


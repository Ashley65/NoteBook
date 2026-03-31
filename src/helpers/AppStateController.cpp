#include "helpers/AppStateController.h"
#include <QSettings>

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

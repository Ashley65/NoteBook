#include "helpers/AppStateController.h"

AppStateController::AppStateController(QObject* parent) : QObject(parent) {}

const AppContext& AppStateController::context() const {
    return ctx_;
}

void AppStateController::setActiveWorkspace(const QString& id, const QString& name) {
    if (ctx_.activeWorkspaceId == id) return;
    ctx_.activeWorkspaceId = id;
    ctx_.activeWorkspaceName = name;
    emit contextChanged(ctx_);
}

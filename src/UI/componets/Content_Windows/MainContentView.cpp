//
// Created by DevAccount on 28/01/2026.
//

#include <UI/components/Content_Windows/MainContentView.h>
#include <UI/components/Content_Windows/WorkspaceView.h>
#include <helpers/Workspace.h>

MainContentView::MainContentView(QWidget* parent) : QStackedWidget(parent) {}

void MainContentView::setActiveWorkspace(const Workspace& ws)
{
    if (!views_.contains(ws.id)) {
        views_[ws.id] = new WorkspaceView(ws, this);
        addWidget(views_[ws.id]);
    }
    setCurrentWidget(views_[ws.id]);
}

void MainContentView::setActiveWorkspace(const AppContext& ctx)
{
    Workspace ws;
    ws.id = ctx.activeWorkspaceId;
    ws.name = ctx.activeWorkspaceName;
    setActiveWorkspace(ws);
}

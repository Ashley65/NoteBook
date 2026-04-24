//
// Created by DevAccount on 28/01/2026.
//

#include <UI/components/Content_Windows/MainContentView.h>
#include <UI/components/Content_Windows/WorkspaceViewFactory.h>
#include <helpers/Workspace.h>

MainContentView::MainContentView(WorkspaceRepository* repo, QWidget* parent) : QStackedWidget(parent), m_repo(repo) {}

void MainContentView::setActiveWorkspace(const Workspace& ws)
{
    if (!views_.contains(ws.id)) {
        IWorkspaceView* view = WorkspaceViewFactory::createWorkspaceView(ws, m_repo, this);

        views_[ws.id] = view;
        addWidget(view);
    } else {
        views_[ws.id]->updateWorkspace(ws);
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

void MainContentView::setActiveProject(const Project& project)
{
    m_activeProject = project;

    IWorkspaceView* view = nullptr;
    if (views_.contains(project.workspaceId)) {
        view = views_[project.workspaceId];
    } else if (m_repo) {
        const Workspace ws = m_repo->getWorkspaceById(project.workspaceId);
        if (!ws.id.isNull()) {
            view = WorkspaceViewFactory::createWorkspaceView(ws, m_repo, this);
            views_[ws.id] = view;
            addWidget(view);
        }
    }

    if (!view) {
        return;
    }

    setCurrentWidget(view);
    view->setActiveProject(project);
}

//
// Created by DevAccount on 28/01/2026.
//

#include <UI/components/Content_Windows/MainContentView.h>
#include <UI/components/Content_Windows/WorkspaceViewFactory.h>
#include <helpers/Workspace.h>

MainContentView::MainContentView(WorkspaceRepository* repo, QWidget* parent) : QStackedWidget(parent), m_repo(repo) {}

void MainContentView::setActiveWorkspace(const Workspace& ws)
{
    if (!m_repo) return;
    
    if (!views_.contains(ws.id)) {
        IWorkspaceView* view = WorkspaceViewFactory::createWorkspaceView(ws, m_repo, this);
        if (!view) return;

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
    if (!m_repo) return;
    
    m_activeProject = project;

    // 1. Look up the view using the PROJECT ID, not the Workspace ID!
    if (!views_.contains(project.id)) {

        // 2. The view doesn't exist for this specific project yet.
        // We still need the parent workspace data to build it.
        const Workspace ws = m_repo->getWorkspaceById(project.workspaceId);
        if (ws.id.isNull()) return; // Failsafe

        // 3. Create a brand new, dedicated widget just for this Project Tab
        IWorkspaceView* newProjectView = WorkspaceViewFactory::createWorkspaceView(ws, m_repo, this);
        if (!newProjectView) return;

        //  its state to show the project
        newProjectView->setActiveProject(project);

        // 4. Save it in the dictionary under the PROJECT ID and add to stack
        views_[project.id] = newProjectView;
        addWidget(newProjectView);
    } else {
        // If it already exists, just update it in case background data changed
        views_[project.id]->setActiveProject(project);
    }

    // 5. Flip the QStackedWidget to the dedicated Project View
    setCurrentWidget(views_[project.id]);
}

void MainContentView::discardView(const QUuid& contextId)
{
    if (views_.contains(contextId)) {
        IWorkspaceView* view = views_[contextId];
        if (currentWidget() != view) {
            removeWidget(view);
            views_.remove(contextId);
            view->deleteLater();
            qDebug() << "Discarded view for context:" << contextId;
        }
    }
}

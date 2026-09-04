//
// Created by DevAccount on 28/01/2026.
//

#include <UI/components/Content_Windows/MainContentView.h>
#include <UI/components/Content_Windows/WorkspaceViewFactory.h>
#include <UI/components/Content_Windows/page/wsNotePage.h>
#include <UI/components/Content_Windows/page/wsNoteListPage.h>
#include <UI/components/Content_Windows/page/wsTaskBoardPage.h>
#include <UI/components/Content_Windows/page/wsHomePage.h>
#include <helpers/Workspace.h>

MainContentView::MainContentView(WorkspaceRepository* repo, QWidget* parent) : QStackedWidget(parent), m_repo(repo)
{
    setObjectName("mainContentView");
    setBorderColor("#6366F1");
}

void MainContentView::setBorderColor(const QString& colorHex)
{
    m_borderColor = colorHex;
    setStyleSheet(QString(R"(
        QStackedWidget#mainContentView {
            border: 1.5px solid %1;
            border-radius: 8px;
            background-color: transparent;
            margin: 0px;
        }
    )").arg(colorHex));
}

void MainContentView::setActiveWorkspace(const Workspace& ws)
{
    if (!m_repo) return;
    
    const QString key = "ws_" + ws.id.toString(QUuid::WithoutBraces);
    if (!views_.contains(key)) {
        IWorkspaceView* view = WorkspaceViewFactory::createWorkspaceView(ws, m_repo, this);
        if (!view) return;

        if (wsHomePage* homePage = qobject_cast<wsHomePage*>(view)) {
            connect(homePage, &wsHomePage::noteOpenRequested, this, [this](const QString& noteId) {
                emit noteOpenRequested(noteId);
            });
        }

        views_[key] = view;
        addWidget(view);
    } else {
        views_[key]->updateWorkspace(ws);
    }
    setCurrentWidget(views_[key]);
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

    const QString key = "proj_" + project.id.toString(QUuid::WithoutBraces);
    if (!views_.contains(key)) {
        const Workspace ws = m_repo->getWorkspaceById(project.workspaceId);
        if (ws.id.isNull()) return;

        IWorkspaceView* newProjectView = WorkspaceViewFactory::createWorkspaceView(ws, m_repo, this);
        if (!newProjectView) return;

        newProjectView->setActiveProject(project);

        views_[key] = newProjectView;
        addWidget(newProjectView);
    } else {
        views_[key]->setActiveProject(project);
    }

    setCurrentWidget(views_[key]);
}

void MainContentView::loadNoteView(const Note& note)
{
    if (!m_repo) return;

    const QString key = "note_" + note.id.toString(QUuid::WithoutBraces);
    if (!views_.contains(key)) {
        const Workspace ws = m_repo->getWorkspaceById(note.workspaceId);
        if (ws.id.isNull()) return;

        Workspace noteWs = ws;
        noteWs.type = "note";
        IWorkspaceView* noteView = WorkspaceViewFactory::createWorkspaceView(noteWs, m_repo, this);
        if (!noteView) return;

        if (wsNotePage* page = qobject_cast<wsNotePage*>(noteView)) {
            page->loadNote(note.id.toString(QUuid::WithoutBraces));
        }

        views_[key] = noteView;
        addWidget(noteView);
    } else {
        if (wsNotePage* page = qobject_cast<wsNotePage*>(views_[key])) {
            page->loadNote(note.id.toString(QUuid::WithoutBraces));
        }
    }

    setCurrentWidget(views_[key]);
}

void MainContentView::loadNoteListView(const Workspace& ws, const Project& project)
{
    if (!m_repo || ws.id.isNull()) return;

    const QUuid contextId = !project.id.isNull() ? project.id : ws.id;
    const QString key = "notelist_" + contextId.toString(QUuid::WithoutBraces);

    if (!views_.contains(key))
    {
        Workspace noteListWs = ws;
        noteListWs.type = "notelist";

        IWorkspaceView* listView = WorkspaceViewFactory::createWorkspaceView(noteListWs, m_repo, this);
        if (!listView) return;

        if (wsNoteListPage* page = qobject_cast<wsNoteListPage*>(listView)) {
            if (!project.id.isNull()) {
                page->setActiveProject(project);
            }
            connect(page, &wsNoteListPage::noteOpenRequested, this, [this](const QString& noteId) {
                emit noteOpenRequested(noteId);
            });
        }

        views_[key] = listView;
        addWidget(listView);
    }
    else
    {
        if (wsNoteListPage* page = qobject_cast<wsNoteListPage*>(views_[key])) {
            if (!project.id.isNull()) {
                page->setActiveProject(project);
            } else {
                page->updateWorkspace(ws);
            }
        }
    }

    setCurrentWidget(views_[key]);
}

void MainContentView::loadTaskBoardView(const Workspace& ws, const Project& project)
{
    if (!m_repo || ws.id.isNull()) return;

    const QUuid contextId = !project.id.isNull() ? project.id : ws.id;
    const QString key = "taskboard_" + contextId.toString(QUuid::WithoutBraces);

    if (!views_.contains(key))
    {
        Workspace taskWs = ws;
        taskWs.type = "taskboard";

        IWorkspaceView* boardView = WorkspaceViewFactory::createWorkspaceView(taskWs, m_repo, this);
        if (!boardView) return;

        if (wsTaskBoardPage* page = qobject_cast<wsTaskBoardPage*>(boardView)) {
            if (!project.id.isNull()) {
                page->setActiveProject(project);
            }
            connect(page, &wsTaskBoardPage::noteOpenRequested, this, [this](const QString& noteId) {
                emit noteOpenRequested(noteId);
            });
        }

        views_[key] = boardView;
        addWidget(boardView);
    }
    else
    {
        if (wsTaskBoardPage* page = qobject_cast<wsTaskBoardPage*>(views_[key])) {
            if (!project.id.isNull()) {
                page->setActiveProject(project);
            } else {
                page->updateWorkspace(ws);
            }
        }
    }

    setCurrentWidget(views_[key]);
}

void MainContentView::discardView(const QUuid& contextId)
{
    const QString idStr = contextId.toString(QUuid::WithoutBraces);
    const auto keys = views_.keys();
    for (const QString& key : keys) {
        if (key.contains(idStr)) {
            IWorkspaceView* view = views_[key];
            if (currentWidget() != view) {
                removeWidget(view);
                views_.remove(key);
                view->deleteLater();
                qDebug() << "Discarded view for context key:" << key;
            }
        }
    }
}

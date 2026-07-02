
#ifndef TASKHELPER_WORKSPACEVIEWFACTORY_H
#define TASKHELPER_WORKSPACEVIEWFACTORY_H

#include <QWidget>
#include <QString>

#include "IWorkspaceView.h"
#include "helpers/Workspace.h"
#include "WorkspaceView.h"
#include "page/NetworkTestPage.h"
#include "page/wsHomePage.h"
#include "page/wsNotePage.h"

class WorkspaceViewFactory
{
public:
    // Factory method to create workspace views based on workspace type
    static IWorkspaceView* createWorkspaceView(const Workspace& ws, WorkspaceRepository* repo, QWidget* parent)
    {
        QString type = ws.type.toLower();
        if (type == "lab" || type == "test") {
            return new NetworkTestPage(ws, parent);
        }else if (type == "note") {
            return new wsNotePage(ws, repo, parent);
        }
        else {
            return new wsHomePage(ws, repo, parent);
        }
    }

};

#endif //TASKHELPER_WORKSPACEVIEWFACTORY_H
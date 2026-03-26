
#ifndef TASKHELPER_WORKSPACEVIEWFACTORY_H
#define TASKHELPER_WORKSPACEVIEWFACTORY_H

#include <QWidget>
#include <QString>

#include "IWorkspaceView.h"
#include "helpers/Workspace.h"
#include "WorkspaceView.h"
#include "page/NetworkTestPage.h"

class WorkspaceViewFactory
{
public:
    // Factory method to create workspace views based on workspace type
    static IWorkspaceView* createWorkspaceView(const Workspace& ws, QWidget* parent)
    {
        QString type = ws.type.toLower();

        if (type == "lab" || type == "test") {

            return new NetworkTestPage(ws, parent);
        } else {
            return new WorkspaceView(ws, parent);
        }
    }
};

#endif //TASKHELPER_WORKSPACEVIEWFACTORY_H
#include "UI/components/Content_Windows/WorkspaceView.h"
#include <QVBoxLayout>
#include <QLabel>

WorkspaceView::WorkspaceView(const Workspace& ws, QWidget* parent)
    : IWorkspaceView(ws, parent), workspaceId_(ws.id)
{
    auto* layout = new QVBoxLayout(this);
    auto* label = new QLabel("Workspace: " + ws.name, this);
    label->setAlignment(Qt::AlignCenter);
    label->setStyleSheet("font-size: 24px; font-weight: bold; color: #333;");
    layout->addWidget(label);
    
    // Placeholder for more workspace-specific content
    auto* subLabel = new QLabel("ID: " + ws.id.toString(QUuid::WithoutBraces), this);
    subLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(subLabel);
}

void WorkspaceView::refresh()
{
    // Implementation for refreshing the view
}

void WorkspaceView::updateWorkspace(const Workspace& ws)
{
    workspaceId_ = ws.id;
    // Update UI elements based on the new workspace data
}

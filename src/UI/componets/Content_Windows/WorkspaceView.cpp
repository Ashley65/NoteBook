#include "UI/components/Content_Windows/WorkspaceView.h"
#include <QVBoxLayout>
#include <QLabel>

WorkspaceView::WorkspaceView(const Workspace& ws, QWidget* parent)
    : QWidget(parent), workspaceId_(ws.id)
{
    auto* layout = new QVBoxLayout(this);
    auto* label = new QLabel("Workspace: " + ws.name, this);
    label->setAlignment(Qt::AlignCenter);
    label->setStyleSheet("font-size: 24px; font-weight: bold; color: #333;");
    layout->addWidget(label);
    
    // Placeholder for more workspace-specific content
    auto* subLabel = new QLabel("ID: " + ws.id, this);
    subLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(subLabel);
}

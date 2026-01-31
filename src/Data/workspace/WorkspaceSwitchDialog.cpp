//
// Created by DevAccount on 30/01/2026.
//
#include "Data/workspace/WorkspaceSwitchDialog.h"
#include <QVBoxLayout>
#include <QDialogButtonBox>



WorkspaceSwitchDialog::WorkspaceSwitchDialog(
    const QList<Workspace>& workspaces,
    const QString& activeWorkspaceId,
    QWidget* parent) : QDialog(parent)
{
    setWindowTitle("Switch Workspace");
    setModal(true);
    resize(300, 400);

    auto* layout = new QVBoxLayout(this);
    list_ = new QListWidget(this);

    for (const auto& ws : workspaces)
    {
        auto* item = new QListWidgetItem(ws.name, list_);
        item->setData(Qt::UserRole, ws.id);

        if (ws.id == activeWorkspaceId)
        {
            item->setSelected(true);
        }
    }

    layout->addWidget(list_);

    auto* buttons = new QDialogButtonBox(
         QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    layout->addWidget(buttons);

    connect(buttons, &QDialogButtonBox::accepted, this, [this]() {
        if (auto* item = list_->currentItem()) {
            selectedId_ = item->data(Qt::UserRole).toString();
        }
        accept();
    });

    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
}


QString WorkspaceSwitchDialog::selectedWorkspaceId() const
{
    return selectedId_;
}
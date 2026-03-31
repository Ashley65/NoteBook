//
// Created by DevAccount on 27/02/2026.
//

#include "Data/workspace/WorkspaceDeleteDialog.h"
#include <QVBoxLayout>
#include <QMessageBox>

WorkspaceDeleteDialog::WorkspaceDeleteDialog(const QList<Workspace>& workspaces, QWidget* parent)
{
   setWindowTitle(tr("Workspace Delete "));
   setModal(true);
   resize(300, 400);

   auto* layout = new QVBoxLayout(this);
   list_ = new QListWidget(this);

   for (const auto& ws : workspaces)
   {
       auto* item = new QListWidgetItem(ws.name, list_);
       item->setData(Qt::UserRole, ws.id);

       if (ws.protectedMode) {
           item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
           item->setToolTip("This workspace is protected and cannot be deleted.");
       }

       if (ws.isArchived) {
           item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
           item->setToolTip("This workspace is archived and cannot be deleted.");
       }

       if (ws.isPinned) {
              item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
              item->setToolTip("This workspace is pinned and cannot be deleted.");
       }


   }

    layout->addWidget(list_);

    auto* buttons = new QDialogButtonBox(
         QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    layout->addWidget(buttons);

    connect(buttons, &QDialogButtonBox::accepted, this, [this]() {
    if (auto* item = list_->currentItem()) {
        selectedId_ = item->data(Qt::UserRole).toUuid();
        QString name = item->text(); // Get the name from the list item
        validateSelection(name);     // Show the confirmation box
    } else {
        // Optionally show a warning if nothing is selected
        QMessageBox::warning(this, tr("No Selection"), tr("Please select a workspace to delete."));
    }
});

    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

}

QUuid WorkspaceDeleteDialog::selectedWorkspaceId() const
{
    return selectedId_;
}

void WorkspaceDeleteDialog::validateSelection(const QString& name)
{

    // This function will be used to generate a prompt to the user asking them to confirm the deletion of the workspace, this is to prevent accidental deletion of workspaces. It can be called when the user clicks "Ok" to ensure they want to delete the selected workspace.
    QMessageBox confirmBox;
    confirmBox.setIcon(QMessageBox::Warning);

    confirmBox.setText(tr("Are you sure you want to delete the workspace \"%1\"?").arg(name));
    confirmBox.setInformativeText("This workspace will not be recoverable after deletion.");
    confirmBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    confirmBox.setDefaultButton(QMessageBox::No);

    if (confirmBox.exec() == QMessageBox::Yes) {
        // Proceed with deletion logic or accept the dialogue
        accept();
    }


}




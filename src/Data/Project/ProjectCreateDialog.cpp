//
// Created by DevAccount on 21/04/2026.
//

#include "Data/Project/ProjectCreateDialog.h"
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>

ProjectCreateDialog::ProjectCreateDialog(const QUuid& workspaceId, QWidget* parent, WorkspaceRepository* repository)
    : QDialog(parent), repository_(repository), workspaceId_(workspaceId)
{
    setWindowTitle("Create New Project");
    setModal(true);
    setMinimumSize(400, 200);

    auto* layout = new QVBoxLayout(this);

    nameEdit_ = new QLineEdit(this);
    nameEdit_->setPlaceholderText("Project Name");
    layout->addWidget(nameEdit_);

    descriptionEdit_ = new QLineEdit(this);
    descriptionEdit_->setPlaceholderText("Project Description (optional)");
    layout->addWidget(descriptionEdit_);

    buttonBox_ = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    if (QPushButton* okButton = buttonBox_->button(QDialogButtonBox::Ok)) {
        okButton->setEnabled(false); // Initially disabled
    }
    layout->addWidget(buttonBox_);

    connect(nameEdit_, &QLineEdit::textChanged, this, &ProjectCreateDialog::validateInput);
    connect(buttonBox_, &QDialogButtonBox::accepted, this, &ProjectCreateDialog::handleAccept);
    connect(buttonBox_, &QDialogButtonBox::rejected, this, &ProjectCreateDialog::reject);
}

QString ProjectCreateDialog::projectName() const
{
    return nameEdit_->text().trimmed();
}

QString ProjectCreateDialog::projectDescription() const
{
    return descriptionEdit_->text().trimmed();
}

void ProjectCreateDialog::validateInput(const QString& text)
{
    const QString candidateName = text.trimmed();
    m_isNameUnique = true;

    if (repository_ && !workspaceId_.isNull() && !candidateName.isEmpty()) {
        const QList<Project> existingProjects = repository_->getProjectsByWorkspace(workspaceId_);
        for (const Project& project : existingProjects) {
            if (project.name.trimmed().compare(candidateName, Qt::CaseInsensitive) == 0) {
                m_isNameUnique = false;
                break;
            }
        }
    }

    QPushButton* okButton = buttonBox_->button(QDialogButtonBox::Ok);
    if (!okButton) {
        return;
    }

    const bool isNameValid = !candidateName.isEmpty() && m_isNameUnique;
    okButton->setEnabled(isNameValid);

    // Give quick inline feedback when duplicate names are entered.
    nameEdit_->setToolTip(m_isNameUnique ? QString() : QStringLiteral("A project with this name already exists in this workspace."));
}

void ProjectCreateDialog::SaveCreatedProject()
{
    handleAccept();
}

void ProjectCreateDialog::handleAccept()
{
    validateInput(nameEdit_->text());
    if (projectName().isEmpty() || !m_isNameUnique) {
        return;
    }

    accept();
}


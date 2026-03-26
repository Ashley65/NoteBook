

#include "Data/workspace/WorkspaceCreateDialog.h"

#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QRegularExpression>
#include <QLabel>

WorkspaceCreateDialog::WorkspaceCreateDialog(QWidget *parent, WorkspaceRepository* repository) : QDialog(parent), repository_(repository), unique(true)
{
    setWindowTitle("Create New Workspace");
    setModal(true);
    resize(300, 250);

    auto* layout = new QVBoxLayout(this);

    layout->addWidget(new QLabel("Name:", this));
    nameEdit_ = new QLineEdit(this);
    nameEdit_->setPlaceholderText("Enter workspace name...");
    layout->addWidget(nameEdit_);

    layout->addWidget(new QLabel("Type:", this));
    typeComboBox_ = new QComboBox(this);
    typeComboBox_->addItems({"Default", "Work", "Study", "Custom"});

    layout->addWidget(typeComboBox_);

    layout->addWidget(new QLabel("Description:", this));
    descriptionEdit_ = new QLineEdit(this);
    descriptionEdit_->setPlaceholderText("Optional description...");
    layout->addWidget(descriptionEdit_);

    // Error label added here to be correctly positioned in the layout
    auto* errorLabel = new QLabel(this);
    errorLabel->setObjectName("errorLabel");
    errorLabel->setStyleSheet("color: red;");
    errorLabel->hide();
    layout->addWidget(errorLabel);

    buttonBox_ = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    layout->addWidget(buttonBox_);

    // Connect text changes to validation
    connect(nameEdit_, &QLineEdit::textChanged, this, &WorkspaceCreateDialog::validateInput);

    // Standard Dialog connections
    connect(buttonBox_, &QDialogButtonBox::accepted, this, &WorkspaceCreateDialog::handleAccept);
    connect(buttonBox_, &QDialogButtonBox::rejected, this, &QDialog::reject);

    // Initial validation to disable "Ok" if name is empty
    validateInput(nameEdit_->text());
}

QString WorkspaceCreateDialog::workspaceName() const
{
    return nameEdit_->text().trimmed();
}

QString WorkspaceCreateDialog::workspaceType() const
{
    return typeComboBox_->currentText().trimmed();
}

QString WorkspaceCreateDialog::workspaceDescription() const
{
    return descriptionEdit_->text().trimmed();
}

void WorkspaceCreateDialog::validateInput(const QString& text)
{
    static const QRegularExpression regex(QStringLiteral("[\\\\/:*?\"<>|]"));
    QString trimmedName = text.trimmed();

    bool isNotEmpty = !trimmedName.isEmpty();
    bool hasValidChars = text.indexOf(regex) == -1;

    // Uniqueness check
    bool isUnique = true;
    if (repository_) {
        for (const auto& ws : repository_->workspaces()) {
            if (ws.name.trimmed().compare(trimmedName, Qt::CaseInsensitive) == 0) {
                isUnique = false;
                break;
            }
        }
    }

    if (QPushButton* okButton = buttonBox_->button(QDialogButtonBox::Ok)) {
        okButton->setEnabled(isNotEmpty && hasValidChars && isUnique);
    }

    QLabel* errorLabel = findChild<QLabel*>("errorLabel");
    if (errorLabel) {
        if (isNotEmpty && hasValidChars && !isUnique) {
            errorLabel->setText("Workspace name already exists.");
            errorLabel->show();
        } else {
            errorLabel->hide();
        }
    }
}

void WorkspaceCreateDialog::ValidateName(const QString& name)
{
    // This function will be used to check if the name has already been used or not before allowing the user to create a new workspace. It can be called when the user clicks "Ok" to ensure the name is unique.
    // why this should work is by using the workspace check if the name already exists in the repository, if it does, then we can show an error message and prevent the dialogue from closing until a unique name is entered.

    unique = true; // Assume the name is unique until we check the repository
    QString trimmedName = name.trimmed();

    // Check the repository for the save workspace
    if (repository_) {
        auto workspaces = repository_->workspaces();
        for (const auto& ws : workspaces)
        {
            if (ws.name.trimmed().compare(trimmedName, Qt::CaseInsensitive) == 0)
            {
                unique = false;
                break;
            }
        }
    }

    QLabel* errorLabel = findChild<QLabel*>("errorLabel");
    if (!unique)
    {
        // show error text at the bottom of the input field saying "Workspace name already exists. Please choose a different name."
        if (!errorLabel) {
            errorLabel = new QLabel(this);
            errorLabel->setObjectName("errorLabel");
            errorLabel->setStyleSheet("color: red;");
            static_cast<QVBoxLayout*>(layout())->addWidget(errorLabel);
        }
        errorLabel->setText("Workspace name already exists. Please choose a different name.");
        errorLabel->show();
    } else if (errorLabel)
    {
        errorLabel->hide();
    }
}

void WorkspaceCreateDialog::SaveCreatedWorkspace()
{
    // This function will be called after the press of the "Ok" button to save the workspace to the repository
}

void WorkspaceCreateDialog::handleAccept()
{
    QString name = nameEdit_->text().trimmed();

    // Validate the name before accepting
    ValidateName(name);

    if (unique)
    {
        // if it is unique, we can proceed to close the dialog
        // the actual creation will be handled by the caller who has the master repository
        accept();
    }
}

// need to add an input validator to ensure the name is valid (not empty, no special chars, etc.), and also consider adding a preview of the workspace name or a confirmation step if needed.

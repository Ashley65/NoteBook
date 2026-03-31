//
// Created by DevAccount on 13/03/2026.
//
#include "Data/workspace/WorkspaceSettingsWindow.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include "Data/helper/validater.h"


// The main idea is that when the user want to change some setting in the workspace, they click the setting button a and popup wind for the current workspace will appear
WorkspaceSettingsWindow::WorkspaceSettingsWindow(const QUuid& workspaceId,const QString& workspaceName,WorkspaceRepository* repository,QWidget* parent)
:QDialog(parent), m_workspaceId(workspaceId), m_workspaceName(workspaceName), m_repository(repository)
{
    // Set the window title
    //setWindowTitle(tr("Workspace Settings: \"%1\"").arg(workspaceId));
    setupUi();
    connectSignals();
}

WorkspaceSettingsWindow::~WorkspaceSettingsWindow()
{
}

void WorkspaceSettingsWindow::onSaveCLicked()
{
    if (!nameLineEdit_) return;

    const NameValidationResult result =
        validateWorkspaceName(nameLineEdit_->text(), m_repository, m_workspaceId);

    if (nameErrorLabel_) {
        nameErrorLabel_->setText(result.message);
        nameErrorLabel_->setVisible(!result.ok);
    }


    if (!result.ok) {
        nameLineEdit_->setFocus();
        return;
    }

    accept();
}

void WorkspaceSettingsWindow::onCancelClicked()
{
    reject();
}

void WorkspaceSettingsWindow::onChangeIconClicked()
{
}

void WorkspaceSettingsWindow::setupUi()
{
    setModal(true);
    resize(400, 300);

    // load the main mainLayout_
    mainLayout_ = new QVBoxLayout(this);
    mainLayout_->setContentsMargins(10, 10, 10, 10);

    QFrame* container = borderFrame();
    mainLayout_->addWidget(container);

    // Add your sections into the frame's layout
    auto* containerLayout = qobject_cast<QVBoxLayout*>(container->layout());
    if (containerLayout) {
        // containerLayout->addWidget(...);
        // add generalInfo
        getGeneralInfo(containerLayout);
        setupActionsButtons();
        setAppearance(containerLayout);
        getPermissions(containerLayout);


    }

}

void  WorkspaceSettingsWindow::getGeneralInfo(QVBoxLayout* targetLayout)

{
    if (!targetLayout) return;

    auto* label = new QLabel("General Information", this);
    label->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    QFont font = label->font();
    font.setPointSize(14); // Increase the number to make it bigger (default is usually 8 or 9)
    font.setBold(true);    // Optional: making it bold looks great for headers!
    label->setFont(font);

    targetLayout->addWidget(label);

    auto* hLine = new QFrame(this);
    hLine->setFrameShape(QFrame::HLine);
    hLine->setFrameShadow(QFrame::Sunken);
    targetLayout->addWidget(hLine);

    // Add a stretching spacer at the end to push everything up
    targetLayout->addStretch();

    //create a horizontal box for the Name input and the Type selection
    auto* NTLayout = new QGridLayout();

    auto* nameLabel = new QLabel("Name:", this);
    auto* typeLabel = new QLabel("Type:", this);
    auto* descriptionLabel = new QLabel("Description:", this);

    nameLineEdit_ = new QLineEdit(this);
    nameLineEdit_->setText(m_workspaceName);                       // preferred
    nameLineEdit_->setPlaceholderText(tr("Workspace name"));
    nameErrorLabel_ = new QLabel(this);
    nameErrorLabel_->setStyleSheet("color: red;");
    nameErrorLabel_->hide();

    NTLayout->addWidget(nameLabel, 0, 0);
    NTLayout->addWidget(nameLineEdit_, 1, 0);
    NTLayout->addWidget(nameErrorLabel_, 0, 2);

    typeComboBox_ = new QComboBox(this);
    typeComboBox_->addItems({"Default", "Work", "Study", "Custom"});
    NTLayout->addWidget(typeLabel, 0, 1);
    NTLayout->addWidget(typeComboBox_, 1, 1);

    descriptionTextEdit_ = new QTextEdit(this);
    descriptionTextEdit_->setPlaceholderText(tr("Workspace description"));
    NTLayout->addWidget(descriptionLabel, 2, 0);
    NTLayout->addWidget(descriptionTextEdit_, 3, 0, 1, 2); // Span across 2 columns





    targetLayout->addLayout(NTLayout);

}

void WorkspaceSettingsWindow::setAppearance(QVBoxLayout* targetLayout)
{
    if (!targetLayout) return;

    auto *label = new QLabel("Appearance", this);
    label->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    QFont font = label->font();
    font.setPointSize(14); // Increase the number to make it bigger (default is
    font.setBold(true);    // Optional: making it bold looks great for headers!
    label->setFont(font);

    targetLayout->addWidget(label);
    auto* hLine = new QFrame(this);
    hLine->setFrameShape(QFrame::HLine);
    hLine->setFrameShadow(QFrame::Sunken);
    targetLayout->addWidget(hLine);

    // Add a stretching spacer at the end to push everything up
    targetLayout->addStretch();

     auto* HoLayout = new QHBoxLayout();

     iconPreviewLabel_ = new QLabel(this);
     iconPreviewLabel_->setFixedSize(40, 40);
     iconPreviewLabel_->setStyleSheet("background-color: gray; border-radius: 20px;");
     HoLayout->addWidget(iconPreviewLabel_);

     changeIconButton_ = new QPushButton("Change Icon", this);
     HoLayout->addWidget(changeIconButton_);

     colourHexInput_ = new QLineEdit(this);
     colourHexInput_->setPlaceholderText("#RRGGBB");
     HoLayout->addWidget(colourHexInput_);

     // Add some colour swatches
     QStringList colors = {"#FF5733", "#33FF57", "#3357FF", "#F1C40F", "#8E44AD"};
     for (const QString& color : colors) {
         QPushButton* swatch = new QPushButton(this);
         swatch->setFixedSize(20, 20);
         swatch->setStyleSheet(QString("background-color: %1; border-radius: 10px;").arg(color));
         colorSwatches.append(swatch);
         HoLayout->addWidget(swatch);
     }

    targetLayout->addLayout(HoLayout);

}

void WorkspaceSettingsWindow::getPermissions(QVBoxLayout* targetLayout)
{
    if (!targetLayout) return;

    auto *label = new QLabel("Permissions & Status", this);
    label->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    QFont font = label->font();
    font.setPointSize(14); // Increase the number to make it bigger (default is
    font.setBold(true);    // Optional: making it bold looks great for headers!
    label->setFont(font);

    targetLayout->addWidget(label);
    auto* hLine = new QFrame(this);
    hLine->setFrameShape(QFrame::HLine);
    hLine->setFrameShadow(QFrame::Sunken);
    targetLayout->addWidget(hLine);

    auto* HoLayout = new QGridLayout();



    protectedToggle = new QCheckBox("Protected Mode", this);
    protectedToggle->setToolTip("Password Required to access workspace");
    HoLayout->addWidget(protectedToggle, 0, 0);

    archivedToggle = new QCheckBox("Archived", this);
    archivedToggle->setToolTip("Archived workspaces are hidden from the main view but are not deleted. Use this to declutter your workspace list without losing data.");
    HoLayout->addWidget(archivedToggle, 0, 1);

    pinnedToggle = new QCheckBox("Pinned", this);
    pinnedToggle->setToolTip("Pinned workspaces are displayed at the top of your workspace list for quick access. Use this to keep your most important workspaces easily accessible.");
    HoLayout->addWidget(pinnedToggle, 0, 2);


    targetLayout->addLayout(HoLayout);

    auto* passwordLayout = new QHBoxLayout();
    passwordLabel = new QLabel("Password:", this);
    passwordInput = new QLineEdit(this);
    passwordInput->setEchoMode(QLineEdit::Password);
    passwordLayout->addWidget(passwordLabel);
    passwordLayout->addWidget(passwordInput);

    passwordLabel-> hide();
    passwordInput->hide();
    targetLayout->addLayout(passwordLayout);

    // Add a stretching spacer at the end to push everything up
    targetLayout->addStretch();

}

void WorkspaceSettingsWindow::setupActionsButtons()
{
    if (!mainLayout_) return;

    auto* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    cancelBtn = new QPushButton(tr("Cancel"), this);
    saveBtn = new QPushButton(tr("Save"), this);

    buttonLayout->addWidget(cancelBtn);
    buttonLayout->addWidget(saveBtn);

    mainLayout_->addLayout(buttonLayout);
}


void WorkspaceSettingsWindow::connectSignals()
{
    if (!nameLineEdit_) return;

    if (cancelBtn) {
        connect(cancelBtn, &QPushButton::clicked, this, &WorkspaceSettingsWindow::onCancelClicked);
    }
    if (saveBtn) {
        connect(saveBtn, &QPushButton::clicked, this, &WorkspaceSettingsWindow::onSaveCLicked);
    }

    connect(protectedToggle, &QCheckBox::toggled, this, [this](bool checked) {
        if (checked) {
            passwordLabel->show();
            passwordInput->show();
        } else {
            // Hide password input when protected mode is disabled
            passwordLabel->hide();
            passwordInput->hide();
        }
    });

    connect(nameLineEdit_, &QLineEdit::textChanged, this, [this](const QString& text) {
        const NameValidationResult result =
            validateWorkspaceName(text, m_repository, m_workspaceId);

        if (nameErrorLabel_) {
            nameErrorLabel_->setText(result.message);
            nameErrorLabel_->setVisible(!result.ok);
        }

        if (saveBtn) {
            saveBtn->setEnabled(result.ok);
        }
    });

    // Run once at startup so initial state is correct
    const NameValidationResult initialResult =
        validateWorkspaceName(nameLineEdit_->text(), m_repository, m_workspaceId);

    if (nameErrorLabel_) {
        nameErrorLabel_->setText(initialResult.message);
        nameErrorLabel_->setVisible(!initialResult.ok);
    }

    if (saveBtn) {
        saveBtn->setEnabled(initialResult.ok);
    }
}


QFrame* WorkspaceSettingsWindow::borderFrame()
{
    auto* _borderFrame = new QFrame(this);
    _borderFrame->setObjectName("workspaceSettingsBorderFrame");
    _borderFrame->setFrameShape(QFrame::NoFrame);

    _borderFrame->setStyleSheet(R"(
        #workspaceSettingsBorderFrame {
            background-color: rgba(32, 32, 32, 220);
            border: 1px solid rgba(255, 255, 255, 25);
            border-radius: 10px;
        }
    )");

    auto* contentLayout = new QVBoxLayout(_borderFrame);
    contentLayout->setContentsMargins(16, 16, 16, 16);
    contentLayout->setSpacing(12);

    return _borderFrame;
}

// QFrame* WorkspaceSettingsWindow::createHorizontalLine()
// {
//
// }

//
// Created by DevAccount on 22/03/2026.
//

#include "UI/components/Content_Windows/page/NetworkTestPage.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QTextEdit>
#include <QDateTime>
#include <QPushButton>


NetworkTestPage::NetworkTestPage(const Workspace& ws, QWidget* parent)
    : IWorkspaceView(ws, parent), workspaceId_(ws.id)
{
    setupUi();
    onDisplayWorkspaceInfo();

}



void NetworkTestPage::setupUi()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(15);

    // Header
    auto* headerLabel = new QLabel("Test & Component playground", this);
    headerLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #333;");
    mainLayout->addWidget(headerLabel);

    // Workspace Info Group
    // Workspace Info Section
    auto* infoGroupBox = new QGroupBox("Workspace Information", this);
    auto* infoLayout = new QVBoxLayout(infoGroupBox);
    m_infoLabel = new QLabel(this);
    m_infoLabel->setWordWrap(true);
    m_infoLabel->setStyleSheet("color: #555; font-size: 11px;");
    infoLayout->addWidget(m_infoLabel);
    mainLayout->addWidget(infoGroupBox);



    // Test Input Section
    auto* testGroupBox = new QGroupBox("Component Test Input", this);
    auto* testLayout = new QVBoxLayout(testGroupBox);

    m_testInputField = new QLineEdit(this);
    m_testInputField->setPlaceholderText("Enter test data here...");
    testLayout->addWidget(m_testInputField);

    auto* testButtonLayout = new QHBoxLayout();
    auto* testBtn = new QPushButton("Test Input", this);
    connect(testBtn, &QPushButton::clicked, this, &NetworkTestPage::onTestButtonClicked);
    testButtonLayout->addWidget(testBtn);
    testLayout->addLayout(testButtonLayout);

    mainLayout->addWidget(testGroupBox);

    // Creation Section
    auto* createGroupBox = new QGroupBox("Create Test Data", this);
    auto* createLayout = new QVBoxLayout(createGroupBox);

    auto* createButtonLayout = new QHBoxLayout();

    auto* createTaskBtn = new QPushButton("Create Task", this);
    connect(createTaskBtn, &QPushButton::clicked, this, &NetworkTestPage::onCreateTaskClicked);
    createButtonLayout->addWidget(createTaskBtn);

    auto* createNoteBtn = new QPushButton("Create Note", this);
    connect(createNoteBtn, &QPushButton::clicked, this, &NetworkTestPage::onCreateNoteClicked);
    createButtonLayout->addWidget(createNoteBtn);

    createLayout->addLayout(createButtonLayout);
    mainLayout->addWidget(createGroupBox);

    auto* statusGroupBox = new QGroupBox("Status", this);
    auto* statusLayout = new QVBoxLayout(statusGroupBox);
    m_statusLabel = new QLabel("Ready", this);
    m_statusLabel->setStyleSheet("color: #28a745; font-weight: bold;");
    statusLayout->addWidget(m_statusLabel);
    mainLayout->addWidget(statusGroupBox);


    // Stretch to fill remaining space
    mainLayout->addStretch();



}

void NetworkTestPage::refresh()
{
    onDisplayWorkspaceInfo();
}

void NetworkTestPage::updateWorkspace(const Workspace& ws)
{
    workspaceId_ = ws.id;
    onDisplayWorkspaceInfo();

}

void NetworkTestPage::onTestButtonClicked()
{
    QString input = m_testInputField->text();
    if (input.isEmpty()) {
        m_statusLabel->setText("Error: Input field is empty");
        m_statusLabel->setStyleSheet("color: #dc3545; font-weight: bold;");
    } else {
        m_statusLabel->setText(QString("✓ Tested input: \"%1\"").arg(input));
        m_statusLabel->setStyleSheet("color: #28a745; font-weight: bold;");
    }

}

void NetworkTestPage::onCreateTaskClicked()
{
    m_statusLabel->setText("✓ Created test task");
    m_statusLabel->setStyleSheet("color: #28a745; font-weight: bold;");
    // TODO: Implement actual task creation logic using WorkspaceRepository and update the status label accordingly
}

void NetworkTestPage::onCreateNoteClicked()
{
    m_statusLabel->setText("✓ Created test Note");
    m_statusLabel->setStyleSheet("color: #28a745; font-weight: bold;");

}

void NetworkTestPage::onDisplayWorkspaceInfo()
{
    QString info = QString(
        "Workspace ID: %1\n"
        "Current Time: %2\n"
        "Status: Ready for testing components"
    ).arg(workspaceId_.toString(QUuid::WithoutBraces), QDateTime::currentDateTime().toString(Qt::ISODate));

    m_infoLabel->setText(info);
}

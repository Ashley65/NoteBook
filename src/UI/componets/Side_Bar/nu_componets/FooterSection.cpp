//
// Created by DevAccount on 28/07/2026.
//
#include <UI/components/SIde_Bar/nu_componets/FooterSection.h>
#include <helpers/IconHelper.h>
#include <QToolButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QIcon>
#include <QMouseEvent>


nu_FooterSection::nu_FooterSection(QWidget* parent)
    : QWidget(parent)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);

    auto* mainLayout = new QVBoxLayout(this);
    // 2px padding around mainLayout
    mainLayout->setContentsMargins(6, 2, 2, 10);
    mainLayout->setSpacing(4);

    // 1. Top Divider Line
    auto* line = new QFrame(this);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Plain);
    line->setStyleSheet("background-color: rgba(255, 255, 255, 0.08); max-height: 1px; border: none; margin-bottom: 2px;");
    mainLayout->addWidget(line);

    // 2. Workspace Info Box (QFrame with zero extra platform padding)
    workspaceInfoBox = new QFrame(this);
    workspaceInfoBox->setObjectName("workspaceInfoBox");
    workspaceInfoBox->setFrameShape(QFrame::NoFrame);
    workspaceInfoBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    workspaceInfoBox->setCursor(Qt::PointingHandCursor);
    workspaceInfoBox->setAttribute(Qt::WA_Hover, true);
    workspaceInfoBox->installEventFilter(this);

    auto* boxLayout = new QVBoxLayout(workspaceInfoBox);
    boxLayout->setContentsMargins(8, 4, 8, 4);
    boxLayout->setSpacing(1);

    workspaceNameLabel = new QLabel("Main Workspace", this);
    workspaceNameLabel->setStyleSheet("color: #FFFFFF; font-weight: 600; font-size: 11.5px; background: transparent; border: none; padding: 0px; margin: 0px;");

    projectCountLabel = new QLabel("2 Projects  ·  2 Tasks", this);
    projectCountLabel->setStyleSheet("color: #8C92A4; font-size: 9.5px; font-weight: 400; background: transparent; border: none; padding: 0px; margin: 0px;");

    workspaceNameLabel->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    projectCountLabel->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    
    boxLayout->addWidget(workspaceNameLabel);
    boxLayout->addWidget(projectCountLabel);

    workspaceInfoBox->setStyleSheet(R"(
        QFrame#workspaceInfoBox {
            background-color: rgba(16, 22, 36, 0.7);
            border: 1px solid rgba(255, 255, 255, 0.08);
            border-radius: 6px;
            margin: 0px;
            padding: 0px;
        }
        QFrame#workspaceInfoBox:hover {
            background-color: rgba(30, 42, 65, 0.8);
            border-color: rgba(255, 255, 255, 0.15);
        }
    )");

    mainLayout->addWidget(workspaceInfoBox);

    // 2.1 Compact Workspace Button
    compactWsBtn = new QToolButton(this);
    compactWsBtn->setObjectName("CompactWsBtn");
    compactWsBtn->setCursor(Qt::PointingHandCursor);
    compactWsBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    compactWsBtn->setIcon(IconHelper::generateInitialsIcon("W", QColor("#6366F1"), Qt::white, 24));
    compactWsBtn->setIconSize(QSize(24, 24));
    compactWsBtn->setToolTip("Workspace Menu");
    compactWsBtn->setStyleSheet(R"(
        QToolButton#CompactWsBtn {
            background-color: rgba(16, 22, 36, 0.7);
            border: 1px solid rgba(255, 255, 255, 0.08);
            border-radius: 8px;
            padding: 6px 0px;
        }
        QToolButton#CompactWsBtn:hover {
            background-color: rgba(30, 42, 65, 0.8);
            border-color: rgba(255, 255, 255, 0.15);
        }
    )");
    compactWsBtn->setVisible(false);
    connect(compactWsBtn, &QToolButton::clicked, this, &nu_FooterSection::workspaceMenuRequested);
    mainLayout->addWidget(compactWsBtn);

    // 3. "Collapse Sidebar" Button
    modeBtn = new QToolButton(this);
    modeBtn->setCursor(Qt::PointingHandCursor);
    modeBtn->setObjectName("FooterModeBtn");
    modeBtn->setText("  Collapse Sidebar");

    modeBtn->setIcon(QIcon(":/icons/sidebar_collapse.svg"));
    modeBtn->setIconSize(QSize(13, 13));
    modeBtn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    modeBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    modeBtn->setStyleSheet(R"(
        QToolButton#FooterModeBtn {
            background-color: rgba(16, 22, 36, 0.5);
            color: #8C92A4;
            border: 1px solid rgba(255, 255, 255, 0.08);
            border-radius: 6px;
            padding: 4px 6px;
            font-size: 11px;
            font-weight: 500;
        }
        QToolButton#FooterModeBtn:hover {
            background-color: rgba(30, 42, 65, 0.8);
            color: #FFFFFF;
            border-color: rgba(255, 255, 255, 0.15);
        }
    )");

    connect(modeBtn, &QToolButton::clicked, this, &nu_FooterSection::modeCycleRequested);
    mainLayout->addWidget(modeBtn);
}

void nu_FooterSection::setMode(SideBarNamespace::Mode mode)
{
    if (m_currentMode == mode) return;
    m_currentMode = mode;

    bool isCompact = (mode == SideBarNamespace::Mode::Compact);

    // Toggle between expanded workspace info box and compact workspace avatar button
    workspaceInfoBox->setVisible(!isCompact);
    if (compactWsBtn) {
        compactWsBtn->setVisible(isCompact);
    }

    auto* lay = qobject_cast<QVBoxLayout*>(layout());
    if (lay) {
        lay->setContentsMargins(isCompact ? 8 : 6, 2, isCompact ? 8 : 2, 10);
    }

    if (isCompact) {
        modeBtn->setText("");
        modeBtn->setToolTip("Expand Sidebar");
        modeBtn->setStyleSheet(R"(
            QToolButton#FooterModeBtn {
                background-color: rgba(16, 22, 36, 0.5);
                color: #8C92A4;
                border: 1px solid rgba(255, 255, 255, 0.08);
                border-radius: 8px;
                padding: 6px 0px;
            }
            QToolButton#FooterModeBtn:hover {
                background-color: rgba(30, 42, 65, 0.8);
                color: #FFFFFF;
                border-color: rgba(255, 255, 255, 0.15);
            }
        )");
    } else {
        modeBtn->setText("  Collapse Sidebar");
        modeBtn->setToolTip("");
        modeBtn->setStyleSheet(R"(
            QToolButton#FooterModeBtn {
                background-color: rgba(16, 22, 36, 0.5);
                color: #8C92A4;
                border: 1px solid rgba(255, 255, 255, 0.08);
                border-radius: 6px;
                padding: 4px 6px;
                font-size: 11px;
                font-weight: 500;
            }
            QToolButton#FooterModeBtn:hover {
                background-color: rgba(30, 42, 65, 0.8);
                color: #FFFFFF;
                border-color: rgba(255, 255, 255, 0.15);
            }
        )");
    }
}


void nu_FooterSection::setAppStateController(AppStateController* controller)
{
    if (m_stateController == controller) return;
    if (m_stateController) {
        disconnect(m_stateController, &AppStateController::activeWorkspaceChanged, this, &nu_FooterSection::refreshData);
    }
    m_stateController = controller;
    if (m_stateController) {
        connect(m_stateController, &AppStateController::activeWorkspaceChanged, this, &nu_FooterSection::refreshData);
    }
    refreshData();
}

void nu_FooterSection::setWorkspaceRepository(WorkspaceRepository* repo)
{
    if (m_workspaceRepo == repo) return;
    if (m_workspaceRepo) {
        disconnect(m_workspaceRepo, &WorkspaceRepository::projectAdded, this, &nu_FooterSection::refreshData);
        disconnect(m_workspaceRepo, &WorkspaceRepository::taskAdded, this, &nu_FooterSection::refreshData);
        disconnect(m_workspaceRepo, &WorkspaceRepository::projectDeleted, this, &nu_FooterSection::refreshData);
        disconnect(m_workspaceRepo, &WorkspaceRepository::taskDeleted, this, &nu_FooterSection::refreshData);
    }
    m_workspaceRepo = repo;
    if (m_workspaceRepo) {
        connect(m_workspaceRepo, &WorkspaceRepository::projectAdded, this, &nu_FooterSection::refreshData);
        connect(m_workspaceRepo, &WorkspaceRepository::taskAdded, this, &nu_FooterSection::refreshData);
        connect(m_workspaceRepo, &WorkspaceRepository::projectDeleted, this, &nu_FooterSection::refreshData);
        connect(m_workspaceRepo, &WorkspaceRepository::taskDeleted, this, &nu_FooterSection::refreshData);
    }
    refreshData();
}



void nu_FooterSection::setWorkspaceName(QString workspaceName)
{

}

void nu_FooterSection::changeEvent(QEvent* event)
{
    QWidget::changeEvent(event);
}

bool nu_FooterSection::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == workspaceInfoBox && event->type() == QEvent::MouseButtonRelease) {
        auto* mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::LeftButton) {
            emit workspaceMenuRequested();
            return true;
        }
    }
    return QWidget::eventFilter(watched, event);
}

void nu_FooterSection::refreshData()
{
    if (!m_stateController || !m_workspaceRepo) return;

    QUuid workspaceId = m_stateController->context().activeWorkspaceId;
    Workspace workspace = m_workspaceRepo->getWorkspaceById(workspaceId);

    int projectCount = m_workspaceRepo->getProjectsByWorkspace(workspaceId).size();
    int taskCount = m_workspaceRepo->getTasksByWorkspace(workspaceId).size();

    workspaceNameLabel->setText(workspace.name);

    // Combine into a single label with middle dot separator
    projectCountLabel->setText(QString("%1 Projects  ·  %2 Tasks").arg(projectCount).arg(taskCount));

    if (compactWsBtn) {
        QString initials = workspace.name.isEmpty() ? "W" : workspace.name.left(2).toUpper();
        compactWsBtn->setIcon(IconHelper::generateInitialsIcon(initials, QColor("#6366F1"), Qt::white, 24));
        compactWsBtn->setToolTip(QString("%1\n%2 Projects · %3 Tasks\nClick for workspace menu").arg(workspace.name).arg(projectCount).arg(taskCount));
    }
}

//
// Created by DevAccount on 18/12/2025.
//
#include <UI/components/SIde_Bar/sideBar.h>

#include <QFrame>
#include <QVBoxLayout>
#include <QDebug>
#include <QMenu>
#include <QCursor>
#include <QPalette>
#include <QApplication>



SideBar::SideBar(AppStateController* stateController, WorkspaceRepository* repo, QWidget* parent)
    : QFrame(parent)
{
    setObjectName("SideBar");
    setFrameShape(QFrame::NoFrame);
    setStyleSheet("background: transparent; border: none;");

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    m_coreNavSection = new nu_CoreNavigationSection(this);
    connect(m_coreNavSection, &nu_CoreNavigationSection::itemSelected, this, [this](nu_CoreNavigationSection::Item item) {
        emit coreItemSelected(static_cast<int>(item));
    });
    connect(m_coreNavSection, &nu_CoreNavigationSection::itemColorSelected, this, &SideBar::navigationColorChanged);

    m_divider = new QFrame(this);
    m_divider->setFrameShape(QFrame::HLine);
    m_divider->setFrameShadow(QFrame::Plain);
    m_divider->setStyleSheet("background-color: rgba(255, 255, 255, 0.08); max-height: 1px; border: none; margin: 6px 12px;");

    m_projectsSection = new nu_ProjectsSection(this);
    connect(m_projectsSection, &nu_ProjectsSection::projectCreateRequested, this, &SideBar::projectCreateRequested);
    connect(m_projectsSection, &nu_ProjectsSection::projectSelected, this, &SideBar::onSwitchProject);

    m_footerSection = new nu_FooterSection(this);
    m_footerSection->setAppStateController(stateController);
    m_footerSection->setWorkspaceRepository(repo);
    connect(m_footerSection, &nu_FooterSection::modeCycleRequested, this, &SideBar::onToggleMode);
    connect(m_footerSection, &nu_FooterSection::workspaceMenuRequested, this, &SideBar::showWorkspaceMenu);

    root->addWidget(m_coreNavSection, 1);
    root->addWidget(m_divider, 0);
    root->addWidget(m_projectsSection, 0);
    root->addStretch(1);
    root->addWidget(m_footerSection, 0);

    applyMode();
}


void SideBar::setMode(Mode m) {
    if (m_mode == m) return;
    const bool wasCompact = (m_mode == Mode::Compact);
    m_mode = m;
    const bool isCompactNow = (m_mode == Mode::Compact);
    if (wasCompact != isCompactNow) {
        emit compactChanged();
    }
    applyMode();
}

void SideBar::setWorkspaceName(const QString& name) {
    if (m_workspaceName == name) return;
    m_workspaceName = name;
    emit workspaceNameChanged();
}

void SideBar::setActiveProjectId(const QUuid& id)
{
    if (m_activeProjectId == id) return;
    m_activeProjectId = id;
    if (m_projectsSection) {
        m_projectsSection->setActiveProjectId(id);
    }
    emit activeProjectIdChanged();
}

void SideBar::setProjects(const QVariantList& projects)
{
    if (m_projects == projects) return;
    m_projects = projects;
    if (m_projectsSection) {
        m_projectsSection->setProjects(projects);
    }
    emit projectsChanged();
}

void SideBar::onItemClicked(const QString& type, const QString& id) {
    qDebug() << "Item clicked:" << type << id;
    if (type == "core") {
        // Emit appropriate signal based on name/id
        emit coreItemSelected(0); // Example mapping
    } else if (type == "project") {
        const QUuid projectId(id);
        if (projectId.isNull()) {
            qWarning() << "Invalid project UUID:" << id;
            return;
        }
        setActiveProjectId(projectId);
        emit projectSelected(projectId);
    } else if (type == "filter") {
        emit filterSelected(id.toInt());
    }
}

void SideBar::onPrimaryClicked() {
    showWorkspaceMenu();
}

void SideBar::onToggleMode() {
    // Keep footer toggle reachable: cycle only between Default and Compact.
    if (m_mode == Mode::Compact) {
        setMode(Mode::Default);
    } else {
        setMode(Mode::Compact);
    }
}

void SideBar::onAddProject()
{
    emit projectCreateRequested();
}

void SideBar::onSwitchProject(const QUuid& projectId)
{
    const QUuid id(projectId);
    if (id.isNull()) {
        qWarning() << "Invalid project UUID:" << projectId;
        return;
    }
    setActiveProjectId(id);
    emit projectSelected(id);
}

void SideBar::applyMode() {
    const bool hidden = (m_mode == Mode::Hidden);
    setVisible(!hidden);

    const bool compact = (m_mode == Mode::Compact);

    if (hidden) {
        setFixedWidth(0);
    } else if (compact) {
        setFixedWidth(60);
    } else {
        setFixedWidth(260);
    }

    // Forward compact property change to core nav section
    if (m_coreNavSection) {
        m_coreNavSection->setCompact(compact);
    }

    if (m_projectsSection) {
        m_projectsSection->setCompact(compact);
    }

    if (m_footerSection) {
        m_footerSection->setMode(m_mode);
    }
}

void SideBar::updateWorkspaceMenuStyle() {
    if (!m_workspaceMenu) return;

    m_workspaceMenu->setStyleSheet(R"(
        QMenu#WorkspaceMenu {
            background-color: #141721;
            border: 1px solid rgba(255, 255, 255, 0.12);
            border-radius: 10px;
            padding: 6px;
        }
        QMenu#WorkspaceMenu::item {
            color: #E2E8F0;
            padding: 8px 20px 8px 10px;
            margin: 2px 0px;
            border-radius: 6px;
            font-size: 13px;
            font-weight: 500;
        }
        QMenu#WorkspaceMenu::icon {
            padding-left: 4px;
            padding-right: 8px;
        }
        QMenu#WorkspaceMenu::item:selected {
            background-color: rgba(99, 102, 241, 0.2);
            color: #FFFFFF;
        }
        QMenu#WorkspaceMenu::separator {
            height: 1px;
            margin: 4px 6px;
            background-color: rgba(255, 255, 255, 0.08);
        }
    )");
}

void SideBar::showWorkspaceMenu() {
    if (!m_workspaceMenu) {
        m_workspaceMenu = new QMenu(this);
        m_workspaceMenu->setObjectName("WorkspaceMenu");

        QAction* switchAction = m_workspaceMenu->addAction(QIcon(":/icons/switch_workspace.svg"), "Switch Workspace");
        QAction* createAction = m_workspaceMenu->addAction(QIcon(":/icons/create_workspace.svg"), "Create Workspace");
        QAction* deleteAction = m_workspaceMenu->addAction(QIcon(":/icons/delete-icon.svg"), "Delete Workspace");

        m_workspaceMenu->addSeparator();

        QAction* settingsAction = m_workspaceMenu->addAction(QIcon(":/icons/settings.svg"), "Workspace Settings");

        connect(switchAction, &QAction::triggered, this, &SideBar::onSwitchWorkspace);
        connect(createAction, &QAction::triggered, this, &SideBar::onCreateWorkspace);
        connect(deleteAction, &QAction::triggered, this, &SideBar::onDeleteWorkspace);
        connect(settingsAction, &QAction::triggered, this, &SideBar::onWorkspaceSettings);
    }

    updateWorkspaceMenuStyle();

    if (m_footerSection) {
        QPoint anchor = m_footerSection->mapToGlobal(QPoint(m_footerSection->width() + 4, m_footerSection->height() - 170));
        m_workspaceMenu->popup(anchor);
    } else {
        m_workspaceMenu->popup(QCursor::pos());
    }
}

void SideBar::onSwitchWorkspace() {
    qDebug() << "Switch workspace requested";
    emit workspaceSwitchRequested();
}

void SideBar::onCreateWorkspace() {
    qDebug() << "Create workspace requested";
    emit workspaceCreateRequested();
}

void SideBar::onDeleteWorkspace() {
    qDebug() << "Delete workspace requested";
    emit workspaceDeleteRequested();
}

void SideBar::onWorkspaceSettings() {
    qDebug() << "Workspace settings requested";
    emit workspaceSettingsRequested(m_workspaceId);
}

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


SideBar::SideBar(QWidget* parent)
    : QFrame(parent)
{
    setObjectName("SideBar");
    setFrameShape(QFrame::NoFrame);
    setStyleSheet("background: transparent; border: none;");

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);

    m_quickWidget = new QQuickWidget(this);
    m_quickWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_quickWidget->setAttribute(Qt::WA_AlwaysStackOnTop);
    m_quickWidget->setClearColor(Qt::transparent);

    // Link C++ to QML
    m_quickWidget->rootContext()->setContextProperty("sideBar", this);
    m_quickWidget->setSource(QUrl("qrc:/qml/SideBar.qml"));

    root->addWidget(m_quickWidget);

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

void SideBar::setProjects(const QVariantList& projects)
{
    if (m_projects == projects) return;
    m_projects = projects;
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
}

void SideBar::updateWorkspaceMenuStyle() {
     if (!m_workspaceMenu) return;

    const QColor win = qApp->palette().window().color();
    const bool darkMode = win.lightness() < 128;


    const QString css = darkMode ?
        R"(
            QMenu {
                background-color: #2B3038;
                border: 1px solid #444C58;
                border-radius: 8px;
                padding: 4px;
            }
            QMenu::item {
                color: #D0D5DD;
                padding: 7px 12px;
                border-radius: 6px;
            }
            QMenu::item:selected {
                background-color: #3C4655;
                color: #FFFFFF;
            }
            QMenu::separator {
                height: 1px;
                margin: 4px 8px;
                background: #444C58;
            }

        )"
        : R"(
            QMenu { background-color: #FFFFFF;
                border: 1px solid #D9DDE3;
                border-radius: 8px;
                padding: 4px;
            }
            QMenu::item {
                color: #1F2937;
                padding: 7px 12px;
                border-radius: 6px;
            }
            QMenu::item:selected {
                background-color: #E8F0FE;
                color: #0B57D0;
            }
            QMenu::separator {
                height: 1px;
                margin: 4px 8px;
                background: #E5E7EB;
            }
        )";
    m_workspaceMenu->setStyleSheet(css);
}

void SideBar::showWorkspaceMenu() {
    if (!m_workspaceMenu) {
        m_workspaceMenu = new QMenu(this);

        QAction* switchAction = m_workspaceMenu->addAction("Switch Workspace");
        QAction* createAction = m_workspaceMenu->addAction("Create Workspace");
        QAction* deleteAction = m_workspaceMenu->addAction("Delete Workspace");

        m_workspaceMenu->addSeparator();

        QAction* settingsAction = m_workspaceMenu->addAction("Workspace Settings");

        connect(switchAction, &QAction::triggered, this, &SideBar::onSwitchWorkspace);
        connect(createAction, &QAction::triggered, this, &SideBar::onCreateWorkspace);
        connect(deleteAction, &QAction::triggered, this, &SideBar::onDeleteWorkspace);
        connect(settingsAction, &QAction::triggered, this, &SideBar::onWorkspaceSettings);
    }

    updateWorkspaceMenuStyle();
    m_workspaceMenu->exec(QCursor::pos());
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

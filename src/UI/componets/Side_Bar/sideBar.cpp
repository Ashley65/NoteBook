//
// Created by DevAccount on 18/12/2025.
//
#include <UI/components/SIde_Bar/sideBar.h>

#include <QFrame>
#include <QVBoxLayout>
#include <QDebug>
#include <QMenu>
#include <QCursor>

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
    m_mode = m;
    emit compactChanged();
    applyMode();
}

void SideBar::setWorkspaceName(const QString& name) {
    if (m_workspaceName == name) return;
    m_workspaceName = name;
    emit workspaceNameChanged();
}

void SideBar::onItemClicked(const QString& type, const QString& id) {
    qDebug() << "Item clicked:" << type << id;
    if (type == "core") {
        // Emit appropriate signal based on name/id
        emit coreItemSelected(0); // Example mapping
    } else if (type == "project") {
        emit projectSelected(id.toInt());
    } else if (type == "filter") {
        emit filterSelected(id.toInt());
    }
}

void SideBar::onPrimaryClicked() {
    showWorkspaceMenu();
}

void SideBar::onToggleMode() {
    if (m_mode == Mode::Default) setMode(Mode::Compact);
    else if (m_mode == Mode::Compact) setMode(Mode::Hidden);
    else setMode(Mode::Default);
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
    emit workspaceSettingsRequested();
}

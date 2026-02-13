//
// Created by DevAccount on 18/12/2025.
//
#include <UI/components/SIde_Bar/sideBar.h>

#include <QFrame>
#include <QVBoxLayout>
#include <QDebug>

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
    emit workspaceSwitchRequested();
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

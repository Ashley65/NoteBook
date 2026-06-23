//
// Created by DevAccount on 26/05/2026.
//

#include "UI/components/Top_Menubar/TabBar.h"
#include <QVBoxLayout>

TabBar::TabBar(TabManager* tabManager, QWidget* parent)
    : QWidget(parent), m_tabManager(tabManager)
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_qmlWidget = new QQuickWidget(this);
    m_qmlWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);

    // allow the widget itself to be translucent
    m_qmlWidget->setAttribute(Qt::WA_TranslucentBackground);
    // ensure the QQuick rendering surface is transparent
    m_qmlWidget->setClearColor(Qt::transparent);
    // keep stylesheet transparent as well
    m_qmlWidget->setStyleSheet("background: transparent; border: none;");

    // Expose the Brain to the Visuals
    m_qmlWidget->rootContext()->setContextProperty("tabManager", m_tabManager);

    // Load the QML outline
    m_qmlWidget->setSource(QUrl("qrc:/qml/components/TabBar.qml"));

     if (m_qmlWidget->status() == QQuickWidget::Error) {
         const auto errs = m_qmlWidget->errors();
         for (const auto& err : errs) {
             qWarning() << err.toString();
         }
     }

    connect(m_tabManager, &TabManager::activeTabIdChanged, this, [this]()
    {
        onTabSelected(m_tabManager->activeTabId());
    });

     connect(m_tabManager, &TabManager::tabClosed, this, [this](const QString& viewType, const QUuid& contextId)
     {
         onTabClosed(contextId);
     });

    layout->addWidget(m_qmlWidget);
}

void TabBar::setActiveTab(const QUuid& contextId)
{
    if (m_tabManager) {
        m_tabManager->setActiveTabId(contextId);
    }
}

void TabBar::onTabSelected(const QUuid& contextId)
{
    emit tabSelected(contextId);
}

void TabBar::onTabClosed(const QUuid& contextId)
{
    emit tabClosed(contextId);
}

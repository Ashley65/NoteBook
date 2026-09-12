//
// Created by DevAccount on 24/05/2026.
//

#include "Data/workspace/Manager/TabManager.h"

#include <QVariantMap>

TabManager::TabManager(QObject* parent)
    : QObject(parent),
    m_activeTabId(QUuid())
{

}

QVariantList TabManager::tabs() const
{
    QVariantList list;
    for (const auto& tab : m_tabs) {
        QVariantMap map;
        map["title"] = tab.title;
        map["viewType"] = tab.viewType;
        map["contextId"] = tab.contextId;
        map["projectColour"] = tab.projectColour;
        list.append(map);
    }
    return list;
}

int TabManager::findTabIndexByContextId(const QUuid& contextId) const
{
    for (int i = 0; i < m_tabs.size(); ++i) {
        if (m_tabs[i].contextId == contextId) {
            return i;
        }
    }
    return -1;
}

void TabManager::addTab(const QString& title, const QString& viewType, const QUuid& contextId,
    const QString& projectColour)
{
    int existingTabIndex = findTabIndexByContextId(contextId);

    if (existingTabIndex != -1) {
        // Tab already exists, just activate it
        setActiveTabId(contextId);
        return;
    }

    bool exists = false;
    for (const auto& tab : m_tabs) {
        if (tab.contextId == contextId) {
            exists = true;
            break;
        }
    }

    if (!exists) {
        TabData newTab;
        newTab.title = title;
        newTab.viewType = viewType;
        newTab.contextId = contextId;
        newTab.projectColour = projectColour;
        newTab.lastAccessed = QDateTime::currentDateTime();
        newTab.history.append({title, viewType, contextId, projectColour});
        newTab.historyIndex = 0;

        m_tabs.append(newTab);
        emit tabsChanged();
    }

    setActiveTabId(contextId);
    discardOldTabs(5); // Keep at most 5 heavy views active
}

void TabManager::navigateActiveTab(const QString& title, const QString& viewType, const QUuid& contextId,
    const QString& projectColour)
{
    const int activeIndex = findTabIndexByContextId(m_activeTabId);
    const int existingIndex = findTabIndexByContextId(contextId);

    // If this context is already open in a DIFFERENT tab, switch to it
    if (existingIndex != -1 && existingIndex != activeIndex) {
        m_tabs[existingIndex].title = title;
        m_tabs[existingIndex].projectColour = projectColour;
        emit tabsChanged();
        setActiveTabId(contextId);
        return;
    }

    if (activeIndex != -1) {
        auto& currentTab = m_tabs[activeIndex];

        // Check if navigating to the same state as current history entry
        bool isSameState = (currentTab.historyIndex >= 0 &&
                            currentTab.historyIndex < currentTab.history.size() &&
                            currentTab.history[currentTab.historyIndex].viewType == viewType &&
                            currentTab.history[currentTab.historyIndex].contextId == contextId);

        if (!isSameState) {
            // Truncate any forward history
            while (currentTab.history.size() > currentTab.historyIndex + 1) {
                currentTab.history.removeLast();
            }
            currentTab.history.append({title, viewType, contextId, projectColour});
            currentTab.historyIndex = currentTab.history.size() - 1;
        } else {
            currentTab.history[currentTab.historyIndex].title = title;
            currentTab.history[currentTab.historyIndex].projectColour = projectColour;
        }

        // Reuse and update the current active tab
        currentTab.title = title;
        currentTab.viewType = viewType;
        currentTab.contextId = contextId;
        currentTab.projectColour = projectColour;
        currentTab.lastAccessed = QDateTime::currentDateTime();

        m_activeTabId = contextId;
        emit tabsChanged();
        emit activeTabIdChanged();
        emit tabOpened(viewType, contextId);
        emit navigationHistoryChanged(canGoBack(), canGoForward());
        discardOldTabs(5);
    } else {
        // No active tab exists yet, fallback to creating one
        addTab(title, viewType, contextId, projectColour);
    }
}

bool TabManager::canGoBack() const
{
    const int idx = findTabIndexByContextId(m_activeTabId);
    if (idx < 0 || idx >= m_tabs.size()) return false;
    return m_tabs[idx].historyIndex > 0;
}

bool TabManager::canGoForward() const
{
    const int idx = findTabIndexByContextId(m_activeTabId);
    if (idx < 0 || idx >= m_tabs.size()) return false;
    return m_tabs[idx].historyIndex + 1 < m_tabs[idx].history.size();
}

void TabManager::goBack()
{
    const int idx = findTabIndexByContextId(m_activeTabId);
    if (idx < 0 || idx >= m_tabs.size()) return;

    auto& tab = m_tabs[idx];
    if (tab.historyIndex > 0) {
        tab.historyIndex--;
        const auto& entry = tab.history[tab.historyIndex];

        tab.title = entry.title;
        tab.viewType = entry.viewType;
        tab.contextId = entry.contextId;
        tab.projectColour = entry.projectColour;
        tab.lastAccessed = QDateTime::currentDateTime();

        m_activeTabId = entry.contextId;
        emit tabsChanged();
        emit activeTabIdChanged();
        emit tabOpened(entry.viewType, entry.contextId);
        emit navigationHistoryChanged(canGoBack(), canGoForward());
    }
}

void TabManager::goForward()
{
    const int idx = findTabIndexByContextId(m_activeTabId);
    if (idx < 0 || idx >= m_tabs.size()) return;

    auto& tab = m_tabs[idx];
    if (tab.historyIndex + 1 < tab.history.size()) {
        tab.historyIndex++;
        const auto& entry = tab.history[tab.historyIndex];

        tab.title = entry.title;
        tab.viewType = entry.viewType;
        tab.contextId = entry.contextId;
        tab.projectColour = entry.projectColour;
        tab.lastAccessed = QDateTime::currentDateTime();

        m_activeTabId = entry.contextId;
        emit tabsChanged();
        emit activeTabIdChanged();
        emit tabOpened(entry.viewType, entry.contextId);
        emit navigationHistoryChanged(canGoBack(), canGoForward());
    }
}

void TabManager::openNewTab()
{
    const QUuid newId = QUuid::createUuid();
    addTab(tr("New Tab"), "Dashboard", newId, "#3B82F6");
}

void TabManager::updateTabTitle(const QUuid& contextId, const QString& newTitle)
{
    const int index = findTabIndexByContextId(contextId);
    if (index != -1 && m_tabs[index].title != newTitle) {
        m_tabs[index].title = newTitle;
        if (m_tabs[index].historyIndex >= 0 && m_tabs[index].historyIndex < m_tabs[index].history.size()) {
            m_tabs[index].history[m_tabs[index].historyIndex].title = newTitle;
        }
        emit tabsChanged();
    }
}

void TabManager::closeTab(const QUuid& contextId)
{
    for (int i = 0; i < m_tabs.size(); ++i) {
        if (m_tabs[i].contextId == contextId) {
            QString viewType = m_tabs[i].viewType;
            m_tabs.removeAt(i);
            emit tabsChanged();
            emit tabClosed(viewType, contextId);

            if (m_activeTabId == contextId) {
                if (m_tabs.isEmpty()) {
                    setActiveTabId(QUuid());
                } else {
                    // Switch to the adjacent tab
                    int nextIndex = qMin(i, m_tabs.size() - 1);
                    setActiveTabId(m_tabs[nextIndex].contextId);
                }
            } else {
                emit navigationHistoryChanged(canGoBack(), canGoForward());
            }
            break;
        }
    }
}

void TabManager::setActiveTabId(const QUuid& contextId)
{
    bool isSame = (m_activeTabId == contextId);
    
    m_activeTabId = contextId;
    emit activeTabIdChanged();

    if (!m_activeTabId.isNull()) {
        for (auto& tab : m_tabs) {
            if (tab.contextId == m_activeTabId) {
                tab.lastAccessed = QDateTime::currentDateTime();
                if (!isSame) {
                    emit tabOpened(tab.viewType, tab.contextId);
                }
                break;
            }
        }
    }
    emit navigationHistoryChanged(canGoBack(), canGoForward());
    discardOldTabs(5);
}

void TabManager::discardOldTabs(int maxActiveTabs)
{
    if (m_tabs.size() <= maxActiveTabs) return;

    // Create a list of indices sorted by lastAccessed (newest first)
    QList<int> indices(m_tabs.size());
    for (int i = 0; i < m_tabs.size(); ++i) indices[i] = i;

    std::sort(indices.begin(), indices.end(), [this](int a, int b) {
        return m_tabs[a].lastAccessed > m_tabs[b].lastAccessed;
    });

    // Mark tabs beyond maxActiveTabs for discarding
    // Note: Never discard the active tab
    int activeCount = 0;
    for (int idx : indices) {
        if (m_tabs[idx].contextId == m_activeTabId) {
            activeCount++;
            continue;
        }

        if (activeCount < maxActiveTabs) {
            activeCount++;
        } else {
            emit tabDiscarded(m_tabs[idx].contextId);
        }
    }
}

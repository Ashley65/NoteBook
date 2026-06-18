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
    int existingTabINdex = findTabIndexByContextId(contextId);

    if (existingTabINdex != -1) {
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
        m_tabs.append({title, viewType, contextId, projectColour, QDateTime::currentDateTime()});
        emit tabsChanged();
    }

    setActiveTabId(contextId);
    discardOldTabs(5); // Keep at most 5 heavy views active
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

//
// Created by DevAccount on 24/05/2026.
//

#ifndef TASKHELPER_TABMANAGER_H
#define TASKHELPER_TABMANAGER_H
#pragma once
#include <QObject>
#include <QVariantList>
#include <QUuid>
#include <QDateTime>

/**
 * @file TabManager.h
 * @brief Navigation tab controller and lifecycle manager for NoteBook's UI.
 */

/**
 * @struct NavigationEntry
 * @brief Represents a single navigation state in a tab's history.
 */
struct NavigationEntry
{
    QString title;          ///< Display title.
    QString viewType;       ///< View identifier (e.g., "Note", "TaskBoard", "Project", "Home").
    QUuid contextId;        ///< Target entity UUID.
    QString projectColour;  ///< Accent color hex string.
};

/**
 * @struct TabData
 * @brief State descriptor representing an individual workspace or document tab.
 */
struct TabData
{
    QString title;                  ///< Display title of the tab.
    QString viewType;               ///< View identifier (e.g., "Note", "TaskBoard", "Project", "Home").
    QUuid contextId;                ///< Target entity UUID (note, project, or workspace).
    QString projectColour;          ///< Hex color string associated with the tab context.
    QDateTime lastAccessed;         ///< Timestamp when the tab was last viewed or focused.
    QList<NavigationEntry> history; ///< Navigation history stack for this tab.
    int historyIndex = -1;          ///< Current position in the tab's history stack.
};

/**
 * @class TabManager
 * @brief Manages open document and view tabs, active tab focus, and LRU tab discarding.
 *
 * TabManager exposes Q_PROPERTY bindings and Q_INVOKABLE methods to both C++ and QML
 * for opening new views, switching between active tabs, updating tab titles dynamically,
 * and pruning inactive tabs beyond configurable memory limits.
 */
class TabManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantList tabs READ tabs NOTIFY tabsChanged)
    Q_PROPERTY(QUuid activeTabId READ activeTabId WRITE setActiveTabId NOTIFY activeTabIdChanged)

public:
    /**
     * @brief Constructs the TabManager.
     * @param parent Optional parent QObject.
     */
    explicit TabManager(QObject* parent = nullptr);

    /**
     * @brief Returns the list of open tabs formatted as QVariantMap items for QML consumers.
     */
    [[nodiscard]] QVariantList tabs() const;

    /**
     * @brief Returns the context ID of the currently active tab.
     */
    [[nodiscard]] QUuid activeTabId() const { return m_activeTabId; }

    /**
     * @brief Opens a new tab or switches to an existing tab matching contextId.
     * @param title Title to display on the tab header.
     * @param viewType Target view page type (e.g. "Note", "Project", "TaskBoard").
     * @param contextId Identifier of the entity being opened.
     * @param projectColour Hex color representation for the project accent.
     */
    Q_INVOKABLE void addTab(const QString& title, const QString& viewType, const QUuid& contextId, const QString& projectColour);

    /**
     * @brief Replaces the view in the currently active tab.
     * @param title New title.
     * @param viewType New view type.
     * @param contextId New target entity ID.
     * @param projectColour Accent color.
     */
    Q_INVOKABLE void navigateActiveTab(const QString& title, const QString& viewType, const QUuid& contextId, const QString& projectColour);

    /**
     * @brief Closes the tab corresponding to the specified context ID.
     * @param contextId Entity identifier of the tab to close.
     */
    Q_INVOKABLE void closeTab(const QUuid& contextId);

    /**
     * @brief Sets the currently active tab by its context ID.
     * @param contextId Entity identifier to focus.
     */
    Q_INVOKABLE void setActiveTabId(const QUuid& contextId);

    /**
     * @brief Discards least-recently-accessed background tabs if total count exceeds the limit.
     * @param maxActiveTabs Maximum allowable active tabs (default: 10).
     */
    Q_INVOKABLE void discardOldTabs(int maxActiveTabs = 10);

    /**
     * @brief Opens a default blank home or workspace tab.
     */
    Q_INVOKABLE void openNewTab();

    /**
     * @brief Dynamically updates the header title of a specific tab.
     * @param contextId Entity identifier of the target tab.
     * @param newTitle Revised title string.
     */
    Q_INVOKABLE void updateTabTitle(const QUuid& contextId, const QString& newTitle);

    /**
     * @brief Helper for QML link navigation using stringified UUIDs.
     * @param title Title of the target page.
     * @param viewType View type string.
     * @param contextIdStr Stringified UUID of the context entity.
     */
    Q_INVOKABLE void openLink(const QString& title, const QString& viewType, const QString& contextIdStr)
    {
        QUuid id = QUuid::fromString(contextIdStr);
        addTab(title, viewType, id, "#3B82F6");
    }

    /**
     * @brief Checks if the currently active tab has backward navigation history.
     */
    [[nodiscard]] bool canGoBack() const;

    /**
     * @brief Checks if the currently active tab has forward navigation history.
     */
    [[nodiscard]] bool canGoForward() const;

    /**
     * @brief Navigates backward in the active tab's history.
     */
    Q_INVOKABLE void goBack();

    /**
     * @brief Navigates forward in the active tab's history.
     */
    Q_INVOKABLE void goForward();

    /**
     * @brief Finds the index of a tab by its context ID.
     * @param contextId Target entity UUID.
     * @return 0-based index if found, -1 otherwise.
     */
    [[nodiscard]] int findTabIndexByContextId(const QUuid& contextId) const;

signals:
    /**
     * @brief Emitted when the active tab's back/forward capability changes.
     */
    void navigationHistoryChanged(bool canGoBack, bool canGoForward);
    /**
     * @brief Emitted when tabs are added, removed, or modified.
     */
    void tabsChanged();

    /**
     * @brief Emitted when active tab selection changes.
     */
    void activeTabIdChanged();

    /**
     * @brief Emitted when a tab is created or focused.
     * @param viewType View type string.
     * @param contextId Target entity UUID.
     */
    void tabOpened(const QString& viewType, const QUuid& contextId);

    /**
     * @brief Emitted when a tab is explicitly closed by the user.
     * @param viewType View type string.
     * @param contextId Target entity UUID.
     */
    void tabClosed(const QString& viewType, const QUuid& contextId);

    /**
     * @brief Emitted when a tab is discarded to save memory.
     * @param contextId Target entity UUID.
     */
    void tabDiscarded(const QUuid& contextId);

private:
    QList<TabData> m_tabs;
    QUuid m_activeTabId;
};

#endif //TASKHELPER_TABMANAGER_H

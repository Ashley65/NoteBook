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


struct TabData
{
    QString title;
    QString viewType;
    QUuid contextId;
    QString projectColour;
    QDateTime lastAccessed;
};


class TabManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY( QVariantList tabs READ tabs NOTIFY tabsChanged)
    Q_PROPERTY( QUuid activeTabId READ activeTabId WRITE setActiveTabId NOTIFY activeTabIdChanged)

public:
    explicit TabManager(QObject* parent = nullptr);
    [[nodiscard]] QVariantList tabs() const;

    [[nodiscard]] QUuid activeTabId() const { return m_activeTabId; }

    Q_INVOKABLE void addTab(const QString& title, const QString& viewType, const QUuid& contextId, const QString& projectColour);
    Q_INVOKABLE void closeTab(const QUuid& contextId);
    Q_INVOKABLE void setActiveTabId(const QUuid& contextId);
    Q_INVOKABLE void discardOldTabs(int maxActiveTabs = 10);
    Q_INVOKABLE void openLink(const QString& title, const QString& viewType, const QString& contextIdStr)
    {
        QUuid id = QUuid::fromString(contextIdStr);
        addTab(title, viewType, id, "#3B82F6");
    }

    [[nodiscard]] int findTabIndexByContextId(const QUuid& contextId) const;


signals:
    void tabsChanged();
    void activeTabIdChanged();
    void tabOpened( const QString& viewType, const QUuid& contextId);
    void tabClosed( const QString& viewType, const QUuid& contextId);
    void tabDiscarded(const QUuid& contextId);

private:
    QList<TabData> m_tabs;
    QUuid m_activeTabId;
};
#endif //TASKHELPER_TABMANAGER_H

//
// Created by DevAccount on 24/05/2026.
//

#ifndef TASKHELPER_TABMANAGER_H
#define TASKHELPER_TABMANAGER_H
#pragma once
#include <QObject>
#include <QVariantList>
#include <QUuid>


struct TabData
{
    QString title;
    QString viewType;
    QUuid contextId;
    QString projectColour;
};


class TabManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY( QVariantList tabs READ tabs NOTIFY tabsChanged)
    Q_PROPERTY( int activeTabIndex READ activeIndex WRITE setActiveIndex NOTIFY activeIndexChanged)

public:
    explicit TabManager(QObject* parent = nullptr);
    [[nodiscard]] QVariantList tabs() const;

    [[nodiscard]] int activeIndex() const { return m_activeTabIndex; }

    Q_INVOKABLE void addTab(const QString& title, const QString& viewType, const QUuid& contextId, const QString& projectColour);
    Q_INVOKABLE void closeTab(int index);
    Q_INVOKABLE void setActiveIndex(int index);

signals:
    void tabsChanged();
    void activeIndexChanged();
    void tabOpened( const QString& viewType, const QUuid& contextId);
    void tabClosed( const QString& viewType, const QUuid& contextId);

private:
    QList<TabData> m_tabs;
    int m_activeTabIndex = -1;
};
#endif //TASKHELPER_TABMANAGER_H

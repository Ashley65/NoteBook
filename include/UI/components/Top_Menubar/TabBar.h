//
// Created by DevAccount on 23/05/2026.
//

#ifndef TASKHELPER_TABBAR_H
#define TASKHELPER_TABBAR_H
#pragma once
#include <QWidget>
#include <QQuickWidget>
#include <QQmlContext>
#include <QUuid>

#include "Data/workspace/Manager/TabManager.h"



class TabBar : public QWidget
{
    Q_OBJECT

public:
    explicit TabBar(TabManager* tabManager, QWidget* parent = nullptr);

    void setActiveTab(const QUuid& contextId);

signals:
    void tabSelected(const QUuid& contextId);
    void tabClosed(const QUuid& contextId);

private slots:
    void onTabSelected(const QUuid& contextId);
    void onTabClosed(const QUuid& contextId);

private:
    TabManager* m_tabManager;
    QQuickWidget* m_qmlWidget;




};


#endif //TASKHELPER_TABBAR_H

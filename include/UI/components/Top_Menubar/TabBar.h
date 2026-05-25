//
// Created by DevAccount on 23/05/2026.
//

#ifndef TASKHELPER_TABBAR_H
#define TASKHELPER_TABBAR_H
#pragma once
#include <QWidget>
#include <QQuickWidget>

#include "Data/workspace/Manager/TabManager.h"



class TabBar : public QWidget
{
    Q_OBJECT

public:
    explicit TabBar(TabManager* tabManager, QWidget* parent = nullptr);


};


#endif //TASKHELPER_TABBAR_H

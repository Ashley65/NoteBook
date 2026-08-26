//
// Created by DevAccount on 28/07/2026.
//

#ifndef TASKHELPER_CORENAVIGATIONSECTION_H
#define TASKHELPER_CORENAVIGATIONSECTION_H
#pragma once

#include <QLabel>
#include <QWidget>
#include "BaseNavTree.h"

class nu_CoreNavigationSection : public QWidget
{
    Q_OBJECT

public:
    enum class Item
    {
        Dashboard,
        Projects,
        TaskBoard,
        Notes,
        AI_Assistant
    };
    
    explicit nu_CoreNavigationSection(QWidget *parent = nullptr);
    void setActiveItem(Item item);
    void setCompact(bool compact);
    bool isCompact() const { return m_isCompact; }
    static QString colorForItem(Item item);

signals:
    void itemSelected(Item item);
    void itemColorSelected(const QString& colorHex);

protected:
    void changeEvent(QEvent *event) override;

private:
    void updateStyles();
    QLabel* navTitle { nullptr };

    nu_BaseNavTree* navTree { nullptr };
    bool m_isCompact { false };
};
#endif //TASKHELPER_CORENAVIGATIONSECTION_H

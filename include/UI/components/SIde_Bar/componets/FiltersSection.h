//
// Created by DevAccount on 18/12/2025.
//

#ifndef TASKHELPER_FILTERSSECTION_H
#define TASKHELPER_FILTERSSECTION_H
#pragma once
#include <QLabel>
#include <QWidget>
#include <QListWidget>
#include <QDynamicPropertyChangeEvent>

#include "BaseNavTree.h"

class FiltersSection : public BaseNavTree
{
    Q_OBJECT
public:
    explicit FiltersSection(QWidget *parent = nullptr);

    void setCompact(bool compact);
protected:
    void changeEvent(QEvent *event) override;
    signals:
        void filterSelected(int filterId);
private:
    QLabel* headerLabel{};
    static auto addFilter(QTreeWidgetItem* parent, const QString& name, int id, int count = 0) -> void;
    QTreeWidget* treeList;

};

#endif //TASKHELPER_FILTERSSECTION_H
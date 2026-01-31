//
// Created by DevAccount on 18/12/2025.
//

#ifndef TASKHELPER_PROJECTSSECTION_H
#define TASKHELPER_PROJECTSSECTION_H
#pragma once

#include <QLabel>
#include <QWidget>
#include <QTreeWidget>
#include <QDynamicPropertyChangeEvent>

class ProjectsSection : public QWidget
{
    Q_OBJECT
public:
    explicit ProjectsSection(QWidget *parent = nullptr);
    void setCompact(bool compact);

    signals:
        void projectSelected(QString projectName);

protected:
    void changeEvent(QEvent *event) override; // For compact mode logic

private:
    QTreeWidget* treeList;
    static void addProject(QTreeWidgetItem* parent, const QString& name, int count = 0);
};




#endif //TASKHELPER_PROJECTSSECTION_H
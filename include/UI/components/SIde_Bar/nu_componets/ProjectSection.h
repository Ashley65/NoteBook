//
// Created by DevAccount on 15/08/2026.
//

#ifndef TASKHELPER_PROJECTSECTION_H
#define TASKHELPER_PROJECTSECTION_H
#pragma once

#include <QLabel>
#include <QWidget>
#include <QToolButton>
#include <QTreeWidget>
#include <QDynamicPropertyChangeEvent>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QUuid>
#include <QVariantList>

#include "BaseNavTree.h"

class nu_ProjectsSection : public QWidget
{
    Q_OBJECT
public:
    explicit nu_ProjectsSection(QWidget* parent = nullptr);
    void setCompact(bool compact);
    bool isCompact() const { return m_isCompact; }
    void setProjects(const QVariantList& projects);
    void setActiveProjectId(const QUuid& projectId);

signals:
    void projectSelected(const QUuid& projectId);
    void projectCreateRequested();

protected:
    void changeEvent(QEvent* event) override;

private:
    nu_BaseNavTree* treeList { nullptr };
    bool m_isCompact { false };

    QVBoxLayout* projectLayout { nullptr };
    QHBoxLayout* projectHLayout { nullptr };
    QLabel* projectTitle { nullptr };
    QToolButton* projectAddButton { nullptr };

    QVariantList m_projects;
    QUuid m_activeProjectId;
};
#endif //TASKHELPER_PROJECTSECTION_H

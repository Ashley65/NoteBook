//
// Created by DevAccount on 18/12/2025.
//

#ifndef TASKHELPER_SIDEBAR_H
#define TASKHELPER_SIDEBAR_H
#pragma once
#include <QFrame>
#include <QPushButton>
#include <QListWidget>
#include <QTreeWidget>
#include <QLabel>

class WorkspaceContextSection;
class CoreNavigationSection;
class ProjectsSection;
class FiltersSection;
class FooterSection;



class SideBar : public QFrame
{
    Q_OBJECT
public:
    explicit SideBar(QWidget *parent = nullptr);
    enum class Mode
    {
        Default,
        Compact,
        Hidden
    };

    Mode mode() const { return m_mode; }
    void setMode(const Mode newMode);


    // Accessors for wiring signals in MainWindow
    CoreNavigationSection* coreNav() const { return m_core; }
    WorkspaceContextSection* primary() const { return m_primary; }
    ProjectsSection* projects() const { return m_projects; }
    FiltersSection* filters() const { return m_filters; }
    FooterSection* footer() const { return m_footer; }


signals:
    void primaryTriggered();
    void coreItemSelected(int item /* map to enum in section */);
    void projectSelected(int projectId);
    void filterSelected(int filterId);


private:

    void applyMode();
    Mode m_mode { Mode::Default };

    CoreNavigationSection* m_core { nullptr };
    WorkspaceContextSection* m_primary { nullptr };
    ProjectsSection* m_projects { nullptr };
    FiltersSection* m_filters { nullptr };
    FooterSection* m_footer { nullptr };

};


#endif //TASKHELPER_SIDEBAR_H
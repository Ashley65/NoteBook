//
// Created by DevAccount on 18/12/2025.
//

#ifndef TASKHELPER_SIDEBAR_H
#define TASKHELPER_SIDEBAR_H
#pragma once
#include <QFrame>
#include <QQuickWidget>
#include <QQmlContext>
#include <QMenu>
#include <QUuid>
#include <QVariantList>

#include "SideBarMode.h"
#include <UI/components/SIde_Bar/nu_componets/CoreNavigationSection.h>
#include <UI/components/SIde_Bar/nu_componets/FooterSection.h>
#include <UI/components/SIde_Bar/nu_componets/ProjectSection.h>


class AppStateController;
class WorkspaceRepository;

class SideBar : public QFrame
{
    Q_OBJECT
    Q_PROPERTY(bool isCompact READ isCompact WRITE setIsCompact NOTIFY compactChanged)
    Q_PROPERTY(QString workspaceName READ workspaceName WRITE setWorkspaceName NOTIFY workspaceNameChanged)
    Q_PROPERTY(QUuid workspaceId READ workspaceId WRITE setWorkspaceId NOTIFY workspaceIdChanged)
    Q_PROPERTY(QVariantList projects READ projects NOTIFY projectsChanged)
    Q_PROPERTY(QUuid activeProjectId READ activeProjectId WRITE setActiveProjectId NOTIFY activeProjectIdChanged)
public:
    explicit SideBar(AppStateController* stateController, WorkspaceRepository* repo, QWidget* parent = nullptr);
    using Mode = SideBarNamespace::Mode;

    Mode mode() const { return m_mode; }
    void setMode(const Mode newMode);

    bool isCompact() const { return m_mode == Mode::Compact; }
    void setIsCompact(bool compact) { setMode(compact ? Mode::Compact : Mode::Default); }

    QString workspaceName() const { return m_workspaceName; }
    void setWorkspaceName(const QString& name);
    QUuid workspaceId() const { return m_workspaceId; }
    QUuid activeProjectId() const { return m_activeProjectId; }
    void setWorkspaceId(const QUuid& id) { if (m_workspaceId == id) return; m_workspaceId = id; emit workspaceIdChanged(); }
    void setActiveProjectId(const QUuid& id);
    QVariantList projects() const { return m_projects; }
    void setProjects(const QVariantList& projects);

    // QML Interface
    Q_INVOKABLE void onItemClicked(const QString& type, const QString& id);
    Q_INVOKABLE void onPrimaryClicked();
    Q_INVOKABLE void onToggleMode();
    Q_INVOKABLE void onAddProject();
    Q_INVOKABLE void onSwitchProject(const QUuid& projectId);

signals:
    void primaryTriggered();
    void workspaceSwitchRequested();
    void workspaceCreateRequested();
    void workspaceDeleteRequested();
    void workspaceSettingsRequested(const QUuid& workspaceId);
    void coreItemSelected(int item /* map to enum in section */);
    void navigationColorChanged(const QString& colorHex);
    void projectSelected(const QUuid& projectId);
    void activeProjectIdChanged();
    void projectCreateRequested();
    void filterSelected(int filterId);
    void compactChanged();
    void workspaceNameChanged();
    void workspaceIdChanged();
    void projectsChanged();


private slots:
    void onSwitchWorkspace();
    void onCreateWorkspace();
    void onDeleteWorkspace();
    void onWorkspaceSettings();

private:
    void applyMode();
    void updateWorkspaceMenuStyle();
    void showWorkspaceMenu();
    Mode m_mode { Mode::Default };
    QString m_workspaceName { "Personal Workspace" };
    QUuid m_workspaceId;
    QUuid m_activeProjectId;
    QVariantList m_projects;
    QMenu* m_workspaceMenu { nullptr };

    nu_CoreNavigationSection* m_coreNavSection { nullptr };
    QFrame* m_divider { nullptr };
    nu_ProjectsSection* m_projectsSection { nullptr };
    nu_FooterSection* m_footerSection { nullptr };
};


#endif //TASKHELPER_SIDEBAR_H
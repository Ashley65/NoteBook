//
// Created by DevAccount on 28/07/2026.
//

#ifndef TASKHELPER_FOOTERSECTION_H
#define TASKHELPER_FOOTERSECTION_H
#pragma once

#include <QWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QLabel>
#include <QEvent>
#include <QFrame>
#include <QToolButton>

#include "UI/components/SIde_Bar/SideBarMode.h"
#include "helpers/AppStateController.h"
#include "Data/workspace/WorkspaceRepository.h"
#include "helpers/Workspace.h"
class SideBar;

class nu_FooterSection : public QWidget
{
    Q_OBJECT

public:
    explicit nu_FooterSection(QWidget* parent = nullptr);
    void setMode(SideBarNamespace::Mode mode);
    void setWorkspaceName(QString workspaceName);
    
    // Dependency injection
    void setAppStateController(AppStateController* controller);
    void setWorkspaceRepository(WorkspaceRepository* repo);

signals:
    void modeCycleRequested();
    void workspaceMenuRequested();

protected:
    void changeEvent(QEvent* event) override;
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    void refreshData();

    QToolButton* modeBtn;
    QToolButton* compactWsBtn { nullptr };
    SideBarNamespace::Mode m_currentMode { SideBarNamespace::Mode::Default };

    // workspace info Box
    QFrame* workspaceInfoBox;
    QLabel* workspaceNameLabel;
    QLabel* projectCountLabel;
    QLabel* taskCountLabel{};
    
    QString currentWorkspaceName_;
    QLayout* projectAmount{};
    AppStateController* m_stateController {nullptr};
    WorkspaceRepository* m_workspaceRepo {nullptr};

};


#endif //TASKHELPER_FOOTERSECTION_H

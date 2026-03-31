//
// Created by DevAccount on 22/03/2026.
//

#ifndef TASKHELPER_NETWORKTESTPAGE_H
#define TASKHELPER_NETWORKTESTPAGE_H
#pragma once
#include "../IWorkspaceView.h"
#include "helpers/Workspace.h"
#include <QLabel>
#include <QLineEdit>
#include <QUuid>


class NetworkTestPage : public IWorkspaceView
{
    Q_OBJECT
public:

    explicit NetworkTestPage(const Workspace& ws, QWidget* parent = nullptr);

    // IWorkspaceView interface
    void refresh() override;
    void updateWorkspace(const Workspace& ws) override;


private slots:
    void onTestButtonClicked();
    void onCreateTaskClicked();
    void onCreateNoteClicked();
    void onDisplayWorkspaceInfo();


private:
    QUuid workspaceId_;
    QLineEdit* m_testInputField {nullptr};
    QLabel* m_statusLabel {nullptr};
    QLabel* m_infoLabel {nullptr};

    void setupUi();

};

#endif //TASKHELPER_NETWORKTESTPAGE_H

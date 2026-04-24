//
// Created by DevAccount on 21/04/2026.
//

#ifndef TASKHELPER_PROJECTCREATEDIALOG_H
#define TASKHELPER_PROJECTCREATEDIALOG_H
#pragma once

#include <QLineEdit>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QDialog>

#include "Data/workspace/Structure/Project.h"
#include "Data/workspace/WorkspaceRepository.h"


class ProjectCreateDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ProjectCreateDialog(const QUuid& workspaceId, QWidget *parent = nullptr, WorkspaceRepository* repository = nullptr);
    [[nodiscard]] QString projectName() const;
    [[nodiscard]] QString projectDescription() const;

private slots:
    void validateInput(const QString& text);
    void SaveCreatedProject();
    void handleAccept();

private:
    // UI Elements
    QLineEdit* nameEdit_ = nullptr;
    QLineEdit* descriptionEdit_ = nullptr;
    QDialogButtonBox* buttonBox_ = nullptr;

    bool m_isNameUnique = true;
    WorkspaceRepository* repository_ = nullptr;
    QUuid workspaceId_;


};



#endif //TASKHELPER_PROJECTCREATEDIALOG_H

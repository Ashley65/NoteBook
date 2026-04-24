//
// Created by DevAccount on 23/02/2026.
//

#ifndef TASKHELPER_WORKSPACECREATEDIALOG_H
#define TASKHELPER_WORKSPACECREATEDIALOG_H
#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QComboBox>

#include "helpers/Workspace.h"
#include "Data/workspace/WorkspaceRepository.h"


class WorkspaceCreateDialog : public QDialog
{
    Q_OBJECT
public:
    explicit WorkspaceCreateDialog(QWidget *parent = nullptr, WorkspaceRepository* repository = nullptr);
    [[nodiscard]] QString workspaceName() const;
    [[nodiscard]] QString workspaceType() const;
    [[nodiscard]] QString workspaceDescription() const;

private slots:
    // This slot will be connected to the textChanged signal of the QLineEdit to validate the input in real-time as the user types.
    void validateInput(const QString& text); // This will need to be changed
    void ValidateName(const QString& name);
    void handleAccept();

private:
    // UI Elements
    QLineEdit* nameEdit_;
    QComboBox* typeComboBox_;
    QLineEdit* descriptionEdit_;
    bool unique = true; // Flag to track if the name is unique

    QDialogButtonBox* buttonBox_;
    // Create a reference to the workspace repository:
    WorkspaceRepository* repository_ = nullptr;
};




#endif //TASKHELPER_WORKSPACECREATEDIALOG_H
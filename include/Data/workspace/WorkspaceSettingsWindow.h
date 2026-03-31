//
// Created by DevAccount on 23/02/2026.
//

#ifndef TASKHELPER_WORKSPACESETTINGSDIALOG_H
#define TASKHELPER_WORKSPACESETTINGSDIALOG_H
#pragma once
#include <QDialog>
#include <QFrame>
#include <QLineEdit>
#include <QTextEdit>
#include <QComboBox>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QPushButton>
#include <QUuid>

#include "helpers/Workspace.h"
#include "Data/workspace/WorkspaceRepository.h"
#include "UI/components/Content_Windows/WorkspaceView.h"

class QVBoxLayout;
class QLabel;
class QCheckBox;

class WorkspaceSettingsWindow : public QDialog
{
    Q_OBJECT
public:
    explicit WorkspaceSettingsWindow(const QUuid& workspaceId,const QString& workspaceName,WorkspaceRepository* repository, QWidget* parent = nullptr);
    ~WorkspaceSettingsWindow() override;

private slots:
    // Event Handler
    void onSaveCLicked();
    void onCancelClicked();
    void onChangeIconClicked();

private:
    // Trying something new
    // Breaking the Ui into smaller chunks to make it cleaner in .cpp file
    void setupUi();
    void getGeneralInfo(QVBoxLayout* targetLayout);
    void setAppearance(QVBoxLayout* targetLayout);
    void getPermissions(QVBoxLayout* targetLayout);
    void setupActionsButtons();
    void connectSignals();

    // Helpers to create horizontal divider lines
    //QFrame* createHorizontalLine();
    QFrame* borderFrame();


    // UI pointers
    QVBoxLayout* mainLayout_ = nullptr;

    QLineEdit* nameLineEdit_ = nullptr;
    QComboBox* typeComboBox_ = nullptr;
    QTextEdit* descriptionTextEdit_ = nullptr;

    QLabel* iconPreviewLabel_ = nullptr;  // For placeholder have an gray circle
    QPushButton* changeIconButton_ = nullptr;
    QLineEdit* colourHexInput_ = nullptr;
    QList<QPushButton*> colorSwatches; // An List to hold the coloured circle buttons

    QCheckBox *protectedToggle = nullptr;
    QCheckBox *archivedToggle = nullptr;
    QCheckBox *pinnedToggle = nullptr;

    QLabel* passwordLabel = nullptr;
    QLineEdit* passwordInput = nullptr;


    QPushButton *cancelBtn = nullptr;
    QPushButton *saveBtn = nullptr;

    WorkspaceRepository* m_repository = nullptr; // To access and update the workspace data
    QLabel* nameErrorLabel_ = nullptr; // To show validation errors for the name input
    QUuid m_workspaceId;
    QString m_workspaceName;
};








#endif //TASKHELPER_WORKSPACESETTINGSDIALOG_H
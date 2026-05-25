//
// Created by DevAccount on 09/05/2026.
//

#ifndef TASKHELPER_UPLOADDIALOG_H
#define TASKHELPER_UPLOADDIALOG_H
#pragma once
#include <QDialog>
#include <QFileDialog>
#include <QLineEdit>
#include <QProgressBar>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include "helpers/AttachmentManager.h"

class DropAreaWidget;



class UploadDialog : public QDialog
{
    Q_OBJECT
public:
    explicit UploadDialog (QWidget* parent = nullptr);
    ~UploadDialog() override = default;

    void setWorkspaceId(const QUuid& wsId);
    void setProjectId(const QUuid& projectId);

    // Set a file filter for the file dialog (e.g., "Images (*.png *.jpg);;Documents (*.pdf *.docx)")
    void setFileFilter(const QString& filter);


signals:
    void uploadRequested(const QStringList& sourcePaths, const QUuid& workspaceId, const QUuid& projectId);

private slots:
    void onChooseFiles();
    void onUploadPressed();
    void onFilesDropped(const QStringList& filePaths);

private:
    QUuid workspaceId_;
    QUuid projectId_;
    QString fileFilter_ = "All Files (*.*)";

    QPushButton* m_btnChoose;
    QPushButton* m_btnUpload;
    QPushButton* m_btnCancel;
    QListWidget* m_fileList;
    QLabel* m_statusLabel;
    QProgressBar* m_progress;

    QStringList m_selectedFiles;
    DropAreaWidget* m_dropArea;
};




#endif //TASKHELPER_UPLOADDIALOG_H

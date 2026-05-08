//
// Created by DevAccount on 24/04/2026.
//

#ifndef TASKHELPER_ATTACHMENTMANAGER_H
#define TASKHELPER_ATTACHMENTMANAGER_H
#pragma once

#include <QObject>
#include <QUuid>
#include "Data/workspace/Structure/FileAttachment.h"

class WorkspaceRepository;
class AttachmentService;
class AttachmentValidator;


struct AttachmentUploadRequest
{
    QUuid workspaceId;
    QUuid projectId;
    AttachmentEntityType linkedEntityType = AttachmentEntityType::Workspace;
    QUuid linkedEntityId;
    QString sourceFilePath;
};



class AttachmentManager : public QObject
{
    Q_OBJECT

public:
    explicit AttachmentManager(WorkspaceRepository* repo, QObject* parent = nullptr);

    bool uploadAttachment(const AttachmentUploadRequest& request, FileAttachment& outAttachment, QString& outError);

    bool removeAttachment(const QUuid& attachmentId, QString& outError);

signals:
    void attachmentAdded(const QUuid& workspaceId, const QUuid& attachmentId);
    void attachmentRemoved(const QUuid& workspaceId, const QUuid& attachmentId);
    void uploadFailed(const QUuid& workspaceId, const QString& reason);

private:
    WorkspaceRepository* m_repo = nullptr;
    AttachmentService* m_service = nullptr;
    AttachmentValidator* m_validator = nullptr;


};
#endif //TASKHELPER_ATTACHMENTMANAGER_H

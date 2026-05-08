//
// Created by DevAccount on 26/02/2026.
//

#ifndef TASKHELPER_FILEATTACHMENT_H
#define TASKHELPER_FILEATTACHMENT_H
#pragma once
#include <QString>
#include <QDateTime>
#include <QUuid>


enum class AttachmentEntityType
{
    Task,
    Note,
    Workspace
};

struct FileAttachment
{
    QUuid id;
    QUuid workspaceId;
    QUuid projectId;
    AttachmentEntityType linkedEntityType = AttachmentEntityType::Workspace;
    QUuid linkedEntityId;

    QString fileName;
    QString relativePath;
    QString mimeType;
    qint64 fileSize = 0;

    QDateTime createdAt;
    QDateTime updatedAt;
};
#endif //TASKHELPER_FILEATTACHMENT_H
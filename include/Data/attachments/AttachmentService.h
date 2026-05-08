//
// Created by DevAccount on 24/04/2026.
//

#ifndef TASKHELPER_ATTACHMENTSERVICE_H
#define TASKHELPER_ATTACHMENTSERVICE_H
#pragma once

#include <QString>
#include <QUuid>

struct StoredAttachmentFile
{

    QString relativePath;
    QString storedFileName;
    qint64 fileSize = 0;

};

class AttachmentService
{
public:
    AttachmentService() = default;

    bool storeFileForWorkspace(const QUuid& workspaceId, const QString& sourceFilePath,
                               StoredAttachmentFile& outStored, QString& outError) const;

    bool deleteStoredFile(const QString& relativePath, QString& outError) const;

    QString resolveAbsolutePath(const QString& relativePath) const;

private:
    QString appStorageRoot() const;
    QString workspaceUploadDir(const QUuid& workspaceId) const;
    QString makeSafeStoredFileName(const QString& originalFileName, const QUuid& attachmentId) const;
    bool ensurePathWithinRoot(const QString& absolutePath) const;
};

#endif //TASKHELPER_ATTACHMENTSERVICE_H

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

/**
 * @file AttachmentManager.h
 * @brief Domain manager for uploading, validating, and removing file attachments.
 */

/**
 * @struct AttachmentUploadRequest
 * @brief Parameters for uploading and associating a file asset with a workspace entity.
 */
struct AttachmentUploadRequest
{
    QUuid workspaceId;                                                      ///< Owning workspace UUID.
    QUuid projectId;                                                        ///< Owning project UUID.
    AttachmentEntityType linkedEntityType = AttachmentEntityType::Workspace; ///< Entity type to associate (Workspace, Project, Note, Task).
    QUuid linkedEntityId;                                                   ///< Target entity UUID.
    QString sourceFilePath;                                                 ///< Filesystem path to the local source file to import.
};

/**
 * @class AttachmentManager
 * @brief Manages attachment ingestion workflows, file validation, and repository registration.
 *
 * AttachmentManager coordinates between the AttachmentValidator (which checks size and MIME type),
 * AttachmentService (which copies/stores files in the designated repository structure), and
 * WorkspaceRepository (which registers attachment metadata in the database).
 */
class AttachmentManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Constructs an AttachmentManager wired to the specified WorkspaceRepository.
     * @param repo Pointer to the shared WorkspaceRepository instance.
     * @param parent Optional parent QObject.
     */
    explicit AttachmentManager(WorkspaceRepository* repo, QObject* parent = nullptr);

    /**
     * @brief Validates, copies, and registers a file upload.
     * @param request Parameters specifying source file and target entity.
     * @param outAttachment Output parameter populated with the created FileAttachment model on success.
     * @param outError Output parameter containing error description if validation or storage fails.
     * @return True if upload succeeded; false otherwise.
     */
    bool uploadAttachment(const AttachmentUploadRequest& request, FileAttachment& outAttachment, QString& outError);

    /**
     * @brief Removes an attachment from the repository and purges its file on disk.
     * @param attachmentId UUID of the attachment to remove.
     * @param outError Output parameter populated with error details on failure.
     * @return True if removal succeeded.
     */
    bool removeAttachment(const QUuid& attachmentId, QString& outError);

signals:
    /**
     * @brief Emitted when a file is successfully uploaded and attached.
     * @param workspaceId Identifier of the parent workspace.
     * @param attachmentId Identifier of the new attachment.
     */
    void attachmentAdded(const QUuid& workspaceId, const QUuid& attachmentId);

    /**
     * @brief Emitted when an attachment is removed.
     * @param workspaceId Identifier of the parent workspace.
     * @param attachmentId Identifier of the removed attachment.
     */
    void attachmentRemoved(const QUuid& workspaceId, const QUuid& attachmentId);

    /**
     * @brief Emitted when an upload attempt fails validation or filesystem storage.
     * @param workspaceId Identifier of the parent workspace.
     * @param reason Human-readable failure explanation.
     */
    void uploadFailed(const QUuid& workspaceId, const QString& reason);

private:
    WorkspaceRepository* m_repo = nullptr;
    AttachmentService* m_service = nullptr;
    AttachmentValidator* m_validator = nullptr;
};

#endif //TASKHELPER_ATTACHMENTMANAGER_H

//
// Created by DevAccount on 11/04/2026.
//

#ifndef TASKHELPER_NOTEMANAGER_H
#define TASKHELPER_NOTEMANAGER_H
#pragma once
#include <QObject>
#include <QList>
#include "Data/workspace/WorkspaceRepository.h"
#include "Data/workspace/Structure/Note.h"

/**
 * @file NoteManager.h
 * @brief Domain manager for note authoring, sorting, pinning, and archiving.
 */

/**
 * @struct NoteCreateRequest
 * @brief Parameters for creating a new markdown note within a project.
 */
struct NoteCreateRequest
{
    QString title;          ///< Note title.
    QString content;        ///< Raw Markdown body content.
    QUuid workspaceId;      ///< Target workspace UUID.
    QUuid projectId;        ///< Target project UUID.
    bool isPinned = false;  ///< Whether the note should appear pinned at the top.
};

/**
 * @class NoteManager
 * @brief Domain manager providing note filtering, pin/archive state toggles, and reactive eventing.
 *
 * NoteManager wraps the note-related facilities of WorkspaceRepository, providing
 * specialized convenience queries for recent notes sorted by modification timestamp,
 * pinned notes, and archival workflows. It listens to repository mutation events and
 * emits `notesChanged` to trigger UI updates.
 */
class NoteManager : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief Constructs a NoteManager connected to a WorkspaceRepository instance.
     * @param repo Pointer to the shared WorkspaceRepository.
     * @param parent Optional parent QObject.
     */
    explicit NoteManager(WorkspaceRepository* repo, QObject* parent = nullptr);

    /**
     * @brief Retrieves non-archived notes sorted by most recently modified or created.
     * @param workspaceId Workspace UUID.
     * @param limit Maximum number of notes to return (default: 10).
     */
    QList<Note> recentNotes(const QUuid& workspaceId, int limit = 10) const;

    /**
     * @brief Retrieves all pinned, non-archived notes within a workspace.
     * @param workspaceId Workspace UUID.
     */
    QList<Note> pinnedNotes(const QUuid& workspaceId) const;

    /**
     * @brief Returns all notes belonging to the specified workspace.
     * @param workspaceId Workspace UUID.
     */
    QList<Note> allNotes(const QUuid& workspaceId) const;

    /**
     * @brief Fetches a single note by its unique identifier.
     * @param noteId Note UUID.
     */
    Note getNoteById(const QUuid& noteId) const;

    /**
     * @brief Creates and stores a note from a creation request.
     * @param request Populated NoteCreateRequest.
     * @return Generated Note UUID.
     */
    QUuid createNote(const NoteCreateRequest& request);

    /**
     * @brief Updates note attributes and markdown content.
     * @param note Note domain model.
     */
    void updateNote(const Note& note);

    /**
     * @brief Toggles whether a note is pinned in lists and dashboards.
     * @param noteId Note UUID.
     * @param pinned True to pin; false to unpin.
     */
    void setPinned(const QUuid& noteId, bool pinned);

    /**
     * @brief Archives or unarchives a note, hiding or showing it in standard lists.
     * @param noteId Note UUID.
     * @param archived True to archive; false to restore.
     */
    void setArchived(const QUuid& noteId, bool archived);

    /**
     * @brief Permanently deletes a note from storage.
     * @param noteId Identifier of the note to remove.
     */
    void deleteNote(const QUuid& noteId);

signals:
    /**
     * @brief Emitted when notes within the workspace are added, modified, or removed.
     * @param workspaceId Affected workspace UUID.
     */
    void notesChanged(const QUuid& workspaceId);

private slots:
    void onNoteAdded(const Note& note);
    void onNoteUpdated(const Note& note);
    void onNoteDeleted(const QUuid& noteId);

private:
    WorkspaceRepository* m_repo {nullptr};
    QUuid m_lastDeletedWorkspaceId;
};

#endif //TASKHELPER_NOTEMANAGER_H

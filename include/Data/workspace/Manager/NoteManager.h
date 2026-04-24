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

struct NoteCreateRequest
{
    QString title;
    QString content;
    QUuid workspaceId;
    QUuid projectId;
    bool isPinned = false;

};
class NoteManager : public QObject
{
    Q_OBJECT
public:
    explicit NoteManager(WorkspaceRepository* repo, QObject* parent = nullptr);
    QList<Note> recentNotes(const QUuid& workspaceId, int limit = 10) const;
    QList<Note> pinnedNotes(const QUuid& workspaceId) const;
    QList<Note> allNotes(const QUuid& workspaceId) const;
    Note getNoteById(const QUuid& noteId) const;

    QUuid createNote(const NoteCreateRequest& request);
    void updateNote(const Note& note);
    void setPinned(const QUuid& noteId, bool pinned);
    void setArchived(const QUuid& noteId, bool archived);
    void deleteNote(const QUuid& noteId);

signals:
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

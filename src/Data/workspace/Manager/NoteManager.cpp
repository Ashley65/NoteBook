//
// Created by DevAccount on 14/04/2026.
//
#include "Data/workspace/Manager/NoteManager.h"
#include <algorithm>

NoteManager::NoteManager(WorkspaceRepository* repo, QObject* parent)
    : QObject(parent), m_repo(repo)
{
    if (!m_repo)
    {
        return;
    }

    connect(m_repo, &WorkspaceRepository::noteAdded, this, &NoteManager::onNoteAdded);
    connect(m_repo, &WorkspaceRepository::noteUpdated, this, &NoteManager::onNoteUpdated);
    connect(m_repo, &WorkspaceRepository::noteDeleted, this, &NoteManager::onNoteDeleted);
}

QList<Note> NoteManager::recentNotes(const QUuid& workspaceId, int limit) const
{
    if (!m_repo || workspaceId.isNull() || limit <= 0)
    {
        return {};
    }

    QList<Note> notes = m_repo->getNotesByWorkspace(workspaceId);

    notes.erase(std::remove_if(notes.begin(), notes.end(), [](const Note& n)
    {
        return n.isArchived;
    }), notes.end());

    std::sort(notes.begin(), notes.end(), [](const Note& a, const Note& b)
    {
        const QDateTime aKey = a.updatedAt.isValid() ? a.updatedAt : a.createdAt;
        const QDateTime bKey = b.updatedAt.isValid() ? b.updatedAt : b.createdAt;
        return aKey > bKey;
    });

    if (notes.size() > limit)
    {
        notes = notes.mid(0, limit);
    }

    return notes;
}

QList<Note> NoteManager::pinnedNotes(const QUuid& workspaceId) const
{
    if (!m_repo || workspaceId.isNull())
    {
        return {};
    }

    QList<Note> notes = m_repo->getNotesByWorkspace(workspaceId);

    notes.erase(std::remove_if(notes.begin(), notes.end(), [](const Note& n)
    {
        return !n.isPinned || n.isArchived;
    }), notes.end());

    std::sort(notes.begin(), notes.end(), [](const Note& a, const Note& b)
    {
        const QDateTime aKey = a.updatedAt.isValid() ? a.updatedAt : a.createdAt;
        const QDateTime bKey = b.updatedAt.isValid() ? b.updatedAt : b.createdAt;
        return aKey > bKey;
    });

    return notes;
}

QList<Note> NoteManager::allNotes(const QUuid& workspaceId) const
{
    if (!m_repo || workspaceId.isNull())
    {
        return {};
    }

    QList<Note> notes = m_repo->getNotesByWorkspace(workspaceId);

    std::sort(notes.begin(), notes.end(), [](const Note& a, const Note& b)
    {
        if (a.isArchived != b.isArchived)
        {
            return !a.isArchived;
        }

        if (a.isPinned != b.isPinned)
        {
            return a.isPinned > b.isPinned;
        }

        const QDateTime aKey = a.updatedAt.isValid() ? a.updatedAt : a.createdAt;
        const QDateTime bKey = b.updatedAt.isValid() ? b.updatedAt : b.createdAt;
        return aKey > bKey;
    });

    return notes;
}

Note NoteManager::getNoteById(const QUuid& noteId) const
{
    if (!m_repo || noteId.isNull())
    {
        return {};
    }

    return m_repo->getNoteById(noteId);
}

QUuid NoteManager::createNote(const NoteCreateRequest& request)
{
    if (!m_repo || request.workspaceId.isNull() || request.title.trimmed().isEmpty())
    {
        return {};
    }

    Note note;
    note.workspaceId = request.workspaceId;
    note.projectId = request.projectId; // Repository assigns default project when null.
    note.title = request.title.trimmed();
    note.content = request.content;
    note.preview = request.content.left(160);
    note.isPinned = request.isPinned;
    note.isArchived = false;

    return m_repo->createNote(note);
}

void NoteManager::updateNote(const Note& note)
{
    if (!m_repo || note.id.isNull() || note.workspaceId.isNull() || note.title.trimmed().isEmpty())
    {
        return;
    }

    const Note existing = m_repo->getNoteById(note.id);
    if (existing.id.isNull())
    {
        return;
    }

    Note updated = note;
    updated.title = updated.title.trimmed();
    updated.preview = updated.content.left(160);

    if (updated.projectId.isNull())
    {
        updated.projectId = existing.projectId;
    }

    m_repo->updateNote(updated);
}

void NoteManager::setPinned(const QUuid& noteId, bool pinned)
{
    if (!m_repo || noteId.isNull())
    {
        return;
    }

    Note note = m_repo->getNoteById(noteId);
    if (note.id.isNull() || note.isPinned == pinned)
    {
        return;
    }

    note.isPinned = pinned;
    note.preview = note.content.left(160);
    m_repo->updateNote(note);
}

void NoteManager::setArchived(const QUuid& noteId, bool archived)
{
    if (!m_repo || noteId.isNull())
    {
        return;
    }

    Note note = m_repo->getNoteById(noteId);
    if (note.id.isNull() || note.isArchived == archived)
    {
        return;
    }

    note.isArchived = archived;
    note.preview = note.content.left(160);
    m_repo->updateNote(note);
}

void NoteManager::deleteNote(const QUuid& noteId)
{
    if (!m_repo || noteId.isNull())
    {
        return;
    }

    const Note existingNote = m_repo->getNoteById(noteId);
    if (existingNote.id.isNull())
    {
        return;
    }

    m_lastDeletedWorkspaceId = existingNote.workspaceId;
    m_repo->deleteNote(noteId);
}

void NoteManager::onNoteAdded(const Note& note)
{
    if (note.workspaceId.isNull())
    {
        return;
    }

    emit notesChanged(note.workspaceId);
}

void NoteManager::onNoteUpdated(const Note& note)
{
    if (note.id.isNull() || note.workspaceId.isNull())
    {
        return;
    }

    emit notesChanged(note.workspaceId);
}

void NoteManager::onNoteDeleted(const QUuid& noteId)
{
    if (noteId.isNull())
    {
        return;
    }

    if (!m_lastDeletedWorkspaceId.isNull())
    {
        emit notesChanged(m_lastDeletedWorkspaceId);
        m_lastDeletedWorkspaceId = QUuid{};
        return;
    }

    emit notesChanged(QUuid{});
}

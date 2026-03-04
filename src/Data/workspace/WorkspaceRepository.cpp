#include "Data/workspace/WorkspaceRepository.h"
#include <QUuid>
#include <QSettings>

WorkspaceRepository::WorkspaceRepository(QObject* parent) : QObject(parent)
{
    // LINK: Load existing workspaces and tasks from persistent storage
    loadWorkspaces();
    loadTasks();
    loadNotes();
    loadAttachments();
}

QList<Workspace> WorkspaceRepository::workspaces() const {
    return workspaces_;
}

Workspace WorkspaceRepository::getWorkspaceById(const QString& id) const {
    for (const auto& ws : workspaces_) {
        if (ws.id == id) return ws;
    }
    return {};
}

QString WorkspaceRepository::createWorkspace(const QString& name, const QString& type, const QString& description) {
    Workspace ws;
    ws.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    ws.name = name;
    ws.type = type;
    ws.description = description;
    ws.protectedMode = false;
    ws.order = workspaces_.size();
    ws.createdAt = QDateTime::currentDateTime();
    ws.updatedAt = ws.createdAt;

    workspaces_.append(ws);

    // LINK: Persist changes immediately
    saveWorkspaces();

    emit workspaceAdded(ws);
    return ws.id;
}

void WorkspaceRepository::updateWorkspace(const Workspace& ws) {
    for (int i = 0; i < workspaces_.size(); ++i) {
        if (workspaces_[i].id == ws.id) {
            workspaces_[i] = ws;
            workspaces_[i].updatedAt = QDateTime::currentDateTime();

            saveWorkspaces();
            emit workspaceUpdated(workspaces_[i]);
            return;
        }
    }
}

void WorkspaceRepository::deleteWorkspace(const QString& id)
{

    for (int i = 0; i < workspaces_.size(); ++i) {
        if (workspaces_[i].id == id) {
            workspaces_.removeAt(i);

            // LINK: Persist changes immediately
            saveWorkspaces();

            // LINK: Also remove all tasks associated with this workspace
            for (int j = tasks_.size() - 1; j >= 0; --j) {
                if (tasks_[j].workspaceId == id) {
                    tasks_.removeAt(j);
                }
            }
            saveTasks();

            // LINK: Also remove all notes and their attachments associated with this workspace
            for (int j = notes_.size() - 1; j >= 0; --j) {
                if (notes_[j].workspaceId.toString(QUuid::WithoutBraces) == id) {
                    deleteNote(notes_[j].id.toString(QUuid::WithoutBraces));
                }
            }
            // attachments are handled by deleteNote or deleted separately if not linked to a note
            for (int j = attachments_.size() - 1; j >= 0; --j) {
                if (attachments_[j].workspaceId.toString(QUuid::WithoutBraces) == id) {
                    attachments_.removeAt(j);
                }
            }
            saveAttachments();
            break;
        }
    }
}

// Note Management Methods
QList<Note> WorkspaceRepository::getNotesByWorkspace(const QString& workspaceId) const {
    QList<Note> result;
    QUuid wsId = QUuid::fromString(workspaceId);
    for (const auto& note : notes_) {
        if (note.workspaceId == wsId) {
            result.append(note);
        }
    }
    return result;
}

Note WorkspaceRepository::getNoteById(const QString& id) const {
    QUuid noteId = QUuid::fromString(id);
    for (const auto& note : notes_) {
        if (note.id == noteId) return note;
    }
    return {};
}

QString WorkspaceRepository::createNote(const Note& note) {
    Note newNote = note;
    newNote.id = QUuid::createUuid();
    newNote.createdAt = QDateTime::currentDateTime();
    newNote.updatedAt = newNote.createdAt;

    notes_.append(newNote);
    saveNotes();

    emit noteAdded(newNote);
    return newNote.id.toString(QUuid::WithoutBraces);
}

void WorkspaceRepository::updateNote(const Note& note) {
    for (int i = 0; i < notes_.size(); ++i) {
        if (notes_[i].id == note.id) {
            notes_[i] = note;
            notes_[i].updatedAt = QDateTime::currentDateTime();
            saveNotes();
            emit noteUpdated(notes_[i]);
            return;
        }
    }
}

void WorkspaceRepository::deleteNote(const QString& id) {
    QUuid noteId = QUuid::fromString(id);
    for (int i = 0; i < notes_.size(); ++i) {
        if (notes_[i].id == noteId) {
            notes_.removeAt(i);

            // Cascade delete attachments
            for (int j = attachments_.size() - 1; j >= 0; --j) {
                if (attachments_[j].noteId == noteId) {
                    attachments_.removeAt(j);
                }
            }
            saveAttachments();
            saveNotes();
            emit noteDeleted(id);
            return;
        }
    }
}

// Attachment Management Methods
QList<FileAttachment> WorkspaceRepository::getAttachmentsByWorkspace(const QString& workspaceId) const {
    QList<FileAttachment> result;
    QUuid wsId = QUuid::fromString(workspaceId);
    for (const auto& att : attachments_) {
        if (att.workspaceId == wsId) {
            result.append(att);
        }
    }
    return result;
}

QList<FileAttachment> WorkspaceRepository::getAttachmentsByNote(const QString& noteId) const {
    QList<FileAttachment> result;
    QUuid nId = QUuid::fromString(noteId);
    for (const auto& att : attachments_) {
        if (att.noteId == nId) {
            result.append(att);
        }
    }
    return result;
}

FileAttachment WorkspaceRepository::getAttachmentById(const QString& id) const {
    QUuid attId = QUuid::fromString(id);
    for (const auto& att : attachments_) {
        if (att.id == attId) return att;
    }
    return {};
}

QString WorkspaceRepository::createAttachment(const FileAttachment& attachment) {
    FileAttachment newAtt = attachment;
    newAtt.id = QUuid::createUuid();
    newAtt.createdAt = QDateTime::currentDateTime();

    attachments_.append(newAtt);
    saveAttachments();

    emit attachmentAdded(newAtt);
    return newAtt.id.toString(QUuid::WithoutBraces);
}

void WorkspaceRepository::deleteAttachment(const QString& id) {
    QUuid attId = QUuid::fromString(id);
    for (int i = 0; i < attachments_.size(); ++i) {
        if (attachments_[i].id == attId) {
            attachments_.removeAt(i);
            saveAttachments();
            emit attachmentDeleted(id);
            return;
        }
    }
}

// Task Management Methods
QList<Task> WorkspaceRepository::getTasksByWorkspace(const QString& workspaceId) const {
    QList<Task> result;
    for (const auto& task : tasks_) {
        if (task.workspaceId == workspaceId) {
            result.append(task);
        }
    }
    return result;
}

Task WorkspaceRepository::getTaskById(const QString& id) const {
    for (const auto& task : tasks_) {
        if (task.id == id) return task;
    }
    return {};
}

QString WorkspaceRepository::createTask(const Task& task) {
    Task newTask = task;
    newTask.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    newTask.createdAt = QDateTime::currentDateTime();

    if (newTask.status == TaskStatus::Completed) {
        newTask.completedAt = QDateTime::currentDateTime();
    }

    tasks_.append(newTask);

    // LINK: Persist changes immediately
    saveTasks();

    emit taskAdded(newTask);
    return newTask.id;
}

void WorkspaceRepository::updateTask(const Task& task) {
    for (int i = 0; i < tasks_.size(); ++i) {
        if (tasks_[i].id == task.id) {
            Task updatedTask = task;

            // LINK: Update completion timestamp if status changed to Completed
            if (updatedTask.status == TaskStatus::Completed && tasks_[i].status != TaskStatus::Completed) {
                updatedTask.completedAt = QDateTime::currentDateTime();
            }
            // LINK: Clear completion timestamp if task is no longer completed
            else if (updatedTask.status != TaskStatus::Completed && tasks_[i].status == TaskStatus::Completed) {
                updatedTask.completedAt = QDateTime();
            }

            tasks_[i] = updatedTask;

            // LINK: Persist changes immediately
            saveTasks();

            emit taskUpdated(updatedTask);
            return;
        }
    }
}

void WorkspaceRepository::deleteTask(const QString& id) {
    for (int i = 0; i < tasks_.size(); ++i) {
        if (tasks_[i].id == id) {
            tasks_.removeAt(i);

            // LINK: Persist changes immediately
            saveTasks();

            emit taskDeleted(id);
            return;
        }
    }
}

void WorkspaceRepository::saveWorkspaces()
{
    QSettings s("data.ini", QSettings::IniFormat);
    s.beginWriteArray("workspaces");

    for (int i = 0; i < workspaces_.size(); ++i) {
        s.setArrayIndex(i);
        s.setValue("id", workspaces_[i].id);
        s.setValue("name", workspaces_[i].name);
        s.setValue("type", workspaces_[i].type);
        s.setValue("description", workspaces_[i].description);
        s.setValue("icon", workspaces_[i].icon);
        s.setValue("color", workspaces_[i].color.name());
        s.setValue("protectedMode", workspaces_[i].protectedMode);
        s.setValue("isArchived", workspaces_[i].isArchived);
        s.setValue("isPinned", workspaces_[i].isPinned);
        s.setValue("order", workspaces_[i].order);
        s.setValue("createdAt", workspaces_[i].createdAt.toString(Qt::ISODate));
        s.setValue("updatedAt", workspaces_[i].updatedAt.toString(Qt::ISODate));
        s.setValue("lastOpenedAt", workspaces_[i].lastOpenedAt.toString(Qt::ISODate));
        s.setValue("taskCount", workspaces_[i].taskCount);
        s.setValue("noteCount", workspaces_[i].noteCount);
    }

    s.sync(); // Ensure data is written to disk immediately
    s.endArray();

}

void WorkspaceRepository::loadWorkspaces()
{
    QSettings s("data.ini", QSettings::IniFormat);
    int count = s.beginReadArray("workspaces");

    // clear existing list to prevent duplicates
    workspaces_.clear();

    for (int i = 0; i < count; ++i) {
        s.setArrayIndex(i);
        Workspace ws;
        ws.id = s.value("id").toString();
        ws.name = s.value("name").toString();
        ws.type = s.value("type", "custom").toString();
        ws.description = s.value("description").toString();
        ws.icon = s.value("icon").toString();
        ws.color = QColor(s.value("color").toString());
        ws.protectedMode = s.value("protectedMode", false).toBool();
        ws.isArchived = s.value("isArchived", false).toBool();
        ws.isPinned = s.value("isPinned", false).toBool();
        ws.order = s.value("order", i).toInt();
        ws.createdAt = QDateTime::fromString(s.value("createdAt").toString(), Qt::ISODate);
        ws.updatedAt = QDateTime::fromString(s.value("updatedAt").toString(), Qt::ISODate);
        ws.lastOpenedAt = QDateTime::fromString(s.value("lastOpenedAt").toString(), Qt::ISODate);
        ws.taskCount = s.value("taskCount", 0).toInt();
        ws.noteCount = s.value("noteCount", 0).toInt();
        workspaces_.append(ws);
    }


    s.endArray();
    s.sync(); // Ensure data is written to disk immediately
}

void WorkspaceRepository::saveTasks()
{
    QSettings s;
    s.beginWriteArray("tasks");

    for (int i = 0; i < tasks_.size(); ++i) {
        s.setArrayIndex(i);
        s.setValue("id", tasks_[i].id);
        s.setValue("workspaceId", tasks_[i].workspaceId);
        s.setValue("title", tasks_[i].title);
        s.setValue("description", tasks_[i].description);
        s.setValue("status", static_cast<int>(tasks_[i].status));
        s.setValue("priority", static_cast<int>(tasks_[i].priority));
        s.setValue("createdAt", tasks_[i].createdAt.toString(Qt::ISODate));
        s.setValue("dueDate", tasks_[i].dueDate.toString(Qt::ISODate));
        s.setValue("completedAt", tasks_[i].completedAt.toString(Qt::ISODate));
    }

    s.endArray();
    s.sync(); // Ensure data is written to disk immediately
}

void WorkspaceRepository::loadTasks()
{
    QSettings s;
    int count = s.beginReadArray("tasks");

    // clear existing list to prevent duplicates
    tasks_.clear();

    for (int i = 0; i < count; ++i) {
        s.setArrayIndex(i);
        Task task;
        task.id = s.value("id").toString();
        task.workspaceId = s.value("workspaceId").toString();
        task.title = s.value("title").toString();
        task.description = s.value("description").toString();
        task.status = static_cast<TaskStatus>(s.value("status", static_cast<int>(TaskStatus::Pending)).toInt());
        task.priority = static_cast<TaskPriority>(s.value("priority", static_cast<int>(TaskPriority::Medium)).toInt());
        task.createdAt = QDateTime::fromString(s.value("createdAt").toString(), Qt::ISODate);
        task.dueDate = QDateTime::fromString(s.value("dueDate").toString(), Qt::ISODate);
        task.completedAt = QDateTime::fromString(s.value("completedAt").toString(), Qt::ISODate);
        tasks_.append(task);
    }

    s.endArray();
}

void WorkspaceRepository::saveNotes()
{
    QSettings s;
    s.beginWriteArray("notes");

    for (int i = 0; i < notes_.size(); ++i) {
        s.setArrayIndex(i);
        s.setValue("id", notes_[i].id.toString(QUuid::WithoutBraces));
        s.setValue("workspaceId", notes_[i].workspaceId.toString(QUuid::WithoutBraces));
        s.setValue("title", notes_[i].title);
        s.setValue("content", notes_[i].content);
        s.setValue("preview", notes_[i].preview);
        s.setValue("isPinned", notes_[i].isPinned);
        s.setValue("isArchived", notes_[i].isArchived);
        s.setValue("createdAt", notes_[i].createdAt.toString(Qt::ISODate));
        s.setValue("updatedAt", notes_[i].updatedAt.toString(Qt::ISODate));
    }

    s.endArray();
}

void WorkspaceRepository::loadNotes()
{
    QSettings s;
    int count = s.beginReadArray("notes");

    notes_.clear();

    for (int i = 0; i < count; ++i) {
        s.setArrayIndex(i);
        Note note;
        note.id = QUuid::fromString(s.value("id").toString());
        note.workspaceId = QUuid::fromString(s.value("workspaceId").toString());
        note.title = s.value("title").toString();
        note.content = s.value("content").toString();
        note.preview = s.value("preview").toString();
        note.isPinned = s.value("isPinned", false).toBool();
        note.isArchived = s.value("isArchived", false).toBool();
        note.createdAt = QDateTime::fromString(s.value("createdAt").toString(), Qt::ISODate);
        note.updatedAt = QDateTime::fromString(s.value("updatedAt").toString(), Qt::ISODate);
        notes_.append(note);
    }

    s.endArray();
}

void WorkspaceRepository::saveAttachments()
{
    QSettings s;
    s.beginWriteArray("attachments");

    for (int i = 0; i < attachments_.size(); ++i) {
        s.setArrayIndex(i);
        s.setValue("id", attachments_[i].id.toString(QUuid::WithoutBraces));
        s.setValue("workspaceId", attachments_[i].workspaceId.toString(QUuid::WithoutBraces));
        s.setValue("noteId", attachments_[i].noteId.toString(QUuid::WithoutBraces));
        s.setValue("fileName", attachments_[i].fileName);
        s.setValue("filePath", attachments_[i].filePath);
        s.setValue("mimeType", attachments_[i].mimeType);
        s.setValue("fileSize", attachments_[i].fileSize);
        s.setValue("createdAt", attachments_[i].createdAt.toString(Qt::ISODate));
    }

    s.endArray();
}

void WorkspaceRepository::loadAttachments()
{
    QSettings s;
    int count = s.beginReadArray("attachments");

    attachments_.clear();

    for (int i = 0; i < count; ++i) {
        s.setArrayIndex(i);
        FileAttachment att;
        att.id = QUuid::fromString(s.value("id").toString());
        att.workspaceId = QUuid::fromString(s.value("workspaceId").toString());
        att.noteId = QUuid::fromString(s.value("noteId").toString());
        att.fileName = s.value("fileName").toString();
        att.filePath = s.value("filePath").toString();
        att.mimeType = s.value("mimeType").toString();
        att.fileSize = s.value("fileSize").toLongLong();
        att.createdAt = QDateTime::fromString(s.value("createdAt").toString(), Qt::ISODate);
        attachments_.append(att);
    }

    s.endArray();
}





#include "Data/workspace/WorkspaceRepository.h"
#include <QUuid>
#include <QSettings>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QFileInfo>

WorkspaceRepository::WorkspaceRepository(QObject* parent) : QObject(parent)
{
    // LINK: Load existing workspaces and tasks from persistent storage
    loadWorkspaces();
    loadProjects();
    loadTasks();
    loadNotes();
    loadAttachments();
    ensureProjectStructure();
}



QList<Workspace> WorkspaceRepository::workspaces() const {
    return workspaces_;
}

Workspace WorkspaceRepository::getWorkspaceById(const QUuid& id) const {
    for (const auto& ws : workspaces_) {
        if (ws.id == id) return ws;
    }
    return {};
}

QUuid WorkspaceRepository::createWorkspace(const QString& name, const QString& type, const QString& description) {
    Workspace ws;
    ws.id = QUuid::createUuid();
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

void WorkspaceRepository::deleteWorkspace(const QUuid& id)
{

    for (int i = 0; i < workspaces_.size(); ++i) {
        if (workspaces_[i].id == id) {
            workspaces_.removeAt(i);

            // Persist changes immediately
            saveWorkspaces();

            // Also remove all tasks associated with this workspace
            for (int j = tasks_.size() - 1; j >= 0; --j) {
                if (tasks_[j].workspaceId == id) {
                    tasks_.removeAt(j);
                }
            }
            saveTasks();

            for (int j = projects_.size() - 1; j >= 0; --j) {
                if (projects_[j].workspaceId == id) {
                    projects_.removeAt(j);
                }
            }
            saveProjects();

            // Also remove all notes and their attachments associated with this workspace
            for (int j = notes_.size() - 1; j >= 0; --j) {
                if (notes_[j].workspaceId == id) {
                    deleteNote(notes_[j].id);
                }
            }
            // attachments are handled by deleteNote or deleted separately if not linked to a note
            for (int j = attachments_.size() - 1; j >= 0; --j) {
                if (attachments_[j].workspaceId == id) {
                    attachments_.removeAt(j);
                }
            }
            saveAttachments();
            cleanUpOrphanedDataForWorkspace(id);
            break;
        }
    }
}

QList<Project> WorkspaceRepository::getProjectsByWorkspace(const QUuid& workspaceId) const {
    QList<Project> result;
    for (const auto& project : projects_) {
        if (project.workspaceId == workspaceId) {
            result.append(project);
        }
    }
    return result;
}

Project WorkspaceRepository::getProjectById(const QUuid& id) const {
    for (const auto& project : projects_) {
        if (project.id == id) return project;
    }
    return {};
}

QUuid WorkspaceRepository::createProject(const Project& project) {
    Project newProject = project;
    newProject.id = QUuid::createUuid();
    newProject.createdAt = QDateTime::currentDateTime();
    newProject.updatedAt = newProject.createdAt;

    projects_.append(newProject);
    saveProjects();

    // LINK: Ensure on-disk project directory structure is created
    ensureProjectDir(newProject.workspaceId, newProject.id);

    emit projectAdded(newProject);
    return newProject.id;
}

void WorkspaceRepository::updateProject(const Project& project) {
    for (int i = 0; i < projects_.size(); ++i) {
        if (projects_[i].id == project.id) {
            projects_[i] = project;
            projects_[i].updatedAt = QDateTime::currentDateTime();
            saveProjects();
            emit projectUpdated(projects_[i]);
            return;
        }
    }
}

void WorkspaceRepository::deleteProject(const QUuid& id) {
    QUuid workspaceId;
    for (int i = 0; i < projects_.size(); ++i) {
        if (projects_[i].id == id) {
            workspaceId = projects_[i].workspaceId;
            projects_.removeAt(i);
            break;
        }
    }

    for (int i = tasks_.size() - 1; i >= 0; --i) {
        if (tasks_[i].projectId == id) {
            tasks_.removeAt(i);
        }
    }

    for (int i = notes_.size() - 1; i >= 0; --i) {
        if (notes_[i].projectId == id) {
            deleteNote(notes_[i].id);
        }
    }

    for (int i = attachments_.size() - 1; i >= 0; --i) {
        if (attachments_[i].projectId == id) {
            attachments_.removeAt(i);
        }
    }

    saveProjects();
    saveTasks();
    saveNotes();
    saveAttachments();

    // LINK: Remove project directory from disk
    if (!workspaceId.isNull()) {
        QDir projectDir(projectPath(workspaceId, id));
        projectDir.removeRecursively();
    }

    emit projectDeleted(id);
}

// Note Management Methods
QList<Note> WorkspaceRepository::getNotesByWorkspace(const QUuid& workspaceId) const {
    QList<Note> result;
    for (const auto& note : notes_) {
        if (note.workspaceId == workspaceId) {
            result.append(note);
        }
    }
    return result;
}

QList<Note> WorkspaceRepository::getNotesByProject(const QUuid& projectId) const {
    QList<Note> result;
    for (const auto& note : notes_) {
        if (note.projectId == projectId) {
            result.append(note);
        }
    }
    return result;
}

Note WorkspaceRepository::getNoteById(const QUuid& id) const {
    for (const auto& note : notes_) {
        if (note.id == id) return note;
    }
    return {};
}

QUuid WorkspaceRepository::createNote(const Note& note) {
    Note newNote = note;
    newNote.id = QUuid::createUuid();
    if (newNote.projectId.isNull()) {
        newNote.projectId = defaultProjectForWorkspace(newNote.workspaceId);
    }
    newNote.createdAt = QDateTime::currentDateTime();
    newNote.updatedAt = newNote.createdAt;

    notes_.append(newNote);
    saveNotes();

    // LINK: Persist note content to disk
    saveNoteToFile(newNote);

    emit noteAdded(newNote);
    return newNote.id;
}

void WorkspaceRepository::updateNote(const Note& note) {
    for (int i = 0; i < notes_.size(); ++i) {
        if (notes_[i].id == note.id) {
            notes_[i] = note;
            notes_[i].updatedAt = QDateTime::currentDateTime();
            saveNotes();

            // LINK: Update note content on disk
            saveNoteToFile(notes_[i]);

            emit noteUpdated(notes_[i]);
            return;
        }
    }
}

void WorkspaceRepository::deleteNote(const QUuid& id) {
    for (int i = 0; i < notes_.size(); ++i) {
        if (notes_[i].id == id) {
            const Note deletingNote = notes_[i];
            notes_.removeAt(i);

            // Cascade delete attachments
            for (int j = attachments_.size() - 1; j >= 0; --j) {
                if (attachments_[j].noteId == id) {
                    deleteAttachment(attachments_[j].id);
                }
            }
            saveAttachments();
            saveNotes();

            // LINK: Remove note file from disk
            removeNoteFromFile(deletingNote);

            emit noteDeleted(id);
            return;
        }
    }
}

// Attachment Management Methods
QList<FileAttachment> WorkspaceRepository::getAttachmentsByWorkspace(const QUuid& workspaceId) const {
    QList<FileAttachment> result;
    for (const auto& att : attachments_) {
        if (att.workspaceId == workspaceId) {
            result.append(att);
        }
    }
    return result;
}

QList<FileAttachment> WorkspaceRepository::getAttachmentsByProject(const QUuid& projectId) const {
    QList<FileAttachment> result;
    for (const auto& att : attachments_) {
        if (att.projectId == projectId) {
            result.append(att);
        }
    }
    return result;
}

QList<FileAttachment> WorkspaceRepository::getAttachmentsByNote(const QUuid& noteId) const {
    QList<FileAttachment> result;
    for (const auto& att : attachments_) {
        if (att.noteId == noteId) {
            result.append(att);
        }
    }
    return result;
}

FileAttachment WorkspaceRepository::getAttachmentById(const QUuid& id) const {
    for (const auto& att : attachments_) {
        if (att.id == id) return att;
    }
    return {};
}

QUuid WorkspaceRepository::createAttachment(const FileAttachment& attachment) {
    FileAttachment newAtt = attachment;
    newAtt.id = QUuid::createUuid();
    if (newAtt.projectId.isNull()) {
        newAtt.projectId = defaultProjectForWorkspace(newAtt.workspaceId);
    }
    newAtt.createdAt = QDateTime::currentDateTime();

    // LINK: Store file in project attachment directory
    QString storedFilePath = storeAttachmentFile(newAtt);
    if (!storedFilePath.isEmpty()) {
        newAtt.filePath = storedFilePath;
    }

    attachments_.append(newAtt);
    saveAttachments();

    emit attachmentAdded(newAtt);
    return newAtt.id;
}

void WorkspaceRepository::deleteAttachment(const QUuid& id) {
    for (int i = 0; i < attachments_.size(); ++i) {
        if (attachments_[i].id == id) {
            const FileAttachment deletingAtt = attachments_[i];
            attachments_.removeAt(i);
            saveAttachments();

            // LINK: Remove attachment file from disk
            removeAttachmentFile(deletingAtt);

            emit attachmentDeleted(id);
            return;
        }
    }
}



// Task Management Methods
QList<Task> WorkspaceRepository::getTasksByWorkspace(const QUuid& workspaceId) const {
    QList<Task> result;
    for (const auto& task : tasks_) {
        if (task.workspaceId == workspaceId) {
            result.append(task);
        }
    }
    return result;
}

QList<Task> WorkspaceRepository::getTasksByProject(const QUuid& projectId) const {
    QList<Task> result;
    for (const auto& task : tasks_) {
        if (task.projectId == projectId) {
            result.append(task);
        }
    }
    return result;
}

Task WorkspaceRepository::getTaskById(const QUuid& id) const {
    for (const auto& task : tasks_) {
        if (task.id == id) return task;
    }
    return {};
}

QUuid WorkspaceRepository::createTask(const Task& task) {
    Task newTask = task;
    newTask.id = QUuid::createUuid();
    if (newTask.projectId.isNull()) {
        newTask.projectId = defaultProjectForWorkspace(newTask.workspaceId);
    }
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

void WorkspaceRepository::deleteTask(const QUuid& id) {
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
        ws.id = QUuid::fromString(s.value("id").toString());
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

void WorkspaceRepository::saveProjects()
{
    QSettings s("data.ini", QSettings::IniFormat);
    s.beginWriteArray("projects");

    for (int i = 0; i < projects_.size(); ++i) {
        s.setArrayIndex(i);
        s.setValue("id", projects_[i].id.toString(QUuid::WithoutBraces));
        s.setValue("workspaceId", projects_[i].workspaceId.toString(QUuid::WithoutBraces));
        s.setValue("name", projects_[i].name);
        s.setValue("description", projects_[i].description);
        s.setValue("isArchived", projects_[i].isArchived);
        s.setValue("createdAt", projects_[i].createdAt.toString(Qt::ISODate));
        s.setValue("updatedAt", projects_[i].updatedAt.toString(Qt::ISODate));
    }

    s.endArray();
    s.sync();
}

void WorkspaceRepository::loadProjects()
{
    QSettings s("data.ini", QSettings::IniFormat);
    int count = s.beginReadArray("projects");

    projects_.clear();

    for (int i = 0; i < count; ++i) {
        s.setArrayIndex(i);
        Project project;
        project.id = QUuid::fromString(s.value("id").toString());
        project.workspaceId = QUuid::fromString(s.value("workspaceId").toString());
        project.name = s.value("name").toString();
        project.description = s.value("description").toString();
        project.isArchived = s.value("isArchived", false).toBool();
        project.createdAt = QDateTime::fromString(s.value("createdAt").toString(), Qt::ISODate);
        project.updatedAt = QDateTime::fromString(s.value("updatedAt").toString(), Qt::ISODate);
        projects_.append(project);
    }

    s.endArray();
}

void WorkspaceRepository::saveTasks()
{
    QSettings s("data.ini", QSettings::IniFormat);
    s.beginWriteArray("tasks");

    for (int i = 0; i < tasks_.size(); ++i) {
        s.setArrayIndex(i);
        s.setValue("id", tasks_[i].id);
        s.setValue("workspaceId", tasks_[i].workspaceId);
        s.setValue("projectId", tasks_[i].projectId);
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
    QSettings s("data.ini", QSettings::IniFormat);
    int count = s.beginReadArray("tasks");

    // clear existing list to prevent duplicates
    tasks_.clear();

    for (int i = 0; i < count; ++i) {
        s.setArrayIndex(i);
        Task task;
        task.id = QUuid::fromString(s.value("id").toString());
        task.workspaceId = QUuid::fromString(s.value("workspaceId").toString());
        task.projectId = QUuid::fromString(s.value("projectId").toString());
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
    s.sync();
}

void WorkspaceRepository::saveNotes()
{
    QSettings s("data.ini", QSettings::IniFormat);
    s.beginWriteArray("notes");

    for (int i = 0; i < notes_.size(); ++i) {
        s.setArrayIndex(i);
        s.setValue("id", notes_[i].id.toString(QUuid::WithoutBraces));
        s.setValue("workspaceId", notes_[i].workspaceId.toString(QUuid::WithoutBraces));
        s.setValue("projectId", notes_[i].projectId.toString(QUuid::WithoutBraces));
        s.setValue("title", notes_[i].title);
        s.setValue("content", notes_[i].content);
        s.setValue("preview", notes_[i].preview);
        s.setValue("isPinned", notes_[i].isPinned);
        s.setValue("isArchived", notes_[i].isArchived);
        s.setValue("createdAt", notes_[i].createdAt.toString(Qt::ISODate));
        s.setValue("updatedAt", notes_[i].updatedAt.toString(Qt::ISODate));
    }

    s.endArray();
    s.sync();
}

void WorkspaceRepository::loadNotes()
{
    QSettings s("data.ini", QSettings::IniFormat);
    int count = s.beginReadArray("notes");

    notes_.clear();

    for (int i = 0; i < count; ++i) {
        s.setArrayIndex(i);
        Note note;
        note.id = QUuid::fromString(s.value("id").toString());
        note.workspaceId = QUuid::fromString(s.value("workspaceId").toString());
        note.projectId = QUuid::fromString(s.value("projectId").toString());
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
    s.sync();
}

void WorkspaceRepository::saveAttachments()
{
    QSettings s("data.ini", QSettings::IniFormat);
    s.beginWriteArray("attachments");

    for (int i = 0; i < attachments_.size(); ++i) {
        s.setArrayIndex(i);
        s.setValue("id", attachments_[i].id.toString(QUuid::WithoutBraces));
        s.setValue("workspaceId", attachments_[i].workspaceId.toString(QUuid::WithoutBraces));
        s.setValue("projectId", attachments_[i].projectId.toString(QUuid::WithoutBraces));
        s.setValue("noteId", attachments_[i].noteId.toString(QUuid::WithoutBraces));
        s.setValue("fileName", attachments_[i].fileName);
        s.setValue("filePath", attachments_[i].filePath);
        s.setValue("mimeType", attachments_[i].mimeType);
        s.setValue("fileSize", attachments_[i].fileSize);
        s.setValue("createdAt", attachments_[i].createdAt.toString(Qt::ISODate));
    }

    s.endArray();
    s.sync();
}

void WorkspaceRepository::loadAttachments()
{
    QSettings s("data.ini", QSettings::IniFormat);
    int count = s.beginReadArray("attachments");

    attachments_.clear();

    for (int i = 0; i < count; ++i) {
        s.setArrayIndex(i);
        FileAttachment att;
        att.id = QUuid::fromString(s.value("id").toString());
        att.workspaceId = QUuid::fromString(s.value("workspaceId").toString());
        att.projectId = QUuid::fromString(s.value("projectId").toString());
        att.noteId = QUuid::fromString(s.value("noteId").toString());
        att.fileName = s.value("fileName").toString();
        att.filePath = s.value("filePath").toString();
        att.mimeType = s.value("mimeType").toString();
        att.fileSize = s.value("fileSize").toLongLong();
        att.createdAt = QDateTime::fromString(s.value("createdAt").toString(), Qt::ISODate);
        attachments_.append(att);
    }

    s.endArray();
    s.sync();
}

void WorkspaceRepository::ensureProjectStructure()
{
    bool projectsChanged = false;
    bool tasksChanged = false;
    bool notesChanged = false;
    bool attachmentsChanged = false;

    for (const auto& ws : workspaces_) {
        if (defaultProjectForWorkspace(ws.id).isNull()) {
            Project project;
            project.id = QUuid::createUuid();
            project.workspaceId = ws.id;
            project.name = "General";
            project.description = "Default project for migrated workspace records.";
            project.isArchived = false;
            project.createdAt = QDateTime::currentDateTime();
            project.updatedAt = project.createdAt;
            projects_.append(project);
            projectsChanged = true;

            // LINK: Ensure on-disk directory structure for new default project
            ensureProjectDir(ws.id, project.id);
        }
    }

    for (auto& task : tasks_) {
        if (task.projectId.isNull()) {
            task.projectId = defaultProjectForWorkspace(task.workspaceId);
            tasksChanged = true;
        }
    }

    for (auto& note : notes_) {
        if (note.projectId.isNull()) {
            note.projectId = defaultProjectForWorkspace(note.workspaceId);
            notesChanged = true;
        }
    }

    for (auto& att : attachments_) {
        if (att.projectId.isNull()) {
            att.projectId = defaultProjectForWorkspace(att.workspaceId);
            attachmentsChanged = true;
        }
    }

    if (projectsChanged) saveProjects();
    if (tasksChanged) saveTasks();
    if (notesChanged) saveNotes();
    if (attachmentsChanged) saveAttachments();
}

QUuid WorkspaceRepository::defaultProjectForWorkspace(const QUuid& workspaceId) const
{
    for (const auto& project : projects_) {
        if (project.workspaceId == workspaceId) {
            return project.id;
        }
    }
    return {};
}

QString WorkspaceRepository::dataRootPath() const {
    const QString base = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    return QDir(base).filePath("TaskHelperData/workspace");
}

QString WorkspaceRepository::projectPath(const QUuid &workspaceId, const QUuid &projectId) const {
    QDir root(dataRootPath());
    return root.filePath(uuidKey(workspaceId) + "/projects/" + uuidKey(projectId));
}

bool WorkspaceRepository::ensureProjectDir(const QUuid &workspaceId, const QUuid &projectId) const {
    QDir d(projectPath(workspaceId, projectId));
    return d.mkpath(".") && d.mkpath("tasks") && d.mkpath("notes") && d.mkpath("attachments");

}

void WorkspaceRepository::saveNoteToFile(const Note& note) const {
    // LINK: Save note content to disk as markdown file
    QString notePath = QDir(projectPath(note.workspaceId, note.projectId)).filePath("notes");
    QString noteFile = QDir(notePath).filePath(uuidKey(note.id) + ".md");

    QFile file(noteFile);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        file.write(note.content.toUtf8());
        file.close();
    }
}

void WorkspaceRepository::removeNoteFromFile(const Note& note) const {
    // LINK: Remove note file from disk
    QString notePath = QDir(projectPath(note.workspaceId, note.projectId)).filePath("notes");
    QString noteFile = QDir(notePath).filePath(uuidKey(note.id) + ".md");

    QFile::remove(noteFile);
}

QString WorkspaceRepository::storeAttachmentFile(const FileAttachment& attachment) const {
    // LINK: Copy/move attachment file to project attachment storage
    if (attachment.filePath.isEmpty()) {
        return "";
    }

    QFileInfo sourceInfo(attachment.filePath);
    if (!sourceInfo.exists()) {
        return "";
    }

    // Create stored path: <project>/attachments/<attachmentUuid>/<originalFileName>
    QString attStorePath = QDir(projectPath(attachment.workspaceId, attachment.projectId))
        .filePath("attachments/" + uuidKey(attachment.id));

    QDir attStoreDir(attStorePath);
    if (!attStoreDir.mkpath(".")) {
        return "";
    }

    QString storedFile = QDir(attStorePath).filePath(sourceInfo.fileName());

    // Copy file to storage location
    if (QFile::copy(attachment.filePath, storedFile)) {
        return storedFile;
    }

    return "";
}

void WorkspaceRepository::removeAttachmentFile(const FileAttachment& attachment) const {
    // LINK: Remove attachment file and folder from disk
    if (attachment.filePath.isEmpty()) {
        return;
    }

    QFile::remove(attachment.filePath);

    // Also try to remove the attachment folder if empty
    QString attFolderPath = QDir(projectPath(attachment.workspaceId, attachment.projectId))
        .filePath("attachments/" + uuidKey(attachment.id));
    QDir(attFolderPath).removeRecursively();
}

OrphanedFileReport WorkspaceRepository::scanForOrphanedFiles() const {
    OrphanedFileReport report;

    report.totalSize = 0;
    report.totalFiles = 0;

    // Scan directories for notes and at
    for (const auto& ws : workspaces_) {
        for (const auto& project : getProjectsByWorkspace(ws.id)) {
            scanOrphanedNotes(ws.id, project.id, report);
            scanOrphanedAttachments(ws.id, project.id, report);

        }
    }
    return report;
}

void WorkspaceRepository::scanOrphanedNotes(const QUuid &workspaceId, const QUuid &projectId,
    OrphanedFileReport &report) const{
    QString notesDirPath = QDir(projectPath(workspaceId, projectId)).filePath("notes");
    QDir notesDir(notesDirPath);

    if (!notesDir.exists()) return;

    QStringList filters;
    filters << "*.md";

    QStringList files = notesDir.entryList(filters, QDir::Files);

    for (const auto& file :files) {
        QString filePath = notesDir.filePath(file);
        QUuid noteId = QUuid::fromString(QFileInfo(file).completeBaseName());


        bool noteExists = false;
        for (const auto& note : notes_) {
            if (note.id == noteId && note.projectId == projectId) {
                noteExists = true;
                break;
            }
        }

        if (!noteExists) {
            OrphanedFileInfo info;
            info.filePath = filePath;
            info.fileName = file;
            info.Type = "note";
            info.lastModified = QFileInfo(filePath).lastModified();
            info.fileSize = QFileInfo(filePath).size();

            report.orphanedFiles.append(info);
            report.totalSize += info.fileSize;
            report.totalFiles++;
        }
    }


}

void WorkspaceRepository::scanOrphanedAttachments(const QUuid &workspaceId, const QUuid &projectId,
    OrphanedFileReport &report) const {
    QString attDirPath = QDir(projectPath(workspaceId, projectId)).filePath("attachments");
    QDir attDir(attDirPath);

    if (!attDir.exists()) return;

    QStringList folders = attDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

    for (const auto& folder : folders) {
        QUuid attachmentId = QUuid::fromString(folder);

        bool attachmentExists = false;
        for (const auto& att : attachments_) {
            if (att.id == attachmentId && att.projectId == projectId) {
                attachmentExists = true;
                break;
            }
        }

        if (!attachmentExists) {
            QString folderPath = attDir.filePath(folder);
            QDir folderDir(folderPath);
            QStringList attFiles = folderDir.entryList(QDir::Files);

            for (const auto& attFile : attFiles) {
                QString filePath = folderDir.filePath(attFile);

                OrphanedFileInfo info;
                info.filePath = filePath;
                info.fileName = attFile;
                info.Type = "attachment";
                info.lastModified = QFileInfo(filePath).lastModified();
                info.fileSize = QFileInfo(filePath).size();

                report.orphanedFiles.append(info);
                report.totalSize += info.fileSize;
                report.totalFiles++;
            }
        }
    }

}

void WorkspaceRepository::deleteOrphanedFiles(const QList<QString> &filePaths) {
   for (const auto& filePath : filePaths) {
       deleteOrphanedFile(filePath);
   }
}

void WorkspaceRepository::deleteOrphanedFile(const QString &filePath) {
    QFile::remove(filePath);

    QDir parentDir = QFileInfo(filePath).dir();
    if (parentDir.entryList(QDir::Files).isEmpty()) {
        parentDir.removeRecursively();
    }
}

void WorkspaceRepository::deleteAllOrphanedFiles() {
    OrphanedFileReport report = scanForOrphanedFiles();
    QList<QString> paths;
    for (const auto& info : report.orphanedFiles) {
        paths.append(info.filePath);
    }
    deleteOrphanedFiles(paths);
}

void WorkspaceRepository::cleanUpOrphanedDataForWorkspace(const QUuid &workspaceId) {
    QDir workspaceDir(QDir(dataRootPath()).filePath(uuidKey(workspaceId)));
    workspaceDir.removeRecursively();
}


void WorkspaceRepository::cleanUpOrphanedData()
{
    deleteAllOrphanedFiles();
}


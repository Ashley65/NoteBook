#include "Data/workspace/WorkspaceRepository.h"
#include "Data/Database/DatabaseManager.h"
#include <QUuid>
#include <QSettings>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QSaveFile>
#include <QFileInfo>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QRegularExpression>

WorkspaceRepository::WorkspaceRepository(QObject* parent) : QObject(parent)
{
    m_dbManager = std::make_unique<DatabaseManager>("taskhelper_main_repo");
    const QString dbPath = QDir(dataRootPath()).filePath("taskhelper.db");
    if (!m_dbManager->open(dbPath)) {
        qCritical() << "Failed to open SQLite database:" << m_dbManager->lastError();
    }

    migrateFromLegacySettingsIfNecessary();

    loadWorkspaces();
    loadProjects();
    loadTasks();
    loadNotes();
    loadAttachments();
    ensureProjectStructure();
}

WorkspaceRepository::~WorkspaceRepository() = default;



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

            // Also remove all tasks associated with this workspace
            for (int j = tasks_.size() - 1; j >= 0; --j) {
                if (tasks_[j].workspaceId == id) {
                    tasks_.removeAt(j);
                }
            }

            for (int j = projects_.size() - 1; j >= 0; --j) {
                if (projects_[j].workspaceId == id) {
                    projects_.removeAt(j);
                }
            }

            // Also remove all notes and their attachments associated with this workspace
            for (int j = notes_.size() - 1; j >= 0; --j) {
                if (notes_[j].workspaceId == id) {
                    deleteNote(notes_[j].id);
                }
            }
            // attachments are handled by deleteNote or deleted separately if not linked to a note
            for (int j = attachments_.size() - 1; j >= 0; --j) {
                if (attachments_[j].workspaceId == id) {
                    deleteAttachment(attachments_[j].id);
                }
            }

            if (m_dbManager && m_dbManager->isOpen()) {
                m_dbManager->deleteWorkspace(id);
            }

            saveWorkspaces();
            saveTasks();
            saveProjects();
            saveNotes();
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
            deleteTask(tasks_[i].id);
        }
    }

    for (int i = notes_.size() - 1; i >= 0; --i) {
        if (notes_[i].projectId == id) {
            deleteNote(notes_[i].id);
        }
    }

    for (int i = attachments_.size() - 1; i >= 0; --i) {
        if (attachments_[i].projectId == id) {
            deleteAttachment(attachments_[i].id);
        }
    }

    if (m_dbManager && m_dbManager->isOpen()) {
        m_dbManager->deleteProject(id);
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
            const Note previousNote = notes_[i];
            notes_[i] = note;
            if (notes_[i].projectId.isNull()) {
                notes_[i].projectId = previousNote.projectId;
            }
            notes_[i].updatedAt = QDateTime::currentDateTime();
            saveNotes();

            if (previousNote.workspaceId != notes_[i].workspaceId || previousNote.projectId != notes_[i].projectId) {
                removeNoteFromFile(previousNote);
            }

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

            // Cascade delete attachments linked to this note (new model uses linkedEntityType + linkedEntityId)
            for (int j = attachments_.size() - 1; j >= 0; --j) {
                if (attachments_[j].linkedEntityType == AttachmentEntityType::Note &&
                    attachments_[j].linkedEntityId == id) {
                    deleteAttachment(attachments_[j].id);
                    }
            }

            if (m_dbManager && m_dbManager->isOpen()) {
                m_dbManager->deleteNote(id);
            }

            // Persist attachment and note changes
            saveAttachments();
            saveNotes();

            // Remove note file from disk
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
        // Primary criterion: explicit projectId field
        if (att.projectId == projectId) {
            result.append(att);
        }
    }
    return result;
}

QList<FileAttachment> WorkspaceRepository::getAttachmentsByNote(const QUuid& noteId) const {
    QList<FileAttachment> result;
    for (const auto& att : attachments_) {
        // New model: attachment linked to a Note via linkedEntityType + linkedEntityId
        if (att.linkedEntityType == AttachmentEntityType::Note && att.linkedEntityId == noteId) {
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
    newAtt.updatedAt = newAtt.createdAt;


    QString storedFilePath = storeAttachmentFile(newAtt);
    if (!storedFilePath.isEmpty()) {
        newAtt.relativePath = storedFilePath;
        // Update fileSize based on stored file
        QFileInfo fi(storedFilePath);
        newAtt.fileSize = fi.size();
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
            if (m_dbManager && m_dbManager->isOpen()) {
                m_dbManager->deleteAttachment(id);
            }
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

            if (m_dbManager && m_dbManager->isOpen()) {
                m_dbManager->deleteTask(id);
            }

            // LINK: Persist changes immediately
            saveTasks();

            emit taskDeleted(id);
            return;
        }
    }
}

void WorkspaceRepository::migrateFromLegacySettingsIfNecessary()
{
    if (!m_dbManager || !m_dbManager->isOpen()) {
        return;
    }

    if (!m_dbManager->getAllWorkspaces(true).isEmpty()) {
        return;
    }

    const QString iniPath = settingsFilePath();
    if (!QFile::exists(iniPath)) {
        return;
    }

    qInfo() << "Legacy data.ini detected and database is empty. Starting migration to SQLite...";

    QSettings s(iniPath, QSettings::IniFormat);

    m_dbManager->beginTransaction();

    // 1. Workspaces
    int wsCount = s.beginReadArray("workspaces");
    for (int i = 0; i < wsCount; ++i) {
        s.setArrayIndex(i);
        Workspace ws;
        QUuid id = s.value("id").toUuid();
        if (id.isNull()) id = QUuid::fromString(s.value("id").toString());
        if (id.isNull()) id = QUuid::createUuid();
        ws.id = id;
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
        ws.taskCount = std::max(0, s.value("taskCount", 0).toInt());
        ws.noteCount = std::max(0, s.value("noteCount", 0).toInt());
        m_dbManager->addWorkspace(ws);
    }
    s.endArray();

    // 2. Projects
    int projCount = s.beginReadArray("projects");
    for (int i = 0; i < projCount; ++i) {
        s.setArrayIndex(i);
        Project proj;
        QUuid id = s.value("id").toUuid();
        if (id.isNull()) id = QUuid::fromString(s.value("id").toString());
        if (id.isNull()) id = QUuid::createUuid();
        proj.id = id;
        proj.workspaceId = QUuid::fromString(s.value("workspaceId").toString());
        proj.name = s.value("name").toString();
        proj.description = s.value("description").toString();
        proj.isArchived = s.value("isArchived", false).toBool();
        proj.createdAt = QDateTime::fromString(s.value("createdAt").toString(), Qt::ISODate);
        proj.updatedAt = QDateTime::fromString(s.value("updatedAt").toString(), Qt::ISODate);
        m_dbManager->addProject(proj);
    }
    s.endArray();

    // 3. Tasks
    int taskCount = s.beginReadArray("tasks");
    for (int i = 0; i < taskCount; ++i) {
        s.setArrayIndex(i);
        Task task;
        QUuid id = s.value("id").toUuid();
        if (id.isNull()) id = QUuid::fromString(s.value("id").toString());
        if (id.isNull()) id = QUuid::createUuid();
        task.id = id;
        task.workspaceId = QUuid::fromString(s.value("workspaceId").toString());
        task.projectId = QUuid::fromString(s.value("projectId").toString());
        task.title = s.value("title").toString();
        task.description = s.value("description").toString();
        task.status = static_cast<TaskStatus>(s.value("status", static_cast<int>(TaskStatus::Pending)).toInt());
        task.priority = static_cast<TaskPriority>(s.value("priority", static_cast<int>(TaskPriority::Medium)).toInt());
        task.createdAt = QDateTime::fromString(s.value("createdAt").toString(), Qt::ISODate);
        task.dueDate = QDateTime::fromString(s.value("dueDate").toString(), Qt::ISODate);
        task.completedAt = QDateTime::fromString(s.value("completedAt").toString(), Qt::ISODate);

        if (s.contains("subtasks")) {
            const QByteArray raw = s.value("subtasks").toByteArray();
            const QJsonArray stArr = QJsonDocument::fromJson(raw).array();
            for (const auto& val : stArr) {
                const QJsonObject stObj = val.toObject();
                SubTask st;
                st.id = QUuid::fromString(stObj["id"].toString());
                if (st.id.isNull()) st.id = QUuid::createUuid();
                st.taskId = task.id;
                st.title = stObj["title"].toString();
                st.isCompleted = stObj["isCompleted"].toBool();
                task.subtasks.append(st);
            }
        }
        m_dbManager->addTask(task);
    }
    s.endArray();

    // 4. Notes
    int noteCount = s.beginReadArray("notes");
    for (int i = 0; i < noteCount; ++i) {
        s.setArrayIndex(i);
        Note note;
        QUuid id = s.value("id").toUuid();
        if (id.isNull()) id = QUuid::fromString(s.value("id").toString());
        if (id.isNull()) id = QUuid::createUuid();
        note.id = id;
        note.workspaceId = QUuid::fromString(s.value("workspaceId").toString());
        note.projectId = QUuid::fromString(s.value("projectId").toString());
        note.title = s.value("title").toString();
        note.preview = s.value("preview").toString();
        note.isPinned = s.value("isPinned", false).toBool();
        note.isArchived = s.value("isArchived", false).toBool();
        note.createdAt = QDateTime::fromString(s.value("createdAt").toString(), Qt::ISODate);
        note.updatedAt = QDateTime::fromString(s.value("updatedAt").toString(), Qt::ISODate);
        note.content = readNoteContentFromFile(note);
        if (note.content.isEmpty() && s.contains("content")) {
            note.content = s.value("content").toString();
            saveNoteToFile(note);
        }
        if (note.preview.isEmpty()) {
            note.preview = note.content.left(160);
        }
        m_dbManager->addNote(note);
    }
    s.endArray();

    // 5. Attachments
    int attCount = s.beginReadArray("attachments");
    for (int i = 0; i < attCount; ++i) {
        s.setArrayIndex(i);
        FileAttachment att;
        QUuid id = s.value("id").toUuid();
        if (id.isNull()) id = QUuid::fromString(s.value("id").toString());
        if (id.isNull()) id = QUuid::createUuid();
        att.id = id;
        att.workspaceId = QUuid::fromString(s.value("workspaceId").toString());
        att.projectId = QUuid::fromString(s.value("projectId").toString());
        if (s.contains("linkedEntityType")) {
            att.linkedEntityType = static_cast<AttachmentEntityType>(s.value("linkedEntityType").toInt());
            att.linkedEntityId = QUuid::fromString(s.value("linkedEntityId").toString());
        } else {
            QString oldNoteId = s.value("noteId").toString();
            if (!oldNoteId.isEmpty()) {
                att.linkedEntityType = AttachmentEntityType::Note;
                att.linkedEntityId = QUuid::fromString(oldNoteId);
            }
        }
        att.fileName = s.value("fileName").toString();
        att.relativePath = s.value("filePath").toString();
        att.mimeType = s.value("mimeType").toString();
        att.fileSize = s.value("fileSize").toLongLong();
        att.createdAt = QDateTime::fromString(s.value("createdAt").toString(), Qt::ISODate);
        att.updatedAt = QDateTime::fromString(s.value("updatedAt").toString(), Qt::ISODate);
        m_dbManager->addAttachment(att);
    }
    s.endArray();

    m_dbManager->commitTransaction();

    QFile::rename(iniPath, iniPath + ".bak");
    qInfo() << "Migration complete. Legacy data.ini backed up to data.ini.bak.";
}

void WorkspaceRepository::saveWorkspaces()
{
    if (!m_dbManager || !m_dbManager->isOpen()) return;
    m_dbManager->beginTransaction();
    for (const auto& ws : workspaces_) {
        if (m_dbManager->getWorkspace(ws.id).has_value()) {
            m_dbManager->updateWorkspace(ws);
        } else {
            m_dbManager->addWorkspace(ws);
        }
    }
    m_dbManager->commitTransaction();
}

void WorkspaceRepository::loadWorkspaces()
{
    if (m_dbManager && m_dbManager->isOpen()) {
        workspaces_ = m_dbManager->getAllWorkspaces(true);
    }
}

void WorkspaceRepository::saveProjects()
{
    if (!m_dbManager || !m_dbManager->isOpen()) return;
    m_dbManager->beginTransaction();
    for (const auto& proj : projects_) {
        if (m_dbManager->getProject(proj.id).has_value()) {
            m_dbManager->updateProject(proj);
        } else {
            m_dbManager->addProject(proj);
        }
    }
    m_dbManager->commitTransaction();
}

void WorkspaceRepository::loadProjects()
{
    projects_.clear();
    if (m_dbManager && m_dbManager->isOpen()) {
        for (const auto& ws : workspaces_) {
            projects_.append(m_dbManager->getProjectsByWorkspace(ws.id));
        }
    }
}

void WorkspaceRepository::saveTasks()
{
    if (!m_dbManager || !m_dbManager->isOpen()) return;
    m_dbManager->beginTransaction();
    for (const auto& task : tasks_) {
        if (m_dbManager->getTask(task.id).has_value()) {
            m_dbManager->updateTask(task);
        } else {
            m_dbManager->addTask(task);
        }
    }
    m_dbManager->commitTransaction();
}

void WorkspaceRepository::loadTasks()
{
    tasks_.clear();
    if (m_dbManager && m_dbManager->isOpen()) {
        for (const auto& ws : workspaces_) {
            tasks_.append(m_dbManager->getTasksByWorkspace(ws.id));
        }
    }
}

void WorkspaceRepository::saveNotes()
{
    if (!m_dbManager || !m_dbManager->isOpen()) return;
    m_dbManager->beginTransaction();
    for (const auto& note : notes_) {
        if (m_dbManager->getNote(note.id).has_value()) {
            m_dbManager->updateNote(note);
        } else {
            m_dbManager->addNote(note);
        }
    }
    m_dbManager->commitTransaction();
}

void WorkspaceRepository::loadNotes()
{
    notes_.clear();
    if (m_dbManager && m_dbManager->isOpen()) {
        for (const auto& ws : workspaces_) {
            notes_.append(m_dbManager->getNotesByWorkspace(ws.id));
        }
    }

    for (auto& note : notes_) {
        if (note.content.isEmpty()) {
            note.content = readNoteContentFromFile(note);
            if (!note.content.isEmpty() && m_dbManager) {
                m_dbManager->updateNote(note);
            }
        } else {
            saveNoteToFile(note);
        }
        if (note.preview.isEmpty()) {
            note.preview = note.content.left(160);
        }
    }
}

void WorkspaceRepository::saveAttachments()
{
    if (!m_dbManager || !m_dbManager->isOpen()) return;
    m_dbManager->beginTransaction();
    for (const auto& att : attachments_) {
        if (m_dbManager->getAttachment(att.id).has_value()) {
            m_dbManager->updateAttachment(att);
        } else {
            m_dbManager->addAttachment(att);
        }
    }
    m_dbManager->commitTransaction();
}

void WorkspaceRepository::loadAttachments()
{
    attachments_.clear();
    if (m_dbManager && m_dbManager->isOpen()) {
        for (const auto& ws : workspaces_) {
            attachments_.append(m_dbManager->getAttachmentsByWorkspace(ws.id));
        }
    }
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

QString WorkspaceRepository::dataRootPath() {
    const QString base = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(base);
    const QString fullPath = QDir(base).filePath("TaskHelperData/workspace");
    QDir().mkpath(fullPath);
    return fullPath;
}

QString WorkspaceRepository::settingsFilePath() const {
    return QDir(dataRootPath()).filePath("data.ini");
}

void WorkspaceRepository::initializeSchema() {
    QSettings s(settingsFilePath(), QSettings::IniFormat);
    const int currentVersion = 1;
    if (!s.contains("schemaVersion")) {
        s.setValue("schemaVersion", currentVersion);
        s.sync();
    } else {
        int version = s.value("schemaVersion", 0).toInt();
        if (version < currentVersion) {
            // Future migration hook can be dispatched here as format versions change
            s.setValue("schemaVersion", currentVersion);
            s.sync();
        }
    }
}

QString WorkspaceRepository::projectPath(const QUuid &workspaceId, const QUuid &projectId) const {
    QDir root(dataRootPath());
    return root.filePath(uuidKey(workspaceId) + "/projects/" + uuidKey(projectId));
}

bool WorkspaceRepository::ensureProjectDir(const QUuid &workspaceId, const QUuid &projectId) const {
    QDir d(projectPath(workspaceId, projectId));
    return d.mkpath(".") && d.mkpath("tasks") && d.mkpath("notes") && d.mkpath("attachments");

}

QString WorkspaceRepository::noteFilePath(const Note& note) const {
    const QString notePath = QDir(projectPath(note.workspaceId, note.projectId)).filePath("notes");
    return QDir(notePath).filePath(uuidKey(note.id) + ".md");
}

QString WorkspaceRepository::readNoteContentFromFile(const Note& note) const {
    QFile file(noteFilePath(note));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {};
    }

    return QString::fromUtf8(file.readAll());
}

void WorkspaceRepository::saveNoteToFile(const Note& note) const {
    // LINK: Save note content to disk as markdown file atomically
    QDir notesDir(QDir(projectPath(note.workspaceId, note.projectId)).filePath("notes"));
    if (!notesDir.exists() && !notesDir.mkpath(".")) {
        qWarning() << "Failed to create notes directory:" << notesDir.path();
        return;
    }
    const QString noteFile = noteFilePath(note);

    QSaveFile file(noteFile);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Failed to open note file for atomic write:" << noteFile << file.errorString();
        return;
    }

    QByteArray data = note.content.toUtf8();
    qint64 bytesWritten = file.write(data);
    if (bytesWritten != data.size()) {
        qWarning() << "Incomplete note write (" << bytesWritten << "of" << data.size() << "bytes), canceling:" << noteFile;
        file.cancelWriting();
        return;
    }

    if (!file.commit()) {
        qWarning() << "Failed to commit atomic note file:" << noteFile << file.errorString();
    }
}

void WorkspaceRepository::removeNoteFromFile(const Note& note) const {
    // LINK: Remove note file from disk
    QFile::remove(noteFilePath(note));
}

QString WorkspaceRepository::storeAttachmentFile(const FileAttachment& attachment) const {
    // LINK: Copy/move attachment file to project attachment storage
    if (attachment.relativePath.isEmpty()) {
        return "";
    }

    QFileInfo sourceInfo(attachment.relativePath);
    if (!sourceInfo.exists() || !sourceInfo.isFile()) {
        qWarning() << "Attachment source does not exist or is not a file:" << attachment.relativePath;
        return "";
    }

    // Sanitize source filename to prevent directory traversal or invalid characters
    QString cleanFileName = sourceInfo.fileName();
    cleanFileName.replace(QRegularExpression("[/\\\\?%*:|\"<>]"), "_");
    if (cleanFileName.startsWith(".")) {
        cleanFileName.prepend("_");
    }
    if (cleanFileName.isEmpty()) {
        cleanFileName = "attachment_" + uuidKey(attachment.id);
    }

    // Create stored path: <project>/attachments/<attachmentUuid>/<sanitizedFileName>
    QString attStorePath = QDir(projectPath(attachment.workspaceId, attachment.projectId))
        .filePath("attachments/" + uuidKey(attachment.id));

    QDir attStoreDir(attStorePath);
    if (!attStoreDir.mkpath(".")) {
        qWarning() << "Failed to create attachment directory:" << attStorePath;
        return "";
    }

    QString storedFile = QDir(attStorePath).filePath(cleanFileName);

    // If destination exists, handle collision or re-storing
    if (QFile::exists(storedFile)) {
        if (QFileInfo(storedFile).canonicalFilePath() == sourceInfo.canonicalFilePath()) {
            return storedFile;
        }
        QFile::remove(storedFile);
    }

    // Copy file to storage location
    if (QFile::copy(attachment.relativePath, storedFile)) {
        return storedFile;
    }

    qWarning() << "Failed to copy attachment file from" << attachment.relativePath << "to" << storedFile;
    return "";
}

void WorkspaceRepository::removeAttachmentFile(const FileAttachment& attachment) const {
    // LINK: Remove attachment file and folder from disk
    if (attachment.relativePath.isEmpty()) {
        return;
    }

    QFile::remove(attachment.relativePath);

    // Verify attFolderPath is strictly inside dataRootPath before recursive deletion
    QString attFolderPath = QDir(projectPath(attachment.workspaceId, attachment.projectId))
        .filePath("attachments/" + uuidKey(attachment.id));

    QString rootCanonical = QDir(dataRootPath()).canonicalPath();
    QString folderCanonical = QDir(attFolderPath).canonicalPath();

    if (!rootCanonical.isEmpty() && !folderCanonical.isEmpty() &&
        folderCanonical.startsWith(rootCanonical) && folderCanonical != rootCanonical) {
        QDir(attFolderPath).removeRecursively();
    }
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
    QString rootCanonical = QDir(dataRootPath()).canonicalPath();
    QString parentCanonical = parentDir.canonicalPath();

    if (!rootCanonical.isEmpty() && !parentCanonical.isEmpty() &&
        parentCanonical.startsWith(rootCanonical) && parentCanonical != rootCanonical) {
        if (parentDir.entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot).isEmpty()) {
            parentDir.removeRecursively();
        }
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
    if (workspaceId.isNull()) {
        return;
    }

    QString rootCanonical = QDir(dataRootPath()).canonicalPath();
    QDir workspaceDir(QDir(dataRootPath()).filePath(uuidKey(workspaceId)));
    QString wsCanonical = workspaceDir.canonicalPath();

    if (!rootCanonical.isEmpty() && !wsCanonical.isEmpty() &&
        wsCanonical.startsWith(rootCanonical) && wsCanonical != rootCanonical) {
        workspaceDir.removeRecursively();
    }
}


void WorkspaceRepository::cleanUpOrphanedData()
{
    deleteAllOrphanedFiles();
}

//
// Created by DevAccount on 28/01/2026.
//

#ifndef TASKHELPER_WORKSPACEREPOSITORY_H
#define TASKHELPER_WORKSPACEREPOSITORY_H
#pragma once
#include <QObject>
#include <memory>
#include "helpers/Workspace.h"
#include "Structure/Project.h"
#include "Structure/Task.h"
#include "Structure/Note.h"
#include "Structure/FileAttachment.h"

class DatabaseManager;

/**
 * @file WorkspaceRepository.h
 * @brief Aggregate repository coordinating workspaces, projects, tasks, notes, and attachments.
 */

/**
 * @struct OrphanedFileInfo
 * @brief Metadata about an orphaned file discovered on disk that lacks a database record.
 */
struct OrphanedFileInfo {
    QString filePath;       ///< Absolute filesystem path to the orphaned file.
    QString fileName;       ///< Basename of the file.
    QString Type;           ///< Categorical type (e.g. note, attachment).
    QDateTime lastModified; ///< Timestamp when the file was last modified.
    qint64 fileSize;        ///< File size in bytes.
};

/**
 * @struct OrphanedFileReport
 * @brief Summary report of orphaned files found across workspace directories.
 */
struct OrphanedFileReport {
    QList<OrphanedFileInfo> orphanedFiles; ///< Collection of orphaned file entries.
    qint64 totalSize;                     ///< Aggregate size of all orphaned files in bytes.
    int totalFiles;                       ///< Total count of orphaned files found.
};

/**
 * @class WorkspaceRepository
 * @brief Central domain repository managing entity lifecycles, file persistence, and reactive signals.
 *
 * WorkspaceRepository coordinates in-memory data structures, SQLite database storage via
 * DatabaseManager, and atomic disk persistence for markdown notes and attached assets.
 * It provides a comprehensive event-driven API via Qt signals to inform UI and manager
 * components of state mutations.
 */
class WorkspaceRepository : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Constructs the repository, initializes the SQLite database, and loads persisted data.
     * @param parent Optional parent QObject for ownership hierarchy.
     */
    explicit WorkspaceRepository(QObject* parent = nullptr);

    /**
     * @brief Destructor. Flushes pending changes and releases resources.
     */
    ~WorkspaceRepository() override;

    // =========================================================================
    // Workspace Management
    // =========================================================================

    /**
     * @brief Returns a copy of all loaded workspaces.
     */
    [[nodiscard]] QList<Workspace> workspaces() const;

    /**
     * @brief Retrieves a workspace by its unique identifier.
     * @param id Workspace UUID.
     * @return Workspace instance if found; empty Workspace with null UUID otherwise.
     */
    [[nodiscard]] Workspace getWorkspaceById(const QUuid& id) const;

    /**
     * @brief Creates and persists a new workspace.
     * @param name Name of the workspace.
     * @param type Category/type string (e.g., "work", "personal", "custom").
     * @param description Optional workspace description.
     * @return Unique identifier of the newly created workspace.
     */
    QUuid createWorkspace(const QString& name, const QString& type = "custom", const QString& description = "");

    /**
     * @brief Updates workspace metadata and notifies subscribers.
     * @param ws Workspace containing updated values matching `ws.id`.
     */
    void updateWorkspace(const Workspace& ws);

    /**
     * @brief Deletes a workspace and its associated projects, tasks, notes, and files.
     * @param id Identifier of the workspace to delete.
     */
    void deleteWorkspace(const QUuid& id);

    // =========================================================================
    // Project Management
    // =========================================================================

    /**
     * @brief Returns all projects belonging to a given workspace.
     * @param workspaceId Workspace UUID.
     */
    [[nodiscard]] QList<Project> getProjectsByWorkspace(const QUuid& workspaceId) const;

    /**
     * @brief Retrieves a single project by its ID.
     * @param id Project UUID.
     */
    [[nodiscard]] Project getProjectById(const QUuid& id) const;

    /**
     * @brief Creates a new project in the repository and provisions its directory structure.
     * @param project Project domain model.
     * @return Generated Project UUID.
     */
    QUuid createProject(const Project& project);

    /**
     * @brief Updates project details and emits projectUpdated.
     * @param project Project with modified fields.
     */
    void updateProject(const Project& project);

    /**
     * @brief Removes a project and cascades deletion to child tasks, notes, and attachments.
     * @param id Identifier of the project to remove.
     */
    void deleteProject(const QUuid& id);

    // =========================================================================
    // Task Management
    // =========================================================================

    /**
     * @brief Retrieves all tasks residing in a workspace.
     * @param workspaceId Workspace UUID.
     */
    [[nodiscard]] QList<Task> getTasksByWorkspace(const QUuid& workspaceId) const;

    /**
     * @brief Retrieves all tasks assigned to a specific project.
     * @param projectId Project UUID.
     */
    [[nodiscard]] QList<Task> getTasksByProject(const QUuid& projectId) const;

    /**
     * @brief Looks up a task by its unique identifier.
     * @param id Task UUID.
     */
    [[nodiscard]] Task getTaskById(const QUuid& id) const;

    /**
     * @brief Creates and stores a new task.
     * @param task Task domain model.
     * @return Generated Task UUID.
     */
    QUuid createTask(const Task& task);

    /**
     * @brief Updates an existing task record.
     * @param task Task domain model.
     */
    void updateTask(const Task& task);

    /**
     * @brief Deletes a task by ID.
     * @param id Task UUID.
     */
    void deleteTask(const QUuid& id);

    // =========================================================================
    // Note Management
    // =========================================================================

    /**
     * @brief Retrieves all notes associated with a workspace.
     * @param workspaceId Workspace UUID.
     */
    [[nodiscard]] QList<Note> getNotesByWorkspace(const QUuid& workspaceId) const;

    /**
     * @brief Retrieves all notes linked to a project.
     * @param projectId Project UUID.
     */
    [[nodiscard]] QList<Note> getNotesByProject(const QUuid& projectId) const;

    /**
     * @brief Retrieves a note by its ID, reading its latest body content from disk.
     * @param id Note UUID.
     */
    [[nodiscard]] Note getNoteById(const QUuid& id) const;

    /**
     * @brief Creates a new note, writes its initial markdown content to disk, and updates the database.
     * @param note Note domain model.
     * @return Generated Note UUID.
     */
    QUuid createNote(const Note& note);

    /**
     * @brief Updates note properties and atomically writes revised content to disk.
     * @param note Note domain model.
     */
    void updateNote(const Note& note);

    /**
     * @brief Removes a note from the repository and removes its markdown file from disk.
     * @param id Note UUID.
     */
    void deleteNote(const QUuid& id);

    // =========================================================================
    // FileAttachment Management
    // =========================================================================

    /**
     * @brief Returns all file attachments belonging to a workspace.
     * @param workspaceId Workspace UUID.
     */
    [[nodiscard]] QList<FileAttachment> getAttachmentsByWorkspace(const QUuid& workspaceId) const;

    /**
     * @brief Returns all file attachments belonging to a project.
     * @param projectId Project UUID.
     */
    [[nodiscard]] QList<FileAttachment> getAttachmentsByProject(const QUuid& projectId) const;

    /**
     * @brief Returns all file attachments linked to a specific note.
     * @param noteId Note UUID.
     */
    [[nodiscard]] QList<FileAttachment> getAttachmentsByNote(const QUuid& noteId) const;

    /**
     * @brief Looks up attachment metadata by its unique ID.
     * @param id FileAttachment UUID.
     */
    [[nodiscard]] FileAttachment getAttachmentById(const QUuid& id) const;

    /**
     * @brief Copies or registers a file attachment into project storage and records metadata.
     * @param attachment FileAttachment domain model.
     * @return Generated Attachment UUID.
     */
    QUuid createAttachment(const FileAttachment& attachment);

    /**
     * @brief Deletes an attachment metadata record and cleans up the stored file on disk.
     * @param id Attachment UUID.
     */
    void deleteAttachment(const QUuid& id);

    // =========================================================================
    // Orphaned File Maintenance
    // =========================================================================

    /**
     * @brief Scans workspace project directories for files missing corresponding database entries.
     */
    [[nodiscard]] OrphanedFileReport scanForOrphanedFiles() const;

    /**
     * @brief Scans note storage for untracked markdown files.
     */
    void scanOrphanedNotes(const QUuid& workspaceId, const QUuid& projectId, OrphanedFileReport& report) const;

    /**
     * @brief Scans attachment storage for untracked asset files.
     */
    void scanOrphanedAttachments(const QUuid& workspaceId, const QUuid& projectId, OrphanedFileReport& report) const;

    /**
     * @brief Deletes a list of orphaned file paths permanently.
     */
    static void deleteOrphanedFiles(const QList<QString>& filePaths);

    /**
     * @brief Deletes a single orphaned file path permanently.
     */
    static void deleteOrphanedFile(const QString& filePath);

    /**
     * @brief Scans and purges all orphaned files detected across all workspaces.
     */
    void deleteAllOrphanedFiles();

    /**
     * @brief Cleans up filesystem directories and orphaned assets for a deleted workspace.
     * @param workspaceId Target workspace UUID.
     */
    void cleanUpOrphanedDataForWorkspace(const QUuid& workspaceId);

signals:
    void workspaceAdded(const Workspace& ws);
    void workspaceUpdated(const Workspace& ws);
    void projectAdded(const Project& project);
    void projectUpdated(const Project& project);
    void projectDeleted(const QUuid& projectId);
    void taskAdded(const Task& task);
    void taskUpdated(const Task& task);
    void taskDeleted(const QUuid& taskId);
    void noteAdded(const Note& note);
    void noteUpdated(const Note& note);
    void noteDeleted(const QUuid& noteId);
    void attachmentAdded(const FileAttachment& attachment);
    void attachmentDeleted(const QUuid& attachmentId);

private:
    QList<Workspace> workspaces_;
    QList<Project> projects_;
    QList<Task> tasks_;
    QList<Note> notes_;
    QList<FileAttachment> attachments_;
    std::unique_ptr<DatabaseManager> m_dbManager;

    // Legacy migration
    void migrateFromLegacySettingsIfNecessary();

    // Persistence
    void saveWorkspaces();
    void loadWorkspaces();
    void saveProjects();
    void loadProjects();
    void saveTasks();
    void loadTasks();
    void saveNotes();
    void loadNotes();
    void saveAttachments();
    void loadAttachments();
    void initializeSchema();
    void ensureProjectStructure();
    [[nodiscard]] QUuid defaultProjectForWorkspace(const QUuid& workspaceId) const;

    static QString uuidKey(const QUuid& id) { return id.toString(QUuid::WithoutBraces); }

    [[nodiscard]] static QString dataRootPath();
    [[nodiscard]] QString settingsFilePath() const;

    [[nodiscard]] QString projectPath(const QUuid& workspaceId, const QUuid& projectId) const;
    [[nodiscard]] bool ensureProjectDir(const QUuid& workspaceId, const QUuid& projectId) const;

    // File-based persistence helpers
    [[nodiscard]] QString noteFilePath(const Note& note) const;
    [[nodiscard]] QString readNoteContentFromFile(const Note& note) const;
    void saveNoteToFile(const Note& note) const;
    void removeNoteFromFile(const Note& note) const;
    [[nodiscard]] QString storeAttachmentFile(const FileAttachment& attachment) const;
    void removeAttachmentFile(const FileAttachment& attachment) const;

    void cleanUpOrphanedData();
};

#endif //TASKHELPER_WORKSPACEREPOSITORY_H
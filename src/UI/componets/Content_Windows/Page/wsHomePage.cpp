//
// Created by DevAccount on 15/04/2026.
//
#include "UI/components/Content_Windows/page/wsHomePage.h"
#include <QVBoxLayout>
#include <QQmlContext>
#include <QDebug>
#include <QLabel>
#include <algorithm>
#include <QFileDialog>


wsHomePage::wsHomePage(const Workspace& ws,WorkspaceRepository* repo ,QWidget* parent)
    : IWorkspaceView(ws, parent), m_workspace(ws), m_repo(repo)
{
    if (!m_repo) {
        return;
    }

    workspaceId_ = ws.id;

    // Initialise Managers
    m_taskManager = new TaskManager(m_repo, this);
    m_noteManager = new NoteManager(m_repo, this);

    //connect signals
    connect(m_taskManager, &TaskManager::tasksChanged, this, [this](const QUuid& wsId) {
        if (wsId == workspaceId_) {
            populateData();
        }
    });

    connect(m_noteManager, &NoteManager::notesChanged, this, [this](const QUuid& wsId) {
        if (wsId == workspaceId_) {
            populateData();
        }
    });

    setupUi();
    populateData();


}

void wsHomePage::refresh()
{
    populateData();
}

void wsHomePage::updateWorkspace(const Workspace& ws)
{
    m_workspace = ws;
    workspaceId_ = ws.id;
    if (!m_activeProject.id.isNull()) {
        m_activeProject = Project{};
        emit activeProjectChanged();
    }
    populateData();
    emit workspaceNameChanged();
}

void wsHomePage::setActiveProject(const Project& project)
{

    if (project.workspaceId != workspaceId_) {
        return;
    }

    if (m_activeProject.id == project.id) {
        return;
    }

    m_activeProject = project;
    emit activeProjectChanged();
    populateData();                 // reload task/note/progress lists with project filter
    if (m_homeQuickView) {
        m_homeQuickView->update();  // optional, binding refresh hint
    }
}

void wsHomePage::createNewTask(const QString& title)
{
    if (!m_taskManager || title.trimmed().isEmpty())
    {
        return;
    }

    TaskCreateRequest request;
    request.workspaceId = workspaceId_;
    request.title = title.trimmed();
    request.projectId = m_activeProject.id;
    request.priority = TaskPriority::Medium;

    m_taskManager->createTask(request);
}

void wsHomePage::createNewNote(const QString& title)
{
    if (!m_noteManager || title.trimmed().isEmpty())
    {
        return;
    }

    const QString trimmedTitle = title.trimmed();
    if (trimmedTitle.isEmpty())
    {
        return;
    }

    NoteCreateRequest request;
    request.title = trimmedTitle;
    request.content = "";
    request.workspaceId = workspaceId_;
    request.projectId = m_activeProject.id;
    request.isPinned = false;


    m_noteManager->createNote(request);
}

void wsHomePage::uploadFile(const QString& filePath)
{
    // TODO: Implement file upload logic, potentially creating a new note with the file attachment or linking it to an existing note/task
    //This would involve creating a FileAttachment via the repositor

    emit uploadMessage(tr("File uploaded successfully: %1").arg(QFileInfo(filePath).fileName()), false);
    
}

void wsHomePage::requestUploadFile()
{
    const QString selected = QFileDialog::getOpenFileName(
      this,
      tr("Select File to Upload"),
      QString(),
      tr("Allowed Files (*.pdf *.png *.jpg *.jpeg *.txt *.md *.docx *.xlsx);;All Files (*.*)")
    );

    if (selected.isEmpty()) {
        emit uploadMessage(tr("File selection cancelled."), true);
        return;
    }

    uploadFile(selected);
}

void wsHomePage::deleteNote(const QString& noteId)
{
    if (!m_noteManager || noteId.isEmpty())
    {
        return;
    }

    const QUuid id = QUuid::fromString(noteId);
    m_noteManager->deleteNote(id);
}

void wsHomePage::openUploadDialog()
{
    if (!m_uploadDialog) {
        m_uploadDialog = new UploadDialog(this);
        m_uploadDialog->setFileFilter(
            tr("Allowed Files (*.pdf *.png *.jpg *.jpeg *.txt *.md *.docx *.xlsx);;All Files (*.*)")
        );

        connect(m_uploadDialog, &UploadDialog::uploadRequested,
                this, &wsHomePage::onUploadRequested);
    }

    // Keep context current in case workspace/project changed since dialog creation
    m_uploadDialog->setWorkspaceId(workspaceId_);
    m_uploadDialog->setProjectId(m_activeProject.id);

    m_uploadDialog->show();
    m_uploadDialog->raise();
    m_uploadDialog->activateWindow();

}

void wsHomePage::toggleTaskCompletion(const QString& taskId, bool isCompleted)
{
    if (!m_taskManager || taskId.isEmpty())
    {
        return;
    }

    const QUuid id = QUuid::fromString(taskId);
    m_taskManager->setCompleted(id, isCompleted);
}

void wsHomePage::openNote(const QString& noteId)
{
    if (noteId.isEmpty())
    {
        return;
    }

    const QUuid id = QUuid::fromString(noteId);
    Note note = m_repo->getNoteById(id);
    if (note.id.isNull())
    {
        emit uploadMessage(tr("Note not found."), true);
        return;
    }

    emit noteOpenRequested(noteId);


}


void wsHomePage::populateData()
{
    if (!m_taskManager || !m_noteManager || !m_repo)
    {
        return;
    }

    const bool hasActiveProject = !m_activeProject.id.isNull();
    const QUuid activeProjectId = m_activeProject.id;

    {
        m_recentNotes.clear();

        QList<Note> notes;
        if (hasActiveProject) {
            notes = m_repo->getNotesByProject(activeProjectId);
            notes.erase(std::remove_if(notes.begin(), notes.end(), [](const Note& n) {
                return n.isArchived;
            }), notes.end());
            std::sort(notes.begin(), notes.end(), [](const Note& a, const Note& b) {
                const QDateTime aKey = a.updatedAt.isValid() ? a.updatedAt : a.createdAt;
                const QDateTime bKey = b.updatedAt.isValid() ? b.updatedAt : b.createdAt;
                return aKey > bKey;
            });
            if (notes.size() > 5) {
                notes = notes.mid(0, 5);
            }
        } else {
            notes = m_noteManager->recentNotes(workspaceId_, 5);
        }

        for (const Note& note : notes)
        {
            QVariantMap noteData;
            noteData["id"] = note.id.toString(QUuid::WithoutBraces);
            noteData["title"] = note.title;
            noteData["preview"] = note.preview.isEmpty() ? note.content.left(100) : note.preview;
            noteData["isPinned"] = note.isPinned;
            noteData["updatedAt"] = note.updatedAt;
            m_recentNotes.append(noteData);

        }

        emit recentNotesChanged();

    }

    {
        m_todayTasks.clear();

        QList<Task> tasks;
        if (hasActiveProject) {
            const QList<Task> projectTasks = m_repo->getTasksByProject(activeProjectId);
            const QDate today = QDate::currentDate();
            for (const Task& task : projectTasks) {
                if (task.status == TaskStatus::Completed || task.status == TaskStatus::Archived) {
                    continue;
                }
                if (task.dueDate.isValid() && task.dueDate.date() == today) {
                    tasks.append(task);
                }
            }
        } else {
            tasks = m_taskManager->todayTasks(workspaceId_);
        }

        for (const Task& task : tasks)
        {
            QVariantMap taskData;
            taskData["id"] = task.id.toString(QUuid::WithoutBraces);
            taskData["title"] = task.title;
            taskData["description"] = task.description;
            taskData["priority"] = QVariant::fromValue(task.priority);
            taskData["dueDate"] = task.dueDate;
            taskData["status"] = QVariant::fromValue(task.status);
            m_todayTasks.append(taskData);
        }

        emit todayTasksChanged();
    }

    {
        m_overdueTasks.clear();

        QList<Task> tasks;
        if (hasActiveProject) {
            const QList<Task> projectTasks = m_repo->getTasksByProject(activeProjectId);
            const QDateTime now = QDateTime::currentDateTime();
            for (const Task& task : projectTasks) {
                if (task.status == TaskStatus::Completed || task.status == TaskStatus::Archived) {
                    continue;
                }
                if (task.dueDate.isValid() && task.dueDate < now) {
                    tasks.append(task);
                }
            }
        } else {
            tasks = m_taskManager->overdueTasks(workspaceId_);
        }

        for (const Task& task : tasks)
        {
            QVariantMap taskData;
            taskData["id"] = task.id.toString(QUuid::WithoutBraces);
            taskData["title"] = task.title;
            taskData["description"] = task.description;
            taskData["priority"] = QVariant::fromValue(task.priority);
            taskData["dueDate"] = task.dueDate;
            taskData["status"] = QVariant::fromValue(task.status);
            m_overdueTasks.append(taskData);
        }

        emit overdueTasksChanged();

    }

    {
        m_projectProgress.clear();
        const QList<Task> allTasks = hasActiveProject
            ? m_repo->getTasksByProject(activeProjectId)
            : m_taskManager->allTasks(workspaceId_);
        int completed = 0, pending = 0, inProgress = 0;
        for (const Task& task : allTasks) {
            if (task.status == TaskStatus::Completed) {
                completed++;
            } else if (task.status == TaskStatus::InProgress) {
                inProgress++;
            } else {
                pending++;
            }
        }
        QVariantMap progressData;
        progressData["completed"] = completed;
        progressData["inProgress"] = inProgress;
        progressData["pending"] = pending;
        progressData["total"] = allTasks.size();

        if (allTasks.size() > 0)
        {
            progressData["completedPercent"] = (completed * 100) / allTasks.size();
            progressData["inProgressPercent"] = (inProgress * 100) / allTasks.size();
            progressData["pendingPercent"] = (pending * 100) / allTasks.size();
        } else {
            progressData["completedPercent"] = 0;
            progressData["inProgressPercent"] = 0;
            progressData["pendingPercent"] = 0;
        }

        m_projectProgress.append(progressData);
        emit projectProgressChanged();



    }

}

void wsHomePage::setupUi()
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(70, 20, 50, 20);
    layout->setSpacing(15);

    m_homeQuickView = new QQuickWidget(this);
    m_homeQuickView->setResizeMode(QQuickWidget::SizeRootObjectToView);


    m_homeQuickView->setClearColor(Qt::transparent);

    m_homeQuickView->setAttribute(Qt::WA_TranslucentBackground);

    m_homeQuickView->setAttribute(Qt::WA_AlwaysStackOnTop);

    m_homeQuickView->setMinimumHeight(300);

    m_homeQuickView->rootContext()->setContextProperty("wsHomePage", this);
    m_homeQuickView->setSource(QUrl("qrc:/qml/wsHomeLinker.qml"));

    if (m_homeQuickView->status() == QQuickWidget::Error) {
        const auto errs = m_homeQuickView->errors();
        for (const auto& err : errs) {
            qWarning() << err.toString();
        }
    }

    layout->addWidget(m_homeQuickView, 1);
}

QString wsHomePage::workspaceName() const
{
    return m_workspace.name;
}

QString wsHomePage::activeProjectName() const
{
    if (m_activeProject.id.isNull()) {
        return QStringLiteral("General");
    }
    return m_activeProject.name;
}

QVariantList wsHomePage::recentNotes() const
{
    return m_recentNotes;
}

QVariantList wsHomePage::todayTasks() const
{
    return m_todayTasks;
}

QVariantList wsHomePage::overdueTasks() const
{
    return m_overdueTasks;
}

QVariantList wsHomePage::projectProgress() const
{
    return m_projectProgress;
}

void wsHomePage::onUploadRequested(const QStringList& sourcePaths, const QUuid& workspaceId, const QUuid& projectId)
{
    Q_UNUSED(workspaceId);
    Q_UNUSED(projectId);

    if (sourcePaths.isEmpty()) {
        emit uploadMessage(tr("No files selected for upload."), true);
        return;
    }

    for (const QString& filePath : sourcePaths) {
        // For now this uses your existing upload entry point.
        // Later, inside uploadFile() call AttachmentManager for secure validation/store.
        uploadFile(filePath);
    }
}

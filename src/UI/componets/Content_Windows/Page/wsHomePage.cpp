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
#include <QRegularExpression>

static QString colorForProject(const QUuid& projectId)
{
    if (projectId.isNull()) return "#81C784";
    static const QStringList palette = {
        "#81C784", "#FFD700", "#9ACD32", "#20B2AA", "#FF69B4", "#64B5F6", "#BA68C8", "#FF8A65"
    };
    return palette.at(qAbs(qHash(projectId.toString())) % palette.size());
}

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

    connect(m_repo, &WorkspaceRepository::projectAdded, this, [this]() { populateData(); });
    connect(m_repo, &WorkspaceRepository::projectDeleted, this, [this]() { populateData(); });
    connect(m_repo, &WorkspaceRepository::projectUpdated, this, [this]() { populateData(); });

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

    QUuid projId = m_activeProject.id;
    if (projId.isNull() && m_repo) {
        const auto projects = m_repo->getProjectsByWorkspace(workspaceId_);
        if (!projects.isEmpty()) {
            projId = projects.first().id;
        }
    }
    request.projectId = projId;
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

    QUuid projId = m_activeProject.id;
    if (projId.isNull() && m_repo) {
        const auto projects = m_repo->getProjectsByWorkspace(workspaceId_);
        if (!projects.isEmpty()) {
            projId = projects.first().id;
        }
    }
    request.projectId = projId;
    request.isPinned = false;

    const QUuid newId = m_noteManager->createNote(request);
    if (!newId.isNull()) {
        emit noteOpenRequested(newId.toString(QUuid::WithoutBraces));
    }
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

void wsHomePage::togglePinNote(const QString& noteId)
{
    if (!m_noteManager || noteId.isEmpty())
    {
        return;
    }

    const QUuid id = QUuid::fromString(noteId);
    if (id.isNull()) return;

    const Note note = m_noteManager->getNoteById(id);
    if (!note.id.isNull()) {
        m_noteManager->setPinned(id, !note.isPinned);
    }
}

void wsHomePage::deleteTask(const QString& taskId)
{
    if (!m_taskManager || taskId.isEmpty())
    {
        return;
    }

    const QUuid id = QUuid::fromString(taskId);
    if (!id.isNull()) {
        m_taskManager->deleteTask(id);
    }
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

    // 1. RECENT NOTES (Workspace-wide)
    {
        m_recentNotes.clear();

        QList<Note> allNotes = m_repo->getNotesByWorkspace(workspaceId_);
        allNotes.erase(std::remove_if(allNotes.begin(), allNotes.end(), [](const Note& n) {
            return n.isArchived;
        }), allNotes.end());

        std::sort(allNotes.begin(), allNotes.end(), [](const Note& a, const Note& b) {
            const QDateTime aKey = a.updatedAt.isValid() ? a.updatedAt : a.createdAt;
            const QDateTime bKey = b.updatedAt.isValid() ? b.updatedAt : b.createdAt;
            return aKey > bKey;
        });

        m_totalNotesCount = allNotes.size();

        const QList<Note> displayNotes = allNotes.mid(0, 6);
        const QDateTime now = QDateTime::currentDateTime();

        for (const Note& note : displayNotes)
        {
            QVariantMap noteData;
            noteData["id"] = note.id.toString(QUuid::WithoutBraces);
            noteData["title"] = note.title.isEmpty() ? tr("Untitled Note") : note.title;

            QString previewText = note.preview;
            if (previewText.isEmpty() && !note.content.isEmpty()) {
                previewText = note.content.left(140).trimmed();
                previewText.replace('\n', ' ');
            }
            noteData["preview"] = previewText;

            // Word count
            int words = 0;
            if (!note.content.trimmed().isEmpty()) {
                words = note.content.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts).size();
            }
            QString wordCountStr = words >= 1000
                ? QString("%1k words").arg(QString::number(words / 1000.0, 'f', 1))
                : QString("%1 words").arg(words);
            noteData["wordCountFormatted"] = wordCountStr;

            // Relative time
            const qint64 secs = note.updatedAt.secsTo(now);
            QString timeStr;
            if (secs < 60) {
                timeStr = tr("Just now");
            } else if (secs < 3600) {
                timeStr = tr("%1m ago").arg(secs / 60);
            } else if (secs < 86400) {
                timeStr = tr("%1h ago").arg(secs / 3600);
            } else if (secs < 86400 * 2) {
                timeStr = tr("Yesterday");
            } else if (secs < 86400 * 7) {
                timeStr = tr("%1d ago").arg(secs / 86400);
            } else {
                timeStr = note.updatedAt.toString("MMM d, yyyy");
            }
            noteData["updatedAtFormatted"] = timeStr;
            noteData["isPinned"] = note.isPinned;

            if (!note.projectId.isNull() && m_repo) {
                const Project p = m_repo->getProjectById(note.projectId);
                noteData["projectName"] = p.name.isEmpty() ? tr("General") : p.name;
                noteData["projectColor"] = colorForProject(note.projectId);
            } else {
                noteData["projectName"] = tr("General");
                noteData["projectColor"] = "#81C784";
            }

            m_recentNotes.append(noteData);
        }

        emit recentNotesChanged();
    }

    // 2. TODAY'S TASKS (Workspace-wide)
    {
        m_todayTasks.clear();
        const QList<Task> tasks = m_taskManager->todayTasks(workspaceId_);
        m_tasksDueTodayCount = tasks.size();
        int remaining = 0;

        for (const Task& task : tasks)
        {
            QVariantMap taskData;
            taskData["id"] = task.id.toString(QUuid::WithoutBraces);
            taskData["title"] = task.title.isEmpty() ? tr("Untitled Task") : task.title;
            taskData["description"] = task.description;

            QString prioStr = "Medium";
            if (task.priority == TaskPriority::High || task.priority == TaskPriority::Critical) prioStr = "High";
            else if (task.priority == TaskPriority::Low) prioStr = "Low";
            taskData["priority"] = prioStr;

            const bool isComp = (task.status == TaskStatus::Completed);
            taskData["isCompleted"] = isComp;
            if (!isComp) {
                remaining++;
            }

            if (task.dueDate.isValid()) {
                taskData["dueTimeFormatted"] = task.dueDate.toString("h:mm AP");
            } else {
                taskData["dueTimeFormatted"] = tr("Today");
            }

            if (!task.projectId.isNull() && m_repo) {
                const Project p = m_repo->getProjectById(task.projectId);
                taskData["projectName"] = p.name.isEmpty() ? tr("General") : p.name;
                taskData["projectColor"] = colorForProject(task.projectId);
            } else {
                taskData["projectName"] = tr("General");
                taskData["projectColor"] = "#81C784";
            }

            m_todayTasks.append(taskData);
        }

        m_tasksRemainingCount = remaining;
        emit todayTasksChanged();
    }

    // 3. OVERDUE TASKS (Workspace-wide)
    {
        m_overdueTasks.clear();
        const QList<Task> tasks = m_taskManager->overdueTasks(workspaceId_);

        for (const Task& task : tasks)
        {
            QVariantMap taskData;
            taskData["id"] = task.id.toString(QUuid::WithoutBraces);
            taskData["title"] = task.title.isEmpty() ? tr("Untitled Task") : task.title;
            taskData["description"] = task.description;

            QString prioStr = "Medium";
            if (task.priority == TaskPriority::High || task.priority == TaskPriority::Critical) prioStr = "High";
            else if (task.priority == TaskPriority::Low) prioStr = "Low";
            taskData["priority"] = prioStr;

            taskData["isCompleted"] = (task.status == TaskStatus::Completed);
            taskData["dueTimeFormatted"] = task.dueDate.isValid() ? ("Due " + task.dueDate.toString("MMM d")) : tr("Overdue");

            if (!task.projectId.isNull() && m_repo) {
                const Project p = m_repo->getProjectById(task.projectId);
                taskData["projectName"] = p.name.isEmpty() ? tr("General") : p.name;
                taskData["projectColor"] = colorForProject(task.projectId);
            } else {
                taskData["projectName"] = tr("General");
                taskData["projectColor"] = "#FF6B57";
            }

            m_overdueTasks.append(taskData);
        }

        emit overdueTasksChanged();
    }

    // 4. WORKSPACE STATS & PROGRESS
    {
        const QList<Project> projects = m_repo->getProjectsByWorkspace(workspaceId_);
        m_activeProjectsCount = projects.size();

        const QList<Task> allTasks = m_taskManager->allTasks(workspaceId_);
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

        m_completionPercentage = allTasks.isEmpty() ? 100 : ((completed * 100) / allTasks.size());

        m_projectProgress.clear();
        QVariantMap progressData;
        progressData["completed"] = completed;
        progressData["inProgress"] = inProgress;
        progressData["pending"] = pending;
        progressData["total"] = allTasks.size();
        progressData["completedPercent"] = m_completionPercentage;
        m_projectProgress.append(progressData);

        emit projectProgressChanged();
        emit statsChanged();
    }
}

void wsHomePage::setupUi()
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_homeQuickView = new QQuickWidget(this);
    m_homeQuickView->setResizeMode(QQuickWidget::SizeRootObjectToView);


    m_homeQuickView->setClearColor(Qt::transparent);
    m_homeQuickView->setAttribute(Qt::WA_TranslucentBackground);
    m_homeQuickView->setAttribute(Qt::WA_AlwaysStackOnTop);
    m_homeQuickView->setStyleSheet("QQuickWidget { border: none; outline: none; background: transparent; }");
    m_homeQuickView->setMinimumHeight(300);

    m_homeQuickView->rootContext()->setContextProperty("wsHomePage", this);
    connect(m_homeQuickView, &QQuickWidget::statusChanged, this, [this](QQuickWidget::Status status) {
        if (status == QQuickWidget::Error) {
            for (const auto& err : m_homeQuickView->errors()) {
                qWarning() << "[wsHomePage QML Error]" << err.toString();
            }
        }
    });
    m_homeQuickView->setSource(QUrl("qrc:/qml/wsHomeLinker.qml"));

    if (m_homeQuickView->status() == QQuickWidget::Error) {
        const auto errs = m_homeQuickView->errors();
        for (const auto& err : errs) {
            qWarning() << "[wsHomePage QML Error]" << err.toString();
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

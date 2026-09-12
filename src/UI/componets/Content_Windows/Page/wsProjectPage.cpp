#include "UI/components/Content_Windows/page/wsProjectPage.h"
#include <QVBoxLayout>
#include <QQmlContext>
#include <QDebug>
#include <algorithm>
#include <QRegularExpression>
#include <QDesktopServices>
#include <QUrl>
#include <QFileInfo>

static QString colorForProject(const QUuid& projectId)
{
    if (projectId.isNull()) return "#81C784";
    static const QStringList palette = {
        "#81C784", "#FFD700", "#9ACD32", "#20B2AA", "#FF69B4", "#64B5F6", "#BA68C8", "#FF8A65"
    };
    return palette.at(qAbs(qHash(projectId.toString())) % palette.size());
}

static QString initialsForProject(const QString& name)
{
    if (name.trimmed().isEmpty()) return "PR";
    const QStringList words = name.trimmed().split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
    if (words.size() >= 2) {
        return (words[0].left(1) + words[1].left(1)).toUpper();
    }
    return name.trimmed().left(2).toUpper();
}

static QString formatBytes(qint64 bytes)
{
    if (bytes < 1024) return QString::number(bytes) + " B";
    if (bytes < 1024 * 1024) return QString::number(bytes / 1024.0, 'f', 1) + " KB";
    return QString::number(bytes / (1024.0 * 1024.0), 'f', 1) + " MB";
}

wsProjectPage::wsProjectPage(const Workspace& ws, const Project& project, WorkspaceRepository* repo, QWidget* parent)
    : IWorkspaceView(ws, parent), m_workspace(ws), m_project(project), m_repo(repo)
{
    if (!m_repo) return;

    m_taskManager = new TaskManager(m_repo, this);
    m_noteManager = new NoteManager(m_repo, this);

    connect(m_taskManager, &TaskManager::tasksChanged, this, [this](const QUuid& wsId) {
        if (wsId == m_workspace.id) {
            populateData();
        }
    });

    connect(m_noteManager, &NoteManager::notesChanged, this, [this](const QUuid& wsId) {
        if (wsId == m_workspace.id) {
            populateData();
        }
    });

    connect(m_repo, &WorkspaceRepository::attachmentAdded, this, [this](const FileAttachment& att) {
        if (att.projectId == m_project.id) {
            populateData();
        }
    });

    connect(m_repo, &WorkspaceRepository::attachmentDeleted, this, [this]() {
        populateData();
    });

    connect(m_repo, &WorkspaceRepository::projectUpdated, this, [this](const Project& p) {
        if (p.id == m_project.id) {
            m_project = p;
            emit projectChanged();
            populateData();
        }
    });

    setupUi();
    populateData();
}

wsProjectPage::wsProjectPage(const Workspace& ws, WorkspaceRepository* repo, QWidget* parent)
    : wsProjectPage(ws, Project{}, repo, parent)
{
}

void wsProjectPage::refresh()
{
    populateData();
}

void wsProjectPage::updateWorkspace(const Workspace& ws)
{
    m_workspace = ws;
    emit projectChanged();
    populateData();
}

void wsProjectPage::setActiveProject(const Project& project)
{
    m_project = project;
    emit projectChanged();
    populateData();
}

void wsProjectPage::createNewTask(const QString& title, const QString& priority)
{
    if (!m_taskManager || title.trimmed().isEmpty()) return;

    TaskCreateRequest req;
    req.workspaceId = m_workspace.id;
    req.projectId = m_project.id;
    req.title = title.trimmed();

    if (priority.compare("High", Qt::CaseInsensitive) == 0) {
        req.priority = TaskPriority::High;
    } else if (priority.compare("Low", Qt::CaseInsensitive) == 0) {
        req.priority = TaskPriority::Low;
    } else {
        req.priority = TaskPriority::Medium;
    }

    m_taskManager->createTask(req);
}

void wsProjectPage::createNewNote(const QString& title)
{
    if (!m_noteManager || title.trimmed().isEmpty()) return;

    NoteCreateRequest req;
    req.title = title.trimmed();
    req.content = "";
    req.workspaceId = m_workspace.id;
    req.projectId = m_project.id;
    req.isPinned = false;

    const QUuid newId = m_noteManager->createNote(req);
    if (!newId.isNull()) {
        emit noteOpenRequested(newId.toString(QUuid::WithoutBraces));
    }
}

void wsProjectPage::toggleTaskCompletion(const QString& taskId, bool completed)
{
    if (!m_taskManager || taskId.isEmpty()) return;
    m_taskManager->setCompleted(QUuid::fromString(taskId), completed);
}

void wsProjectPage::deleteTask(const QString& taskId)
{
    if (!m_taskManager || taskId.isEmpty()) return;
    m_taskManager->deleteTask(QUuid::fromString(taskId));
}

void wsProjectPage::openNote(const QString& noteId)
{
    if (noteId.isEmpty()) return;
    emit noteOpenRequested(noteId);
}

void wsProjectPage::deleteNote(const QString& noteId)
{
    if (!m_noteManager || noteId.isEmpty()) return;
    m_noteManager->deleteNote(QUuid::fromString(noteId));
}

void wsProjectPage::togglePinNote(const QString& noteId)
{
    if (!m_noteManager || noteId.isEmpty()) return;
    const QUuid id = QUuid::fromString(noteId);
    if (id.isNull()) return;

    const Note note = m_noteManager->getNoteById(id);
    if (!note.id.isNull()) {
        m_noteManager->setPinned(id, !note.isPinned);
    }
}

void wsProjectPage::openUploadDialog()
{
    if (!m_uploadDialog) {
        m_uploadDialog = new UploadDialog(this);
        m_uploadDialog->setFileFilter(
            tr("Allowed Files (*.pdf *.png *.jpg *.jpeg *.txt *.md *.docx *.xlsx);;All Files (*.*)")
        );
        connect(m_uploadDialog, &UploadDialog::uploadRequested,
                this, &wsProjectPage::onUploadRequested);
    }

    m_uploadDialog->setWorkspaceId(m_workspace.id);
    m_uploadDialog->setProjectId(m_project.id);
    m_uploadDialog->show();
    m_uploadDialog->raise();
    m_uploadDialog->activateWindow();
}

void wsProjectPage::onUploadRequested(const QStringList& sourcePaths, const QUuid& workspaceId, const QUuid& projectId)
{
    Q_UNUSED(workspaceId);
    if (!m_repo || sourcePaths.isEmpty()) return;

    for (const QString& path : sourcePaths) {
        QFileInfo fi(path);
        FileAttachment att;
        att.workspaceId = m_workspace.id;
        att.projectId = projectId.isNull() ? m_project.id : projectId;
        att.fileName = fi.fileName();
        att.relativePath = path;
        att.fileSize = fi.size();
        att.linkedEntityType = AttachmentEntityType::Workspace;
        m_repo->createAttachment(att);
    }
    emit uploadMessage(tr("Files uploaded successfully."), false);
    populateData();
}

void wsProjectPage::openAttachment(const QString& attachmentId)
{
    if (!m_repo || attachmentId.isEmpty()) return;
    const FileAttachment att = m_repo->getAttachmentById(QUuid::fromString(attachmentId));
    if (!att.relativePath.isEmpty()) {
        QDesktopServices::openUrl(QUrl::fromLocalFile(att.relativePath));
    }
}

void wsProjectPage::deleteAttachment(const QString& attachmentId)
{
    if (!m_repo || attachmentId.isEmpty()) return;
    m_repo->deleteAttachment(QUuid::fromString(attachmentId));
    populateData();
}

QString wsProjectPage::workspaceName() const
{
    return m_workspace.name.isEmpty() ? tr("Workspace") : m_workspace.name;
}

QString wsProjectPage::projectId() const
{
    return m_project.id.toString(QUuid::WithoutBraces);
}

QString wsProjectPage::projectName() const
{
    return m_project.name.isEmpty() ? tr("Project") : m_project.name;
}

QString wsProjectPage::projectColor() const
{
    return colorForProject(m_project.id);
}

QString wsProjectPage::projectInitials() const
{
    return initialsForProject(m_project.name);
}

QString wsProjectPage::projectDescription() const
{
    return m_project.description.isEmpty()
        ? tr("Track milestones, tasks, notes, and assets for this project.")
        : m_project.description;
}

QString wsProjectPage::createdAtFormatted() const
{
    if (m_project.createdAt.isValid()) {
        return m_project.createdAt.toString("MMM d, yyyy");
    }
    return tr("Recently");
}

void wsProjectPage::populateData()
{
    if (!m_repo) return;

    const QUuid projId = m_project.id;

    // 1. TASKS
    {
        m_tasks.clear();
        const QList<Task> projTasks = m_repo->getTasksByProject(projId);

        int completed = 0;
        int inProgress = 0;
        int pending = 0;

        for (const Task& task : projTasks) {
            QVariantMap map;
            map["id"] = task.id.toString(QUuid::WithoutBraces);
            map["title"] = task.title.isEmpty() ? tr("Untitled Task") : task.title;
            map["description"] = task.description;

            QString prioStr = "Medium";
            if (task.priority == TaskPriority::High || task.priority == TaskPriority::Critical) prioStr = "High";
            else if (task.priority == TaskPriority::Low) prioStr = "Low";
            map["priority"] = prioStr;

            const bool isComp = (task.status == TaskStatus::Completed);
            map["isCompleted"] = isComp;

            if (isComp) {
                completed++;
            } else if (task.status == TaskStatus::InProgress) {
                inProgress++;
            } else {
                pending++;
            }

            if (task.dueDate.isValid()) {
                map["dueDateFormatted"] = task.dueDate.toString("MMM d");
            } else {
                map["dueDateFormatted"] = "";
            }

            m_tasks.append(map);
        }

        m_totalTasksCount = projTasks.size();
        m_completedTasksCount = completed;
        m_inProgressTasksCount = inProgress;
        m_pendingTasksCount = pending;
        m_completionPercentage = m_totalTasksCount > 0 ? (completed * 100) / m_totalTasksCount : 0;

        emit tasksChanged();
    }

    // 2. NOTES
    {
        m_notes.clear();
        QList<Note> projNotes = m_repo->getNotesByProject(projId);
        projNotes.erase(std::remove_if(projNotes.begin(), projNotes.end(), [](const Note& n) {
            return n.isArchived;
        }), projNotes.end());

        std::sort(projNotes.begin(), projNotes.end(), [](const Note& a, const Note& b) {
            const QDateTime aKey = a.updatedAt.isValid() ? a.updatedAt : a.createdAt;
            const QDateTime bKey = b.updatedAt.isValid() ? b.updatedAt : b.createdAt;
            return aKey > bKey;
        });

        m_notesCount = projNotes.size();
        const QDateTime now = QDateTime::currentDateTime();

        for (const Note& note : projNotes) {
            QVariantMap map;
            map["id"] = note.id.toString(QUuid::WithoutBraces);
            map["title"] = note.title.isEmpty() ? tr("Untitled Note") : note.title;

            QString previewText = note.preview;
            if (previewText.isEmpty() && !note.content.isEmpty()) {
                previewText = note.content.left(140).trimmed();
                previewText.replace('\n', ' ');
            }
            map["preview"] = previewText;

            int words = 0;
            if (!note.content.trimmed().isEmpty()) {
                words = note.content.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts).size();
            }
            map["wordCountFormatted"] = words >= 1000
                ? QString("%1k words").arg(QString::number(words / 1000.0, 'f', 1))
                : QString("%1 words").arg(words);

            const qint64 secs = note.updatedAt.secsTo(now);
            QString timeStr;
            if (secs < 60) timeStr = tr("Just now");
            else if (secs < 3600) timeStr = tr("%1m ago").arg(secs / 60);
            else if (secs < 86400) timeStr = tr("%1h ago").arg(secs / 3600);
            else if (secs < 86400 * 2) timeStr = tr("Yesterday");
            else if (secs < 86400 * 7) timeStr = tr("%1d ago").arg(secs / 86400);
            else timeStr = note.updatedAt.toString("MMM d");

            map["updatedAtFormatted"] = timeStr;
            map["isPinned"] = note.isPinned;
            map["projectName"] = m_project.name;
            map["projectColor"] = colorForProject(m_project.id);

            m_notes.append(map);
        }

        emit notesChanged();
    }

    // 3. ATTACHMENTS
    {
        m_attachments.clear();
        const QList<FileAttachment> atts = m_repo->getAttachmentsByProject(projId);
        m_filesCount = atts.size();

        for (const FileAttachment& att : atts) {
            QVariantMap map;
            map["id"] = att.id.toString(QUuid::WithoutBraces);
            map["fileName"] = att.fileName;
            map["fileSizeFormatted"] = formatBytes(att.fileSize);

            const QString ext = QFileInfo(att.fileName).suffix().toLower();
            map["fileExtension"] = ext.toUpper();

            QString icon = "📄";
            if (ext == "pdf") icon = "📕";
            else if (ext == "png" || ext == "jpg" || ext == "jpeg" || ext == "svg") icon = "🖼️";
            else if (ext == "xlsx" || ext == "csv") icon = "📊";
            else if (ext == "docx" || ext == "doc" || ext == "txt" || ext == "md") icon = "📝";
            map["iconEmoji"] = icon;

            m_attachments.append(map);
        }

        emit attachmentsChanged();
    }

    emit statsChanged();
}

void wsProjectPage::setupUi()
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_quickView = new QQuickWidget(this);
    m_quickView->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_quickView->setClearColor(Qt::transparent);
    m_quickView->setAttribute(Qt::WA_TranslucentBackground);
    m_quickView->setAttribute(Qt::WA_AlwaysStackOnTop);
    m_quickView->setStyleSheet("QQuickWidget { border: none; outline: none; background: transparent; }");
    m_quickView->setMinimumHeight(300);

    m_quickView->rootContext()->setContextProperty("wsProjectPage", this);
    m_quickView->setSource(QUrl("qrc:/qml/wsProjectLinker.qml"));

    if (m_quickView->status() == QQuickWidget::Error) {
        for (const auto& err : m_quickView->errors()) {
            qWarning() << "wsProjectPage QML Error:" << err.toString();
        }
    }

    layout->addWidget(m_quickView, 1);
}

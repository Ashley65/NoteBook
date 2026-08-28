//
// Created by DevAccount on 27/08/2026.
//

#include "UI/components/Content_Windows/page/wsTaskBoardPage.h"
#include <QVBoxLayout>
#include <QQmlContext>
#include <algorithm>

wsTaskBoardPage::wsTaskBoardPage(const Workspace& ws, WorkspaceRepository* repo, QWidget* parent)
    : IWorkspaceView(ws, parent), m_workspace(ws), workspaceId_(ws.id), m_repo(repo)
{
    if (!m_repo) {
        return;
    }

    m_taskManager = new TaskManager(m_repo, this);

    connect(m_taskManager, &TaskManager::tasksChanged, this, [this](const QUuid& wsId) {
        if (wsId == workspaceId_ || wsId.isNull()) {
            populateData();
        }
    });

    setupUi();
    populateData();
}

void wsTaskBoardPage::setupUi()
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_TaskQuickView = new QQuickWidget(this);
    m_TaskQuickView->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_TaskQuickView->setAttribute(Qt::WA_TranslucentBackground);
    m_TaskQuickView->setClearColor(Qt::transparent);

    m_TaskQuickView->rootContext()->setContextProperty("taskBoard", this);
    m_TaskQuickView->setSource(QUrl("qrc:/qml/wsTaskBoardLinker.qml"));

    if (m_TaskQuickView->status() == QQuickWidget::Error) {
        const auto errs = m_TaskQuickView->errors();
        for (const auto& err : errs) {
            qWarning() << "wsTaskBoardPage QML Error:" << err.toString();
        }
    }

    layout->addWidget(m_TaskQuickView, 1);
}

void wsTaskBoardPage::refresh()
{
    populateData();
}

void wsTaskBoardPage::updateWorkspace(const Workspace& ws)
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

void wsTaskBoardPage::setActiveProject(const Project& project)
{
    if (project.workspaceId != workspaceId_) {
        return;
    }

    if (m_activeProject.id == project.id) {
        return;
    }

    m_activeProject = project;
    emit activeProjectChanged();
    populateData();
    if (m_TaskQuickView) {
        m_TaskQuickView->update();
    }
}

QString wsTaskBoardPage::workspaceName() const
{
    return m_workspace.name;
}

QString wsTaskBoardPage::activeProjectId() const
{
    if (m_activeProject.id.isNull()) return QString();
    return m_activeProject.id.toString(QUuid::WithoutBraces);
}

QString wsTaskBoardPage::activeProjectName() const
{
    if (m_activeProject.id.isNull()) return QString();
    return m_activeProject.name;
}

QVariantList wsTaskBoardPage::todoTasks() const
{
    return m_todoTasks;
}

QVariantList wsTaskBoardPage::inProgressTasks() const
{
    return m_inProgressTasks;
}

QVariantList wsTaskBoardPage::completedTasks() const
{
    return m_completedTasks;
}

int wsTaskBoardPage::todoCount() const
{
    return m_todoTasks.size();
}

int wsTaskBoardPage::inProgressCount() const
{
    return m_inProgressTasks.size();
}

int wsTaskBoardPage::completedCount() const
{
    return m_completedTasks.size();
}

QString wsTaskBoardPage::searchQuery() const
{
    return m_searchQuery;
}

int wsTaskBoardPage::priorityFilter() const
{
    return m_priorityFilter;
}

int wsTaskBoardPage::sortMode() const
{
    return m_sortMode;
}

void wsTaskBoardPage::setActiveProjectId(const QString& id)
{
    if (id.isEmpty()) {
        m_activeProject = Project{};
        emit activeProjectChanged();
        populateData();
        return;
    }

    const QUuid projId = QUuid::fromString(id);
    if (m_repo) {
        Project proj = m_repo->getProjectById(projId);
        setActiveProject(proj);
    }
}

void wsTaskBoardPage::setSearchQuery(const QString& query)
{
    if (m_searchQuery == query) return;
    m_searchQuery = query;
    emit filterChanged();
    populateData();
}

void wsTaskBoardPage::setPriorityFilter(int priority)
{
    if (m_priorityFilter == priority) return;
    m_priorityFilter = priority;
    emit filterChanged();
    populateData();
}

void wsTaskBoardPage::setSortMode(int mode)
{
    if (m_sortMode == mode) return;
    m_sortMode = mode;
    emit sortModeChanged();
    populateData();
}

void wsTaskBoardPage::populateData()
{
    if (!m_taskManager || !m_repo) return;

    m_todoTasks.clear();
    m_inProgressTasks.clear();
    m_completedTasks.clear();

    QList<Task> allTasks;
    if (!m_activeProject.id.isNull()) {
        allTasks = m_repo->getTasksByProject(m_activeProject.id);
    } else {
        allTasks = m_taskManager->allTasks(workspaceId_);
    }

    const QDateTime now = QDateTime::currentDateTime();
    const QString cleanQuery = m_searchQuery.trimmed().toLower();

    // 1. Filter by Search Query & Priority
    QList<Task> filteredTasks;
    for (const Task& task : allTasks) {
        if (!cleanQuery.isEmpty()) {
            const bool matchTitle = task.title.toLower().contains(cleanQuery);
            const bool matchDesc = task.description.toLower().contains(cleanQuery);
            if (!matchTitle && !matchDesc) {
                continue;
            }
        }

        if (m_priorityFilter >= 0) {
            if (static_cast<int>(task.priority) != m_priorityFilter) {
                continue;
            }
        }

        filteredTasks.append(task);
    }

    // 2. Sort Tasks
    std::sort(filteredTasks.begin(), filteredTasks.end(), [this](const Task& a, const Task& b) {
        switch (m_sortMode) {
            case 0: { // Due Date Earliest
                const bool aHasDue = a.dueDate.isValid();
                const bool bHasDue = b.dueDate.isValid();
                if (aHasDue && bHasDue) return a.dueDate < b.dueDate;
                if (aHasDue != bHasDue) return aHasDue;
                return a.createdAt < b.createdAt;
            }
            case 1: { // Priority (Critical -> High -> Medium -> Low)
                if (a.priority != b.priority) {
                    return static_cast<int>(a.priority) > static_cast<int>(b.priority);
                }
                return a.createdAt < b.createdAt;
            }
            case 2: { // Title Alphabetical
                return a.title.localeAwareCompare(b.title) < 0;
            }
            case 3: { // Newest Created
                return a.createdAt > b.createdAt;
            }
            default:
                return a.createdAt < b.createdAt;
        }
    });

    // 3. Convert to QVariantMap and partition by status
    for (const Task& task : filteredTasks) {
        QVariantMap map;
        map["id"] = task.id.toString(QUuid::WithoutBraces);
        map["workspaceId"] = task.workspaceId.toString(QUuid::WithoutBraces);
        map["projectId"] = task.projectId.toString(QUuid::WithoutBraces);
        map["title"] = task.title;
        map["description"] = task.description;
        map["status"] = static_cast<int>(task.status);
        map["priority"] = static_cast<int>(task.priority);

        // Priority text
        QString prioText = "Medium";
        switch (task.priority) {
            case TaskPriority::Low: prioText = "Low"; break;
            case TaskPriority::Medium: prioText = "Medium"; break;
            case TaskPriority::High: prioText = "High"; break;
            case TaskPriority::Critical: prioText = "Critical"; break;
        }
        map["priorityText"] = prioText;

        // Due date & overdue calculation
        map["dueDate"] = task.dueDate;
        if (task.dueDate.isValid()) {
            map["dueDateFormatted"] = task.dueDate.toString("MMM d");
            const bool isOverdue = (task.dueDate < now) && (task.status != TaskStatus::Completed && task.status != TaskStatus::Archived);
            map["isOverdue"] = isOverdue;
            if (isOverdue) {
                qint64 secs = task.dueDate.secsTo(now);
                int days = static_cast<int>(secs / 86400);
                if (days < 1) days = 1;
                map["dueStatusText"] = QString("Due: %1 (Overdue: -%2d)").arg(task.dueDate.toString("MMM d")).arg(days);
            } else {
                map["dueStatusText"] = QString("Due: %1").arg(task.dueDate.toString("MMM d"));
            }
        } else {
            map["dueDateFormatted"] = "";
            map["isOverdue"] = false;
            map["dueStatusText"] = "";
        }

        // Finished date for completed tasks
        if (task.status == TaskStatus::Completed && task.completedAt.isValid()) {
            map["finishedText"] = QString("Finished: %1").arg(task.completedAt.toString("MMM d"));
        } else {
            map["finishedText"] = "";
        }

        // Subtasks progress
        const int totalSub = task.totalSubTasks();
        const int compSub = task.completedSubTasks();
        map["subtasksTotal"] = totalSub;
        map["subtasksCompleted"] = compSub;
        map["hasSubtasks"] = (totalSub > 0);
        map["allSubtasksDone"] = (totalSub > 0 && compSub == totalSub);

        QVariantList subtaskList;
        for (const auto& st : task.subtasks) {
            QVariantMap stMap;
            stMap["id"] = st.id.toString(QUuid::WithoutBraces);
            stMap["taskId"] = st.taskId.toString(QUuid::WithoutBraces);
            stMap["title"] = st.title;
            stMap["isCompleted"] = st.isCompleted;
            subtaskList.append(stMap);
        }
        map["subtasks"] = subtaskList;

        // Linked notes / attachments count
        map["linkedNotesCount"] = 0;

        // Partition into columns
        switch (task.status) {
            case TaskStatus::Pending:
                m_todoTasks.append(map);
                break;
            case TaskStatus::InProgress:
                m_inProgressTasks.append(map);
                break;
            case TaskStatus::Completed:
            case TaskStatus::Archived:
                m_completedTasks.append(map);
                break;
        }
    }

    emit tasksChanged();
}

void wsTaskBoardPage::createNewTask(const QString& title, const QString& description, int priority, const QDateTime& dueDate)
{
    if (!m_taskManager) return;
    const QString cleanTitle = title.trimmed();
    if (cleanTitle.isEmpty()) return;

    TaskCreateRequest request;
    request.workspaceId = m_workspace.id;
    request.projectId   = m_activeProject.id;
    request.title       = cleanTitle;
    request.description = description.trimmed();
    request.priority    = static_cast<TaskPriority>(priority);
    request.dueDate     = dueDate;
    request.status      = TaskStatus::Pending;

    const QUuid newId = m_taskManager->createTask(request);
    if (!newId.isNull()) {
        populateData();
    }
}

void wsTaskBoardPage::updateTaskStatus(const QString& taskId, int newStatus)
{
    if (!m_taskManager || taskId.isEmpty()) return;
    const QUuid id = QUuid::fromString(taskId);
    if (id.isNull()) return;

    m_taskManager->setTaskStatus(id, static_cast<TaskStatus>(newStatus));
    populateData();
}

void wsTaskBoardPage::toggleTaskCompletion(const QString& taskId, bool isCompleted)
{
    if (!m_taskManager || taskId.isEmpty()) return;
    const QUuid id = QUuid::fromString(taskId);
    if (id.isNull()) return;

    m_taskManager->setCompleted(id, isCompleted);
    populateData();
}

void wsTaskBoardPage::deleteTask(const QString& taskId)
{
    if (!m_taskManager || taskId.isEmpty()) return;
    const QUuid id = QUuid::fromString(taskId);
    if (id.isNull()) return;

    m_taskManager->deleteTask(id);
    populateData();
}

void wsTaskBoardPage::openTaskDetails(const QString& taskId)
{
    Q_UNUSED(taskId);
}

void wsTaskBoardPage::openLinkedNote(const QString& noteId)
{
    if (noteId.isEmpty()) return;
    emit noteOpenRequested(noteId);
}

void wsTaskBoardPage::addSubtask(const QString& taskId, const QString& title)
{
    if (!m_taskManager || taskId.isEmpty() || title.trimmed().isEmpty()) return;
    const QUuid id = QUuid::fromString(taskId);
    if (id.isNull()) return;

    m_taskManager->addSubtask(id, title);
    populateData();
}

void wsTaskBoardPage::toggleSubtask(const QString& taskId, const QString& subtaskId, bool completed)
{
    if (!m_taskManager || taskId.isEmpty() || subtaskId.isEmpty()) return;
    const QUuid tId = QUuid::fromString(taskId);
    const QUuid sId = QUuid::fromString(subtaskId);
    if (tId.isNull() || sId.isNull()) return;

    m_taskManager->toggleSubtask(tId, sId, completed);
    populateData();
}

void wsTaskBoardPage::deleteSubtask(const QString& taskId, const QString& subtaskId)
{
    if (!m_taskManager || taskId.isEmpty() || subtaskId.isEmpty()) return;
    const QUuid tId = QUuid::fromString(taskId);
    const QUuid sId = QUuid::fromString(subtaskId);
    if (tId.isNull() || sId.isNull()) return;

    m_taskManager->deleteSubtask(tId, sId);
    populateData();
}

void wsTaskBoardPage::updateSubtask(const QString& taskId, const QString& subtaskId, const QString& title, bool completed)
{
    if (!m_taskManager || taskId.isEmpty() || subtaskId.isEmpty()) return;
    const QUuid tId = QUuid::fromString(taskId);
    const QUuid sId = QUuid::fromString(subtaskId);
    if (tId.isNull() || sId.isNull()) return;

    m_taskManager->updateSubtask(tId, sId, title, completed);
    populateData();
}

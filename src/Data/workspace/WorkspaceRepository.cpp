#include "Data/workspace/WorkspaceRepository.h"
#include <QUuid>
#include <QSettings>

WorkspaceRepository::WorkspaceRepository(QObject* parent) : QObject(parent)
{
    // LINK: Load existing workspaces and tasks from persistent storage
    loadWorkspaces();
    loadTasks();
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

QString WorkspaceRepository::createWorkspace(const QString& name) {
    Workspace ws;
    ws.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    ws.name = name;
    workspaces_.append(ws);

    // LINK: Persist changes immediately
    saveWorkspaces();

    emit workspaceAdded(ws);
    return ws.id;
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
            break;
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
    QSettings s;
    s.beginWriteArray("workspaces");

    for (int i = 0; i < workspaces_.size(); ++i) {
        s.setArrayIndex(i);
        s.setValue("id", workspaces_[i].id);
        s.setValue("name", workspaces_[i].name);
    }

    s.endArray();
}

void WorkspaceRepository::loadWorkspaces()
{
    QSettings s;
    int count = s.beginReadArray("workspaces");

    // clear existing list to prevent duplicates
    workspaces_.clear();

    for (int i = 0; i < count; ++i) {
        s.setArrayIndex(i);
        Workspace ws;
        ws.id = s.value("id").toString();
        ws.name = s.value("name").toString();
        workspaces_.append(ws);
    }

    s.endArray();
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





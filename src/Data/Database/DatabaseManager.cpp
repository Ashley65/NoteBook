//
// Created by DevAccount on 09/09/2026.
//

#include "Data/Database/DatabaseManager.h"
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QVariant>
#include <QDateTime>
#include <QDebug>

DatabaseManager::DatabaseManager(const QString& connectionName)
    : m_connectionName(connectionName)
{
}

DatabaseManager::~DatabaseManager()
{
    close();
}

QSqlDatabase DatabaseManager::db() const
{
    if (QSqlDatabase::contains(m_connectionName)) {
        return QSqlDatabase::database(m_connectionName);
    }
    return QSqlDatabase::addDatabase("QSQLITE", m_connectionName);
}

bool DatabaseManager::open(const QString& dbPath)
{
    QFileInfo fi(dbPath);
    QDir().mkpath(fi.absolutePath());

    QSqlDatabase d = db();
    d.setDatabaseName(dbPath);

    if (!d.open()) {
        m_lastError = d.lastError().text();
        return false;
    }

    // Enable Pragmas
    QSqlQuery pragmaQuery(d);
    pragmaQuery.exec("PRAGMA foreign_keys = ON;");
    pragmaQuery.exec("PRAGMA journal_mode = WAL;");
    pragmaQuery.exec("PRAGMA synchronous = NORMAL;");

    return initializeSchema();
}

void DatabaseManager::close()
{
    if (QSqlDatabase::contains(m_connectionName)) {
        {
            QSqlDatabase d = QSqlDatabase::database(m_connectionName);
            if (d.isOpen()) {
                d.close();
            }
        }
        QSqlDatabase::removeDatabase(m_connectionName);
    }
}

bool DatabaseManager::isOpen() const
{
    if (!QSqlDatabase::contains(m_connectionName)) return false;
    return QSqlDatabase::database(m_connectionName).isOpen();
}

QString DatabaseManager::lastError() const
{
    return m_lastError;
}

bool DatabaseManager::beginTransaction()
{
    return db().transaction();
}

bool DatabaseManager::commitTransaction()
{
    return db().commit();
}

bool DatabaseManager::rollbackTransaction()
{
    return db().rollback();
}

bool DatabaseManager::initializeSchema()
{
    QFile file(":/sql/schema.sql");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        m_lastError = QString("Failed to load schema resource: %1").arg(file.errorString());
        return false;
    }

    const QString schemaSql = QString::fromUtf8(file.readAll());
    file.close();

    if (!beginTransaction()) {
        m_lastError = db().lastError().text();
        return false;
    }

    const QStringList lines = schemaSql.split('\n');
    QString currentStatement;

    for (const QString& line : lines) {
        const QString trimmed = line.trimmed();
        if (trimmed.startsWith("--") || trimmed.isEmpty()) {
            continue;
        }

        currentStatement += line + "\n";

        if (trimmed.endsWith(';')) {
            QSqlQuery q(db());
            if (!q.exec(currentStatement)) {
                m_lastError = QString("Schema execution error: %1\nQuery: %2")
                                  .arg(q.lastError().text(), currentStatement);
                rollbackTransaction();
                return false;
            }
            currentStatement.clear();
        }
    }

    if (currentSchemaVersion() == 0) {
        setSchemaVersion(1);
    }

    if (!commitTransaction()) {
        m_lastError = db().lastError().text();
        return false;
    }

    return runMigrations();
}

int DatabaseManager::currentSchemaVersion() const
{
    QSqlQuery q(db());
    if (q.exec("SELECT MAX(version) FROM schema_migrations;")) {
        if (q.next() && !q.value(0).isNull()) {
            return q.value(0).toInt();
        }
    }
    return 0;
}

bool DatabaseManager::setSchemaVersion(int version)
{
    QSqlQuery q(db());
    q.prepare("INSERT INTO schema_migrations (version, applied_at) VALUES (:version, :applied_at);");
    q.bindValue(":version", version);
    q.bindValue(":applied_at", QDateTime::currentDateTime().toString(Qt::ISODate));
    if (!q.exec()) {
        m_lastError = q.lastError().text();
        return false;
    }
    return true;
}

bool DatabaseManager::runMigrations()
{
    return true;
}

// Workspaces


QUuid DatabaseManager::addWorkspace(const Workspace& ws)
{
    const QUuid id = ws.id.isNull() ? QUuid::createUuid() : ws.id;

    QSqlQuery q(db());
    q.prepare(R"(
        INSERT INTO workspaces (
            id, name, type, description, icon, color,
            protected_mode, is_archived, is_pinned, sort_order,
            created_at, updated_at, last_opened_at, task_count, note_count
        ) VALUES (
            :id, :name, :type, :description, :icon, :color,
            :protected_mode, :is_archived, :is_pinned, :sort_order,
            :created_at, :updated_at, :last_opened_at, :task_count, :note_count
        );
    )");

    q.bindValue(":id", uuidToDb(id));
    q.bindValue(":name", ws.name);
    q.bindValue(":type", ws.type.isEmpty() ? "custom" : ws.type);
    q.bindValue(":description", ws.description);
    q.bindValue(":icon", ws.icon);
    q.bindValue(":color", ws.color.isValid() ? ws.color.name() : "");
    q.bindValue(":protected_mode", ws.protectedMode ? 1 : 0);
    q.bindValue(":is_archived", ws.isArchived ? 1 : 0);
    q.bindValue(":is_pinned", ws.isPinned ? 1 : 0);
    q.bindValue(":sort_order", ws.order);
    q.bindValue(":created_at", ws.createdAt.isValid() ? ws.createdAt.toString(Qt::ISODate) : QDateTime::currentDateTime().toString(Qt::ISODate));
    q.bindValue(":updated_at", ws.updatedAt.isValid() ? ws.updatedAt.toString(Qt::ISODate) : QDateTime::currentDateTime().toString(Qt::ISODate));
    q.bindValue(":last_opened_at", ws.lastOpenedAt.isValid() ? ws.lastOpenedAt.toString(Qt::ISODate) : QVariant());
    q.bindValue(":task_count", ws.taskCount);
    q.bindValue(":note_count", ws.noteCount);

    if (!q.exec()) {
        m_lastError = q.lastError().text();
        return {};
    }

    return id;
}

bool DatabaseManager::updateWorkspace(const Workspace& ws)
{
    QSqlQuery q(db());
    q.prepare(R"(
        UPDATE workspaces SET
            name = :name,
            type = :type,
            description = :description,
            icon = :icon,
            color = :color,
            protected_mode = :protected_mode,
            is_archived = :is_archived,
            is_pinned = :is_pinned,
            sort_order = :sort_order,
            updated_at = :updated_at,
            last_opened_at = :last_opened_at,
            task_count = :task_count,
            note_count = :note_count
        WHERE id = :id;
    )");

    q.bindValue(":id", uuidToDb(ws.id));
    q.bindValue(":name", ws.name);
    q.bindValue(":type", ws.type);
    q.bindValue(":description", ws.description);
    q.bindValue(":icon", ws.icon);
    q.bindValue(":color", ws.color.isValid() ? ws.color.name() : "");
    q.bindValue(":protected_mode", ws.protectedMode ? 1 : 0);
    q.bindValue(":is_archived", ws.isArchived ? 1 : 0);
    q.bindValue(":is_pinned", ws.isPinned ? 1 : 0);
    q.bindValue(":sort_order", ws.order);
    q.bindValue(":updated_at", ws.updatedAt.isValid() ? ws.updatedAt.toString(Qt::ISODate) : QDateTime::currentDateTime().toString(Qt::ISODate));
    q.bindValue(":last_opened_at", ws.lastOpenedAt.isValid() ? ws.lastOpenedAt.toString(Qt::ISODate) : QVariant());
    q.bindValue(":task_count", ws.taskCount);
    q.bindValue(":note_count", ws.noteCount);

    if (!q.exec()) {
        m_lastError = q.lastError().text();
        return false;
    }
    return true;
}

std::optional<Workspace> DatabaseManager::getWorkspace(const QUuid& id) const
{
    QSqlQuery q(db());
    q.prepare("SELECT id, name, type, description, icon, color, protected_mode, is_archived, is_pinned, sort_order, created_at, updated_at, last_opened_at, task_count, note_count FROM workspaces WHERE id = :id;");
    q.bindValue(":id", uuidToDb(id));

    if (!q.exec() || !q.next()) {
        return std::nullopt;
    }

    Workspace ws;
    ws.id = dbToUuid(q.value("id").toString());
    ws.name = q.value("name").toString();
    ws.type = q.value("type").toString();
    ws.description = q.value("description").toString();
    ws.icon = q.value("icon").toString();
    ws.color = QColor(q.value("color").toString());
    ws.protectedMode = q.value("protected_mode").toInt() != 0;
    ws.isArchived = q.value("is_archived").toInt() != 0;
    ws.isPinned = q.value("is_pinned").toInt() != 0;
    ws.order = q.value("sort_order").toInt();
    ws.createdAt = QDateTime::fromString(q.value("created_at").toString(), Qt::ISODate);
    ws.updatedAt = QDateTime::fromString(q.value("updated_at").toString(), Qt::ISODate);
    if (!q.value("last_opened_at").isNull() && !q.value("last_opened_at").toString().isEmpty()) {
        ws.lastOpenedAt = QDateTime::fromString(q.value("last_opened_at").toString(), Qt::ISODate);
    }
    ws.taskCount = q.value("task_count").toInt();
    ws.noteCount = q.value("note_count").toInt();

    return ws;
}

QList<Workspace> DatabaseManager::getAllWorkspaces(bool includeArchived) const
{
    QList<Workspace> result;
    QSqlQuery q(db());
    QString sql = "SELECT id, name, type, description, icon, color, protected_mode, is_archived, is_pinned, sort_order, created_at, updated_at, last_opened_at, task_count, note_count FROM workspaces";
    if (!includeArchived) {
        sql += " WHERE is_archived = 0";
    }
    sql += " ORDER BY sort_order ASC, created_at ASC;";

    if (!q.exec(sql)) {
        m_lastError = q.lastError().text();
        return result;
    }

    while (q.next()) {
        Workspace ws;
        ws.id = dbToUuid(q.value("id").toString());
        ws.name = q.value("name").toString();
        ws.type = q.value("type").toString();
        ws.description = q.value("description").toString();
        ws.icon = q.value("icon").toString();
        ws.color = QColor(q.value("color").toString());
        ws.protectedMode = q.value("protected_mode").toInt() != 0;
        ws.isArchived = q.value("is_archived").toInt() != 0;
        ws.isPinned = q.value("is_pinned").toInt() != 0;
        ws.order = q.value("sort_order").toInt();
        ws.createdAt = QDateTime::fromString(q.value("created_at").toString(), Qt::ISODate);
        ws.updatedAt = QDateTime::fromString(q.value("updated_at").toString(), Qt::ISODate);
        if (!q.value("last_opened_at").isNull() && !q.value("last_opened_at").toString().isEmpty()) {
            ws.lastOpenedAt = QDateTime::fromString(q.value("last_opened_at").toString(), Qt::ISODate);
        }
        ws.taskCount = q.value("task_count").toInt();
        ws.noteCount = q.value("note_count").toInt();
        result.append(ws);
    }

    return result;
}

bool DatabaseManager::deleteWorkspace(const QUuid& id)
{
    QSqlQuery q(db());
    q.prepare("DELETE FROM workspaces WHERE id = :id;");
    q.bindValue(":id", uuidToDb(id));
    if (!q.exec()) {
        m_lastError = q.lastError().text();
        return false;
    }
    return true;
}


// Projects


QUuid DatabaseManager::addProject(const Project& proj)
{
    const QUuid id = proj.id.isNull() ? QUuid::createUuid() : proj.id;

    QSqlQuery q(db());
    q.prepare(R"(
        INSERT INTO projects (
            id, workspace_id, name, description, is_archived, created_at, updated_at
        ) VALUES (
            :id, :workspace_id, :name, :description, :is_archived, :created_at, :updated_at
        );
    )");

    q.bindValue(":id", uuidToDb(id));
    q.bindValue(":workspace_id", uuidToDb(proj.workspaceId));
    q.bindValue(":name", proj.name);
    q.bindValue(":description", proj.description);
    q.bindValue(":is_archived", proj.isArchived ? 1 : 0);
    q.bindValue(":created_at", proj.createdAt.isValid() ? proj.createdAt.toString(Qt::ISODate) : QDateTime::currentDateTime().toString(Qt::ISODate));
    q.bindValue(":updated_at", proj.updatedAt.isValid() ? proj.updatedAt.toString(Qt::ISODate) : QDateTime::currentDateTime().toString(Qt::ISODate));

    if (!q.exec()) {
        m_lastError = q.lastError().text();
        return {};
    }
    return id;
}

std::optional<Project> DatabaseManager::getProject(const QUuid& id) const
{
    QSqlQuery q(db());
    q.prepare("SELECT id, workspace_id, name, description, is_archived, created_at, updated_at FROM projects WHERE id = :id;");
    q.bindValue(":id", uuidToDb(id));
    if (!q.exec() || !q.next()) {
        return std::nullopt;
    }

    Project p;
    p.id = dbToUuid(q.value("id").toString());
    p.workspaceId = dbToUuid(q.value("workspace_id").toString());
    p.name = q.value("name").toString();
    p.description = q.value("description").toString();
    p.isArchived = q.value("is_archived").toInt() != 0;
    p.createdAt = QDateTime::fromString(q.value("created_at").toString(), Qt::ISODate);
    p.updatedAt = QDateTime::fromString(q.value("updated_at").toString(), Qt::ISODate);
    return p;
}

QList<Project> DatabaseManager::getProjectsByWorkspace(const QUuid& workspaceId) const
{
    QList<Project> result;
    QSqlQuery q(db());
    q.prepare("SELECT id, workspace_id, name, description, is_archived, created_at, updated_at FROM projects WHERE workspace_id = :workspace_id ORDER BY created_at ASC;");
    q.bindValue(":workspace_id", uuidToDb(workspaceId));

    if (!q.exec()) {
        m_lastError = q.lastError().text();
        return result;
    }

    while (q.next()) {
        Project p;
        p.id = dbToUuid(q.value("id").toString());
        p.workspaceId = dbToUuid(q.value("workspace_id").toString());
        p.name = q.value("name").toString();
        p.description = q.value("description").toString();
        p.isArchived = q.value("is_archived").toInt() != 0;
        p.createdAt = QDateTime::fromString(q.value("created_at").toString(), Qt::ISODate);
        p.updatedAt = QDateTime::fromString(q.value("updated_at").toString(), Qt::ISODate);
        result.append(p);
    }
    return result;
}

bool DatabaseManager::updateProject(const Project& proj)
{
    QSqlQuery q(db());
    q.prepare(R"(
        UPDATE projects SET
            name = :name,
            description = :description,
            is_archived = :is_archived,
            updated_at = :updated_at
        WHERE id = :id;
    )");
    q.bindValue(":id", uuidToDb(proj.id));
    q.bindValue(":name", proj.name);
    q.bindValue(":description", proj.description);
    q.bindValue(":is_archived", proj.isArchived ? 1 : 0);
    q.bindValue(":updated_at", proj.updatedAt.isValid() ? proj.updatedAt.toString(Qt::ISODate) : QDateTime::currentDateTime().toString(Qt::ISODate));

    if (!q.exec()) {
        m_lastError = q.lastError().text();
        return false;
    }
    return true;
}

bool DatabaseManager::deleteProject(const QUuid& id)
{
    QSqlQuery q(db());
    q.prepare("DELETE FROM projects WHERE id = :id;");
    q.bindValue(":id", uuidToDb(id));
    if (!q.exec()) {
        m_lastError = q.lastError().text();
        return false;
    }
    return true;
}


// Tasks


QUuid DatabaseManager::addTask(const Task& task)
{
    const QUuid id = task.id.isNull() ? QUuid::createUuid() : task.id;

    QSqlQuery q(db());
    q.prepare(R"(
        INSERT INTO tasks (
            id, workspace_id, project_id, title, description, status, priority,
            created_at, due_date, completed_at
        ) VALUES (
            :id, :workspace_id, :project_id, :title, :description, :status, :priority,
            :created_at, :due_date, :completed_at
        );
    )");

    q.bindValue(":id", uuidToDb(id));
    q.bindValue(":workspace_id", uuidToDb(task.workspaceId));
    q.bindValue(":project_id", task.projectId.isNull() ? QVariant() : uuidToDb(task.projectId));
    q.bindValue(":title", task.title);
    q.bindValue(":description", task.description);
    q.bindValue(":status", static_cast<int>(task.status));
    q.bindValue(":priority", static_cast<int>(task.priority));
    q.bindValue(":created_at", task.createdAt.isValid() ? task.createdAt.toString(Qt::ISODate) : QDateTime::currentDateTime().toString(Qt::ISODate));
    q.bindValue(":due_date", task.dueDate.isValid() ? task.dueDate.toString(Qt::ISODate) : QVariant());
    q.bindValue(":completed_at", task.completedAt.isValid() ? task.completedAt.toString(Qt::ISODate) : QVariant());

    if (!q.exec()) {
        m_lastError = q.lastError().text();
        return {};
    }

    // Insert subtasks if present
    for (const auto& st : task.subtasks) {
        SubTask sub = st;
        sub.taskId = id;
        addSubtask(sub);
    }

    return id;
}

std::optional<Task> DatabaseManager::getTask(const QUuid& id) const
{
    QSqlQuery q(db());
    q.prepare("SELECT id, workspace_id, project_id, title, description, status, priority, created_at, due_date, completed_at FROM tasks WHERE id = :id;");
    q.bindValue(":id", uuidToDb(id));

    if (!q.exec() || !q.next()) {
        return std::nullopt;
    }

    Task t;
    t.id = dbToUuid(q.value("id").toString());
    t.workspaceId = dbToUuid(q.value("workspace_id").toString());
    t.projectId = dbToUuid(q.value("project_id").toString());
    t.title = q.value("title").toString();
    t.description = q.value("description").toString();
    t.status = static_cast<TaskStatus>(q.value("status").toInt());
    t.priority = static_cast<TaskPriority>(q.value("priority").toInt());
    t.createdAt = QDateTime::fromString(q.value("created_at").toString(), Qt::ISODate);
    if (!q.value("due_date").isNull() && !q.value("due_date").toString().isEmpty()) {
        t.dueDate = QDateTime::fromString(q.value("due_date").toString(), Qt::ISODate);
    }
    if (!q.value("completed_at").isNull() && !q.value("completed_at").toString().isEmpty()) {
        t.completedAt = QDateTime::fromString(q.value("completed_at").toString(), Qt::ISODate);
    }

    t.subtasks = getSubtasks(t.id);
    return t;
}

QList<Task> DatabaseManager::getTasksByWorkspace(const QUuid& workspaceId) const
{
    QList<Task> result;
    QSqlQuery q(db());
    q.prepare("SELECT id, workspace_id, project_id, title, description, status, priority, created_at, due_date, completed_at FROM tasks WHERE workspace_id = :workspace_id ORDER BY created_at ASC;");
    q.bindValue(":workspace_id", uuidToDb(workspaceId));

    if (!q.exec()) {
        m_lastError = q.lastError().text();
        return result;
    }

    while (q.next()) {
        Task t;
        t.id = dbToUuid(q.value("id").toString());
        t.workspaceId = dbToUuid(q.value("workspace_id").toString());
        t.projectId = dbToUuid(q.value("project_id").toString());
        t.title = q.value("title").toString();
        t.description = q.value("description").toString();
        t.status = static_cast<TaskStatus>(q.value("status").toInt());
        t.priority = static_cast<TaskPriority>(q.value("priority").toInt());
        t.createdAt = QDateTime::fromString(q.value("created_at").toString(), Qt::ISODate);
        if (!q.value("due_date").isNull() && !q.value("due_date").toString().isEmpty()) {
            t.dueDate = QDateTime::fromString(q.value("due_date").toString(), Qt::ISODate);
        }
        if (!q.value("completed_at").isNull() && !q.value("completed_at").toString().isEmpty()) {
            t.completedAt = QDateTime::fromString(q.value("completed_at").toString(), Qt::ISODate);
        }
        t.subtasks = getSubtasks(t.id);
        result.append(t);
    }
    return result;
}

QList<Task> DatabaseManager::getTasksByProject(const QUuid& projectId) const
{
    QList<Task> result;
    QSqlQuery q(db());
    q.prepare("SELECT id, workspace_id, project_id, title, description, status, priority, created_at, due_date, completed_at FROM tasks WHERE project_id = :project_id ORDER BY created_at ASC;");
    q.bindValue(":project_id", uuidToDb(projectId));

    if (!q.exec()) {
        m_lastError = q.lastError().text();
        return result;
    }

    while (q.next()) {
        Task t;
        t.id = dbToUuid(q.value("id").toString());
        t.workspaceId = dbToUuid(q.value("workspace_id").toString());
        t.projectId = dbToUuid(q.value("project_id").toString());
        t.title = q.value("title").toString();
        t.description = q.value("description").toString();
        t.status = static_cast<TaskStatus>(q.value("status").toInt());
        t.priority = static_cast<TaskPriority>(q.value("priority").toInt());
        t.createdAt = QDateTime::fromString(q.value("created_at").toString(), Qt::ISODate);
        if (!q.value("due_date").isNull() && !q.value("due_date").toString().isEmpty()) {
            t.dueDate = QDateTime::fromString(q.value("due_date").toString(), Qt::ISODate);
        }
        if (!q.value("completed_at").isNull() && !q.value("completed_at").toString().isEmpty()) {
            t.completedAt = QDateTime::fromString(q.value("completed_at").toString(), Qt::ISODate);
        }
        t.subtasks = getSubtasks(t.id);
        result.append(t);
    }
    return result;
}

bool DatabaseManager::updateTask(const Task& task)
{
    QSqlQuery q(db());
    q.prepare(R"(
        UPDATE tasks SET
            project_id = :project_id,
            title = :title,
            description = :description,
            status = :status,
            priority = :priority,
            due_date = :due_date,
            completed_at = :completed_at
        WHERE id = :id;
    )");

    q.bindValue(":id", uuidToDb(task.id));
    q.bindValue(":project_id", task.projectId.isNull() ? QVariant() : uuidToDb(task.projectId));
    q.bindValue(":title", task.title);
    q.bindValue(":description", task.description);
    q.bindValue(":status", static_cast<int>(task.status));
    q.bindValue(":priority", static_cast<int>(task.priority));
    q.bindValue(":due_date", task.dueDate.isValid() ? task.dueDate.toString(Qt::ISODate) : QVariant());
    q.bindValue(":completed_at", task.completedAt.isValid() ? task.completedAt.toString(Qt::ISODate) : QVariant());

    if (!q.exec()) {
        m_lastError = q.lastError().text();
        return false;
    }

    // Synchronize subtasks
    QSqlQuery delQ(db());
    delQ.prepare("DELETE FROM subtasks WHERE task_id = :task_id;");
    delQ.bindValue(":task_id", uuidToDb(task.id));
    delQ.exec();

    for (const auto& st : task.subtasks) {
        SubTask sub = st;
        sub.taskId = task.id;
        addSubtask(sub);
    }

    return true;
}

bool DatabaseManager::deleteTask(const QUuid& id)
{
    QSqlQuery q(db());
    q.prepare("DELETE FROM tasks WHERE id = :id;");
    q.bindValue(":id", uuidToDb(id));
    if (!q.exec()) {
        m_lastError = q.lastError().text();
        return false;
    }
    return true;
}

// Subtasks


QUuid DatabaseManager::addSubtask(const SubTask& subtask)
{
    const QUuid id = subtask.id.isNull() ? QUuid::createUuid() : subtask.id;

    QSqlQuery q(db());
    q.prepare(R"(
        INSERT INTO subtasks (id, task_id, title, is_completed)
        VALUES (:id, :task_id, :title, :is_completed);
    )");
    q.bindValue(":id", uuidToDb(id));
    q.bindValue(":task_id", uuidToDb(subtask.taskId));
    q.bindValue(":title", subtask.title);
    q.bindValue(":is_completed", subtask.isCompleted ? 1 : 0);

    if (!q.exec()) {
        m_lastError = q.lastError().text();
        return {};
    }
    return id;
}

QList<SubTask> DatabaseManager::getSubtasks(const QUuid& taskId) const
{
    QList<SubTask> result;
    QSqlQuery q(db());
    q.prepare("SELECT id, task_id, title, is_completed FROM subtasks WHERE task_id = :task_id ORDER BY rowid ASC;");
    q.bindValue(":task_id", uuidToDb(taskId));

    if (!q.exec()) {
        m_lastError = q.lastError().text();
        return result;
    }

    while (q.next()) {
        SubTask st;
        st.id = dbToUuid(q.value("id").toString());
        st.taskId = dbToUuid(q.value("task_id").toString());
        st.title = q.value("title").toString();
        st.isCompleted = q.value("is_completed").toInt() != 0;
        result.append(st);
    }
    return result;
}

bool DatabaseManager::updateSubtask(const SubTask& subtask)
{
    QSqlQuery q(db());
    q.prepare("UPDATE subtasks SET title = :title, is_completed = :is_completed WHERE id = :id;");
    q.bindValue(":id", uuidToDb(subtask.id));
    q.bindValue(":title", subtask.title);
    q.bindValue(":is_completed", subtask.isCompleted ? 1 : 0);

    if (!q.exec()) {
        m_lastError = q.lastError().text();
        return false;
    }
    return true;
}

bool DatabaseManager::deleteSubtask(const QUuid& id)
{
    QSqlQuery q(db());
    q.prepare("DELETE FROM subtasks WHERE id = :id;");
    q.bindValue(":id", uuidToDb(id));
    if (!q.exec()) {
        m_lastError = q.lastError().text();
        return false;
    }
    return true;
}


// Notes

QUuid DatabaseManager::addNote(const Note& note)
{
    const QUuid id = note.id.isNull() ? QUuid::createUuid() : note.id;

    QSqlQuery q(db());
    q.prepare(R"(
        INSERT INTO notes (
            id, workspace_id, project_id, title, content, preview,
            is_pinned, is_archived, created_at, updated_at
        ) VALUES (
            :id, :workspace_id, :project_id, :title, :content, :preview,
            :is_pinned, :is_archived, :created_at, :updated_at
        );
    )");

    q.bindValue(":id", uuidToDb(id));
    q.bindValue(":workspace_id", uuidToDb(note.workspaceId));
    q.bindValue(":project_id", note.projectId.isNull() ? QVariant() : uuidToDb(note.projectId));
    q.bindValue(":title", note.title);
    q.bindValue(":content", note.content);
    q.bindValue(":preview", note.preview.isEmpty() ? note.content.left(160) : note.preview);
    q.bindValue(":is_pinned", note.isPinned ? 1 : 0);
    q.bindValue(":is_archived", note.isArchived ? 1 : 0);
    q.bindValue(":created_at", note.createdAt.isValid() ? note.createdAt.toString(Qt::ISODate) : QDateTime::currentDateTime().toString(Qt::ISODate));
    q.bindValue(":updated_at", note.updatedAt.isValid() ? note.updatedAt.toString(Qt::ISODate) : QDateTime::currentDateTime().toString(Qt::ISODate));

    if (!q.exec()) {
        m_lastError = q.lastError().text();
        return {};
    }
    return id;
}

std::optional<Note> DatabaseManager::getNote(const QUuid& id) const
{
    QSqlQuery q(db());
    q.prepare("SELECT id, workspace_id, project_id, title, content, preview, is_pinned, is_archived, created_at, updated_at FROM notes WHERE id = :id;");
    q.bindValue(":id", uuidToDb(id));

    if (!q.exec() || !q.next()) {
        return std::nullopt;
    }

    Note n;
    n.id = dbToUuid(q.value("id").toString());
    n.workspaceId = dbToUuid(q.value("workspace_id").toString());
    n.projectId = dbToUuid(q.value("project_id").toString());
    n.title = q.value("title").toString();
    n.content = q.value("content").toString();
    n.preview = q.value("preview").toString();
    n.isPinned = q.value("is_pinned").toInt() != 0;
    n.isArchived = q.value("is_archived").toInt() != 0;
    n.createdAt = QDateTime::fromString(q.value("created_at").toString(), Qt::ISODate);
    n.updatedAt = QDateTime::fromString(q.value("updated_at").toString(), Qt::ISODate);
    return n;
}

QList<Note> DatabaseManager::getNotesByWorkspace(const QUuid& workspaceId) const
{
    QList<Note> result;
    QSqlQuery q(db());
    q.prepare("SELECT id, workspace_id, project_id, title, content, preview, is_pinned, is_archived, created_at, updated_at FROM notes WHERE workspace_id = :workspace_id ORDER BY is_pinned DESC, updated_at DESC;");
    q.bindValue(":workspace_id", uuidToDb(workspaceId));

    if (!q.exec()) {
        m_lastError = q.lastError().text();
        return result;
    }

    while (q.next()) {
        Note n;
        n.id = dbToUuid(q.value("id").toString());
        n.workspaceId = dbToUuid(q.value("workspace_id").toString());
        n.projectId = dbToUuid(q.value("project_id").toString());
        n.title = q.value("title").toString();
        n.content = q.value("content").toString();
        n.preview = q.value("preview").toString();
        n.isPinned = q.value("is_pinned").toInt() != 0;
        n.isArchived = q.value("is_archived").toInt() != 0;
        n.createdAt = QDateTime::fromString(q.value("created_at").toString(), Qt::ISODate);
        n.updatedAt = QDateTime::fromString(q.value("updated_at").toString(), Qt::ISODate);
        result.append(n);
    }
    return result;
}

QList<Note> DatabaseManager::getNotesByProject(const QUuid& projectId) const
{
    QList<Note> result;
    QSqlQuery q(db());
    q.prepare("SELECT id, workspace_id, project_id, title, content, preview, is_pinned, is_archived, created_at, updated_at FROM notes WHERE project_id = :project_id ORDER BY is_pinned DESC, updated_at DESC;");
    q.bindValue(":project_id", uuidToDb(projectId));

    if (!q.exec()) {
        m_lastError = q.lastError().text();
        return result;
    }

    while (q.next()) {
        Note n;
        n.id = dbToUuid(q.value("id").toString());
        n.workspaceId = dbToUuid(q.value("workspace_id").toString());
        n.projectId = dbToUuid(q.value("project_id").toString());
        n.title = q.value("title").toString();
        n.content = q.value("content").toString();
        n.preview = q.value("preview").toString();
        n.isPinned = q.value("is_pinned").toInt() != 0;
        n.isArchived = q.value("is_archived").toInt() != 0;
        n.createdAt = QDateTime::fromString(q.value("created_at").toString(), Qt::ISODate);
        n.updatedAt = QDateTime::fromString(q.value("updated_at").toString(), Qt::ISODate);
        result.append(n);
    }
    return result;
}

bool DatabaseManager::updateNote(const Note& note)
{
    QSqlQuery q(db());
    q.prepare(R"(
        UPDATE notes SET
            project_id = :project_id,
            title = :title,
            content = :content,
            preview = :preview,
            is_pinned = :is_pinned,
            is_archived = :is_archived,
            updated_at = :updated_at
        WHERE id = :id;
    )");

    q.bindValue(":id", uuidToDb(note.id));
    q.bindValue(":project_id", note.projectId.isNull() ? QVariant() : uuidToDb(note.projectId));
    q.bindValue(":title", note.title);
    q.bindValue(":content", note.content);
    q.bindValue(":preview", note.preview.isEmpty() ? note.content.left(160) : note.preview);
    q.bindValue(":is_pinned", note.isPinned ? 1 : 0);
    q.bindValue(":is_archived", note.isArchived ? 1 : 0);
    q.bindValue(":updated_at", note.updatedAt.isValid() ? note.updatedAt.toString(Qt::ISODate) : QDateTime::currentDateTime().toString(Qt::ISODate));

    if (!q.exec()) {
        m_lastError = q.lastError().text();
        return false;
    }
    return true;
}

bool DatabaseManager::deleteNote(const QUuid& id)
{
    QSqlQuery q(db());
    q.prepare("DELETE FROM notes WHERE id = :id;");
    q.bindValue(":id", uuidToDb(id));
    if (!q.exec()) {
        m_lastError = q.lastError().text();
        return false;
    }
    return true;
}


// Attachments


QUuid DatabaseManager::addAttachment(const FileAttachment& att)
{
    const QUuid id = att.id.isNull() ? QUuid::createUuid() : att.id;

    QSqlQuery q(db());
    q.prepare(R"(
        INSERT INTO attachments (
            id, workspace_id, project_id, linked_entity_type, linked_entity_id,
            file_name, file_path, mime_type, file_size, created_at, updated_at
        ) VALUES (
            :id, :workspace_id, :project_id, :linked_entity_type, :linked_entity_id,
            :file_name, :file_path, :mime_type, :file_size, :created_at, :updated_at
        );
    )");

    q.bindValue(":id", uuidToDb(id));
    q.bindValue(":workspace_id", uuidToDb(att.workspaceId));
    q.bindValue(":project_id", att.projectId.isNull() ? QVariant() : uuidToDb(att.projectId));
    q.bindValue(":linked_entity_type", static_cast<int>(att.linkedEntityType));
    q.bindValue(":linked_entity_id", uuidToDb(att.linkedEntityId));
    q.bindValue(":file_name", att.fileName);
    q.bindValue(":file_path", att.relativePath);
    q.bindValue(":mime_type", att.mimeType);
    q.bindValue(":file_size", att.fileSize);
    q.bindValue(":created_at", att.createdAt.isValid() ? att.createdAt.toString(Qt::ISODate) : QDateTime::currentDateTime().toString(Qt::ISODate));
    q.bindValue(":updated_at", att.updatedAt.isValid() ? att.updatedAt.toString(Qt::ISODate) : QDateTime::currentDateTime().toString(Qt::ISODate));

    if (!q.exec()) {
        m_lastError = q.lastError().text();
        return {};
    }
    return id;
}

std::optional<FileAttachment> DatabaseManager::getAttachment(const QUuid& id) const
{
    QSqlQuery q(db());
    q.prepare("SELECT id, workspace_id, project_id, linked_entity_type, linked_entity_id, file_name, file_path, mime_type, file_size, created_at, updated_at FROM attachments WHERE id = :id;");
    q.bindValue(":id", uuidToDb(id));

    if (!q.exec() || !q.next()) {
        return std::nullopt;
    }

    FileAttachment a;
    a.id = dbToUuid(q.value("id").toString());
    a.workspaceId = dbToUuid(q.value("workspace_id").toString());
    a.projectId = dbToUuid(q.value("project_id").toString());
    a.linkedEntityType = static_cast<AttachmentEntityType>(q.value("linked_entity_type").toInt());
    a.linkedEntityId = dbToUuid(q.value("linked_entity_id").toString());
    a.fileName = q.value("file_name").toString();
    a.relativePath = q.value("file_path").toString();
    a.mimeType = q.value("mime_type").toString();
    a.fileSize = q.value("file_size").toLongLong();
    a.createdAt = QDateTime::fromString(q.value("created_at").toString(), Qt::ISODate);
    a.updatedAt = QDateTime::fromString(q.value("updated_at").toString(), Qt::ISODate);
    return a;
}

QList<FileAttachment> DatabaseManager::getAttachmentsForEntity(AttachmentEntityType entityType, const QUuid& entityId) const
{
    QList<FileAttachment> result;
    QSqlQuery q(db());
    q.prepare("SELECT id, workspace_id, project_id, linked_entity_type, linked_entity_id, file_name, file_path, mime_type, file_size, created_at, updated_at FROM attachments WHERE linked_entity_type = :entity_type AND linked_entity_id = :entity_id ORDER BY created_at ASC;");
    q.bindValue(":entity_type", static_cast<int>(entityType));
    q.bindValue(":entity_id", uuidToDb(entityId));

    if (!q.exec()) {
        m_lastError = q.lastError().text();
        return result;
    }

    while (q.next()) {
        FileAttachment a;
        a.id = dbToUuid(q.value("id").toString());
        a.workspaceId = dbToUuid(q.value("workspace_id").toString());
        a.projectId = dbToUuid(q.value("project_id").toString());
        a.linkedEntityType = static_cast<AttachmentEntityType>(q.value("linked_entity_type").toInt());
        a.linkedEntityId = dbToUuid(q.value("linked_entity_id").toString());
        a.fileName = q.value("file_name").toString();
        a.relativePath = q.value("file_path").toString();
        a.mimeType = q.value("mime_type").toString();
        a.fileSize = q.value("file_size").toLongLong();
        a.createdAt = QDateTime::fromString(q.value("created_at").toString(), Qt::ISODate);
        a.updatedAt = QDateTime::fromString(q.value("updated_at").toString(), Qt::ISODate);
        result.append(a);
    }
    return result;
}

QList<FileAttachment> DatabaseManager::getAttachmentsByWorkspace(const QUuid& workspaceId) const
{
    QList<FileAttachment> result;
    QSqlQuery q(db());
    q.prepare("SELECT id, workspace_id, project_id, linked_entity_type, linked_entity_id, file_name, file_path, mime_type, file_size, created_at, updated_at FROM attachments WHERE workspace_id = :workspace_id ORDER BY created_at ASC;");
    q.bindValue(":workspace_id", uuidToDb(workspaceId));

    if (!q.exec()) {
        m_lastError = q.lastError().text();
        return result;
    }

    while (q.next()) {
        FileAttachment a;
        a.id = dbToUuid(q.value("id").toString());
        a.workspaceId = dbToUuid(q.value("workspace_id").toString());
        a.projectId = dbToUuid(q.value("project_id").toString());
        a.linkedEntityType = static_cast<AttachmentEntityType>(q.value("linked_entity_type").toInt());
        a.linkedEntityId = dbToUuid(q.value("linked_entity_id").toString());
        a.fileName = q.value("file_name").toString();
        a.relativePath = q.value("file_path").toString();
        a.mimeType = q.value("mime_type").toString();
        a.fileSize = q.value("file_size").toLongLong();
        a.createdAt = QDateTime::fromString(q.value("created_at").toString(), Qt::ISODate);
        a.updatedAt = QDateTime::fromString(q.value("updated_at").toString(), Qt::ISODate);
        result.append(a);
    }
    return result;
}

QList<FileAttachment> DatabaseManager::getAttachmentsByProject(const QUuid& projectId) const
{
    QList<FileAttachment> result;
    QSqlQuery q(db());
    q.prepare("SELECT id, workspace_id, project_id, linked_entity_type, linked_entity_id, file_name, file_path, mime_type, file_size, created_at, updated_at FROM attachments WHERE project_id = :project_id ORDER BY created_at ASC;");
    q.bindValue(":project_id", uuidToDb(projectId));

    if (!q.exec()) {
        m_lastError = q.lastError().text();
        return result;
    }

    while (q.next()) {
        FileAttachment a;
        a.id = dbToUuid(q.value("id").toString());
        a.workspaceId = dbToUuid(q.value("workspace_id").toString());
        a.projectId = dbToUuid(q.value("project_id").toString());
        a.linkedEntityType = static_cast<AttachmentEntityType>(q.value("linked_entity_type").toInt());
        a.linkedEntityId = dbToUuid(q.value("linked_entity_id").toString());
        a.fileName = q.value("file_name").toString();
        a.relativePath = q.value("file_path").toString();
        a.mimeType = q.value("mime_type").toString();
        a.fileSize = q.value("file_size").toLongLong();
        a.createdAt = QDateTime::fromString(q.value("created_at").toString(), Qt::ISODate);
        a.updatedAt = QDateTime::fromString(q.value("updated_at").toString(), Qt::ISODate);
        result.append(a);
    }
    return result;
}

QList<FileAttachment> DatabaseManager::getAttachmentsByNote(const QUuid& noteId) const
{
    return getAttachmentsForEntity(AttachmentEntityType::Note, noteId);
}

bool DatabaseManager::updateAttachment(const FileAttachment& att)
{
    QSqlQuery q(db());
    q.prepare(R"(
        UPDATE attachments SET
            project_id = :project_id,
            linked_entity_type = :linked_entity_type,
            linked_entity_id = :linked_entity_id,
            file_name = :file_name,
            file_path = :file_path,
            mime_type = :mime_type,
            file_size = :file_size,
            updated_at = :updated_at
        WHERE id = :id;
    )");

    q.bindValue(":id", uuidToDb(att.id));
    q.bindValue(":project_id", att.projectId.isNull() ? QVariant() : uuidToDb(att.projectId));
    q.bindValue(":linked_entity_type", static_cast<int>(att.linkedEntityType));
    q.bindValue(":linked_entity_id", att.linkedEntityId.isNull() ? QVariant() : uuidToDb(att.linkedEntityId));
    q.bindValue(":file_name", att.fileName);
    q.bindValue(":file_path", att.relativePath);
    q.bindValue(":mime_type", att.mimeType);
    q.bindValue(":file_size", att.fileSize);
    q.bindValue(":updated_at", att.updatedAt.isValid() ? att.updatedAt.toString(Qt::ISODate) : QDateTime::currentDateTime().toString(Qt::ISODate));

    if (!q.exec()) {
        m_lastError = q.lastError().text();
        return false;
    }
    return true;
}

bool DatabaseManager::deleteAttachment(const QUuid& id)
{
    QSqlQuery q(db());
    q.prepare("DELETE FROM attachments WHERE id = :id;");
    q.bindValue(":id", uuidToDb(id));
    if (!q.exec()) {
        m_lastError = q.lastError().text();
        return false;
    }
    return true;
}

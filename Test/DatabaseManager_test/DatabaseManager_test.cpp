#include <QTest>
#include <QCoreApplication>
#include <QDir>
#include <QTemporaryDir>
#include "Data/Database/DatabaseManager.h"

class DatabaseManagerTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    void testOpenAndSchema();
    void testWorkspaceCrud();
    void testProjectCrud();
    void testTaskAndSubtaskCrud();
    void testNoteCrud();
    void testAttachmentCrud();
    void testCascadeDelete();
    void testTransactions();

private:
    QTemporaryDir* m_tempDir = nullptr;
    QString m_dbPath;
    DatabaseManager* m_dbManager = nullptr;
};

void DatabaseManagerTest::initTestCase()
{
    Q_INIT_RESOURCE(resources);

    m_tempDir = new QTemporaryDir();
    QVERIFY(m_tempDir->isValid());
    m_dbPath = m_tempDir->filePath("test_taskhelper.db");

    m_dbManager = new DatabaseManager("unit_test_connection");
    QVERIFY2(m_dbManager->open(m_dbPath), qPrintable(m_dbManager->lastError()));
    QVERIFY(m_dbManager->isOpen());
}

void DatabaseManagerTest::cleanupTestCase()
{
    if (m_dbManager) {
        m_dbManager->close();
        delete m_dbManager;
        m_dbManager = nullptr;
    }
    delete m_tempDir;
    m_tempDir = nullptr;
}

void DatabaseManagerTest::testOpenAndSchema()
{
    QVERIFY(m_dbManager->isOpen());
    QCOMPARE(m_dbManager->lastError(), QString());

    QVERIFY(m_dbManager->beginTransaction());
    QVERIFY(m_dbManager->commitTransaction());
}

void DatabaseManagerTest::testWorkspaceCrud()
{
    Workspace ws;
    ws.name = "Engineering Space";
    ws.type = "Work";
    ws.description = "Core software development workspace";
    ws.icon = "code";
    ws.color = QColor("#4F46E5");
    ws.protectedMode = false;
    ws.isArchived = false;
    ws.isPinned = true;
    ws.order = 1;
    ws.createdAt = QDateTime::currentDateTime();
    ws.updatedAt = ws.createdAt;
    ws.lastOpenedAt = ws.createdAt;
    ws.taskCount = 10;
    ws.noteCount = 4;

    QUuid wsId = m_dbManager->addWorkspace(ws);
    QVERIFY2(!wsId.isNull(), qPrintable(m_dbManager->lastError()));

    // Get
    auto fetched = m_dbManager->getWorkspace(wsId);
    QVERIFY(fetched.has_value());
    QCOMPARE(fetched->id, wsId);
    QCOMPARE(fetched->name, QString("Engineering Space"));
    QCOMPARE(fetched->type, QString("Work"));
    QCOMPARE(fetched->color.name().toUpper(), QString("#4F46E5"));
    QCOMPARE(fetched->isPinned, true);
    QCOMPARE(fetched->taskCount, 10);
    QCOMPARE(fetched->noteCount, 4);

    // Update
    ws.id = wsId;
    ws.name = "Engineering & Systems";
    ws.color = QColor("#10B981");
    ws.isArchived = true;
    QVERIFY2(m_dbManager->updateWorkspace(ws), qPrintable(m_dbManager->lastError()));

    auto updated = m_dbManager->getWorkspace(wsId);
    QVERIFY(updated.has_value());
    QCOMPARE(updated->name, QString("Engineering & Systems"));
    QCOMPARE(updated->color.name().toUpper(), QString("#10B981"));
    QCOMPARE(updated->isArchived, true);

    // Filtered vs All
    auto activeWorkspaces = m_dbManager->getAllWorkspaces(false);
    bool foundInActive = false;
    for (const auto& item : activeWorkspaces) {
        if (item.id == wsId) foundInActive = true;
    }
    QVERIFY(!foundInActive);

    auto allWorkspaces = m_dbManager->getAllWorkspaces(true);
    bool foundInAll = false;
    for (const auto& item : allWorkspaces) {
        if (item.id == wsId) foundInAll = true;
    }
    QVERIFY(foundInAll);

    // Reset archived to false for following tests
    ws.isArchived = false;
    QVERIFY(m_dbManager->updateWorkspace(ws));
}

void DatabaseManagerTest::testProjectCrud()
{
    auto workspaces = m_dbManager->getAllWorkspaces(false);
    QVERIFY(!workspaces.isEmpty());
    QUuid wsId = workspaces.first().id;

    Project proj;
    proj.workspaceId = wsId;
    proj.name = "Alpha Project";
    proj.description = "Top priority deliverables";
    proj.isArchived = false;
    proj.createdAt = QDateTime::currentDateTime();
    proj.updatedAt = proj.createdAt;

    QUuid projId = m_dbManager->addProject(proj);
    QVERIFY2(!projId.isNull(), qPrintable(m_dbManager->lastError()));

    auto fetched = m_dbManager->getProject(projId);
    QVERIFY(fetched.has_value());
    QCOMPARE(fetched->id, projId);
    QCOMPARE(fetched->workspaceId, wsId);
    QCOMPARE(fetched->name, QString("Alpha Project"));

    // Update
    proj.id = projId;
    proj.name = "Beta Project";
    proj.isArchived = true;
    QVERIFY2(m_dbManager->updateProject(proj), qPrintable(m_dbManager->lastError()));

    auto updated = m_dbManager->getProject(projId);
    QVERIFY(updated.has_value());
    QCOMPARE(updated->name, QString("Beta Project"));
    QCOMPARE(updated->isArchived, true);

    auto byWorkspace = m_dbManager->getProjectsByWorkspace(wsId);
    bool found = false;
    for (const auto& p : byWorkspace) {
        if (p.id == projId) found = true;
    }
    QVERIFY(found);

    // Reset archived
    proj.isArchived = false;
    QVERIFY(m_dbManager->updateProject(proj));
}

void DatabaseManagerTest::testTaskAndSubtaskCrud()
{
    auto workspaces = m_dbManager->getAllWorkspaces(false);
    QVERIFY(!workspaces.isEmpty());
    QUuid wsId = workspaces.first().id;

    auto projects = m_dbManager->getProjectsByWorkspace(wsId);
    QVERIFY(!projects.isEmpty());
    QUuid projId = projects.first().id;

    Task task;
    task.workspaceId = wsId;
    task.projectId = projId;
    task.title = "Implement Database Layer";
    task.description = "SQLite integration and migrations";
    task.status = TaskStatus::InProgress;
    task.priority = TaskPriority::High;
    task.createdAt = QDateTime::currentDateTime();
    task.dueDate = task.createdAt.addDays(3);

    QUuid taskId = m_dbManager->addTask(task);
    QVERIFY2(!taskId.isNull(), qPrintable(m_dbManager->lastError()));

    auto fetched = m_dbManager->getTask(taskId);
    QVERIFY(fetched.has_value());
    QCOMPARE(fetched->title, QString("Implement Database Layer"));
    QCOMPARE(fetched->priority, TaskPriority::High);
    QCOMPARE(fetched->status, TaskStatus::InProgress);
    QVERIFY(fetched->dueDate.isValid());

    // Update task
    task.id = taskId;
    task.title = "Implement Database Layer & Tests";
    task.status = TaskStatus::Completed;
    task.completedAt = QDateTime::currentDateTime();
    QVERIFY2(m_dbManager->updateTask(task), qPrintable(m_dbManager->lastError()));

    auto updated = m_dbManager->getTask(taskId);
    QVERIFY(updated.has_value());
    QCOMPARE(updated->title, QString("Implement Database Layer & Tests"));
    QCOMPARE(updated->status, TaskStatus::Completed);
    QVERIFY(updated->completedAt.isValid());

    // Test Subtasks
    SubTask st1;
    st1.taskId = taskId;
    st1.title = "Create schema.sql";
    st1.isCompleted = true;
    QUuid st1Id = m_dbManager->addSubtask(st1);
    QVERIFY2(!st1Id.isNull(), qPrintable(m_dbManager->lastError()));

    SubTask st2;
    st2.taskId = taskId;
    st2.title = "Write Unit tests";
    st2.isCompleted = false;
    QUuid st2Id = m_dbManager->addSubtask(st2);
    QVERIFY2(!st2Id.isNull(), qPrintable(m_dbManager->lastError()));

    auto subtasks = m_dbManager->getSubtasks(taskId);
    QCOMPARE(subtasks.size(), 2);

    st2.id = st2Id;
    st2.isCompleted = true;
    QVERIFY2(m_dbManager->updateSubtask(st2), qPrintable(m_dbManager->lastError()));

    QVERIFY(m_dbManager->deleteSubtask(st1Id));
    auto remainingSubtasks = m_dbManager->getSubtasks(taskId);
    QCOMPARE(remainingSubtasks.size(), 1);
    QCOMPARE(remainingSubtasks.first().id, st2Id);
}

void DatabaseManagerTest::testNoteCrud()
{
    auto workspaces = m_dbManager->getAllWorkspaces(false);
    QVERIFY(!workspaces.isEmpty());
    QUuid wsId = workspaces.first().id;

    auto projects = m_dbManager->getProjectsByWorkspace(wsId);
    QVERIFY(!projects.isEmpty());
    QUuid projId = projects.first().id;

    Note note;
    note.workspaceId = wsId;
    note.projectId = projId;
    note.title = "Database Architecture";
    note.content = "# Database Decision\nUsing SQLite with UUIDs for local structured persistence.";
    note.preview = "Using SQLite with UUIDs...";
    note.isPinned = true;
    note.isArchived = false;
    note.createdAt = QDateTime::currentDateTime();
    note.updatedAt = note.createdAt;

    QUuid noteId = m_dbManager->addNote(note);
    QVERIFY2(!noteId.isNull(), qPrintable(m_dbManager->lastError()));

    auto fetched = m_dbManager->getNote(noteId);
    QVERIFY(fetched.has_value());
    QCOMPARE(fetched->title, QString("Database Architecture"));
    QCOMPARE(fetched->isPinned, true);
    QCOMPARE(fetched->content, QString("# Database Decision\nUsing SQLite with UUIDs for local structured persistence."));

    // Update
    note.id = noteId;
    note.title = "Database Architecture (v2)";
    note.isPinned = false;
    QVERIFY2(m_dbManager->updateNote(note), qPrintable(m_dbManager->lastError()));

    auto updated = m_dbManager->getNote(noteId);
    QVERIFY(updated.has_value());
    QCOMPARE(updated->title, QString("Database Architecture (v2)"));
    QCOMPARE(updated->isPinned, false);

    auto notesList = m_dbManager->getNotesByWorkspace(wsId);
    bool found = false;
    for (const auto& n : notesList) {
        if (n.id == noteId) found = true;
    }
    QVERIFY(found);

    auto notesByProject = m_dbManager->getNotesByProject(projId);
    bool foundInProj = false;
    for (const auto& n : notesByProject) {
        if (n.id == noteId) foundInProj = true;
    }
    QVERIFY(foundInProj);
}

void DatabaseManagerTest::testAttachmentCrud()
{
    auto workspaces = m_dbManager->getAllWorkspaces(false);
    QVERIFY(!workspaces.isEmpty());
    QUuid wsId = workspaces.first().id;

    FileAttachment att;
    att.workspaceId = wsId;
    att.linkedEntityType = AttachmentEntityType::Workspace;
    att.linkedEntityId = wsId;
    att.fileName = "specs.pdf";
    att.relativePath = "attachments/specs.pdf";
    att.mimeType = "application/pdf";
    att.fileSize = 4096;
    att.createdAt = QDateTime::currentDateTime();
    att.updatedAt = att.createdAt;

    QUuid attId = m_dbManager->addAttachment(att);
    QVERIFY2(!attId.isNull(), qPrintable(m_dbManager->lastError()));

    auto fetched = m_dbManager->getAttachment(attId);
    QVERIFY(fetched.has_value());
    QCOMPARE(fetched->fileName, QString("specs.pdf"));
    QCOMPARE(fetched->fileSize, static_cast<qint64>(4096));

    auto byEntity = m_dbManager->getAttachmentsForEntity(AttachmentEntityType::Workspace, wsId);
    QVERIFY(!byEntity.isEmpty());
    QCOMPARE(byEntity.first().id, attId);

    auto byWorkspace = m_dbManager->getAttachmentsByWorkspace(wsId);
    QVERIFY(!byWorkspace.isEmpty());

    // Update attachment test
    att.id = attId;
    att.fileName = "specs_v2.pdf";
    att.fileSize = 8192;
    QVERIFY2(m_dbManager->updateAttachment(att), qPrintable(m_dbManager->lastError()));
    auto updatedAtt = m_dbManager->getAttachment(attId);
    QVERIFY(updatedAtt.has_value());
    QCOMPARE(updatedAtt->fileName, QString("specs_v2.pdf"));
    QCOMPARE(updatedAtt->fileSize, static_cast<qint64>(8192));

    QVERIFY(m_dbManager->deleteAttachment(attId));
    auto afterDelete = m_dbManager->getAttachment(attId);
    QVERIFY(!afterDelete.has_value());
}

void DatabaseManagerTest::testCascadeDelete()
{
    // Create an isolated hierarchy
    Workspace ws;
    ws.name = "Cascade Delete Space";
    ws.createdAt = QDateTime::currentDateTime();
    ws.updatedAt = ws.createdAt;
    QUuid wsId = m_dbManager->addWorkspace(ws);
    QVERIFY(!wsId.isNull());

    Project proj;
    proj.workspaceId = wsId;
    proj.name = "Cascade Project";
    proj.createdAt = QDateTime::currentDateTime();
    proj.updatedAt = proj.createdAt;
    QUuid projId = m_dbManager->addProject(proj);
    QVERIFY(!projId.isNull());

    Task task;
    task.workspaceId = wsId;
    task.projectId = projId;
    task.title = "Cascade Task";
    task.createdAt = QDateTime::currentDateTime();
    QUuid taskId = m_dbManager->addTask(task);
    QVERIFY(!taskId.isNull());

    SubTask st;
    st.taskId = taskId;
    st.title = "Cascade Subtask";
    QUuid stId = m_dbManager->addSubtask(st);
    QVERIFY(!stId.isNull());

    Note note;
    note.workspaceId = wsId;
    note.projectId = projId;
    note.title = "Cascade Note";
    note.createdAt = QDateTime::currentDateTime();
    note.updatedAt = note.createdAt;
    QUuid noteId = m_dbManager->addNote(note);
    QVERIFY(!noteId.isNull());

    // 1. Delete task -> check subtask was deleted by CASCADE
    QVERIFY(m_dbManager->deleteTask(taskId));
    QVERIFY(!m_dbManager->getTask(taskId).has_value());
    QVERIFY(m_dbManager->getSubtasks(taskId).isEmpty());

    // 2. Delete workspace -> check project and note were deleted by CASCADE
    QVERIFY(m_dbManager->deleteWorkspace(wsId));
    QVERIFY(!m_dbManager->getWorkspace(wsId).has_value());
    QVERIFY(!m_dbManager->getProject(projId).has_value());
    QVERIFY(!m_dbManager->getNote(noteId).has_value());
    QVERIFY(m_dbManager->getProjectsByWorkspace(wsId).isEmpty());
    QVERIFY(m_dbManager->getNotesByWorkspace(wsId).isEmpty());
}

void DatabaseManagerTest::testTransactions()
{
    QVERIFY(m_dbManager->beginTransaction());

    Workspace ws;
    ws.name = "Rollback Candidate";
    ws.createdAt = QDateTime::currentDateTime();
    ws.updatedAt = ws.createdAt;
    QUuid wsId = m_dbManager->addWorkspace(ws);
    QVERIFY(!wsId.isNull());

    // Rollback
    QVERIFY(m_dbManager->rollbackTransaction());

    // Record should not exist
    auto fetched = m_dbManager->getWorkspace(wsId);
    QVERIFY(!fetched.has_value());
}

QTEST_MAIN(DatabaseManagerTest)
#include "DatabaseManager_test.moc"

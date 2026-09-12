#include <QTest>
#include <QCoreApplication>
#include <QStandardPaths>
#include <QTemporaryDir>
#include "Data/workspace/WorkspaceRepository.h"

class WorkspaceRepositoryTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    void testWorkspaceCrud();
    void testProjectCrud();
    void testTaskCrud();
    void testNoteCrud();
    void testAttachmentCrud();
    void testWorkspaceDeletionCascades();

private:
    QTemporaryDir* m_tempDir = nullptr;
    WorkspaceRepository* m_repo = nullptr;
};

void WorkspaceRepositoryTest::initTestCase()
{
    Q_INIT_RESOURCE(resources);
    QStandardPaths::setTestModeEnabled(true);

    m_tempDir = new QTemporaryDir();
    QVERIFY(m_tempDir->isValid());

    m_repo = new WorkspaceRepository();
    QVERIFY(m_repo != nullptr);
}

void WorkspaceRepositoryTest::cleanupTestCase()
{
    delete m_repo;
    m_repo = nullptr;

    delete m_tempDir;
    m_tempDir = nullptr;
}

void WorkspaceRepositoryTest::testWorkspaceCrud()
{
    const int initialCount = m_repo->workspaces().size();

    // Create
    QUuid wsId = m_repo->createWorkspace("Engineering", "work", "Engineering department tasks");
    QVERIFY(!wsId.isNull());
    QCOMPARE(m_repo->workspaces().size(), initialCount + 1);

    // Get
    Workspace ws = m_repo->getWorkspaceById(wsId);
    QCOMPARE(ws.id, wsId);
    QCOMPARE(ws.name, QString("Engineering"));
    QCOMPARE(ws.type, QString("work"));
    QCOMPARE(ws.description, QString("Engineering department tasks"));

    // Update
    ws.name = "Engineering & Dev";
    ws.description = "Updated description";
    m_repo->updateWorkspace(ws);

    Workspace updatedWs = m_repo->getWorkspaceById(wsId);
    QCOMPARE(updatedWs.name, QString("Engineering & Dev"));
    QCOMPARE(updatedWs.description, QString("Updated description"));
}

void WorkspaceRepositoryTest::testProjectCrud()
{
    QUuid wsId = m_repo->createWorkspace("Product Space", "work", "");
    QVERIFY(!wsId.isNull());

    // Create Project
    Project project;
    project.workspaceId = wsId;
    project.name = "Mobile App v2";
    project.description = "QML client overhaul";

    QUuid projId = m_repo->createProject(project);
    QVERIFY(!projId.isNull());

    // Fetch
    Project fetched = m_repo->getProjectById(projId);
    QCOMPARE(fetched.id, projId);
    QCOMPARE(fetched.workspaceId, wsId);
    QCOMPARE(fetched.name, QString("Mobile App v2"));

    auto list = m_repo->getProjectsByWorkspace(wsId);
    QVERIFY(!list.isEmpty());
    bool found = false;
    for (const auto& p : list) {
        if (p.id == projId) found = true;
    }
    QVERIFY(found);

    // Update Project
    fetched.name = "Mobile App v2.1";
    m_repo->updateProject(fetched);
    Project updated = m_repo->getProjectById(projId);
    QCOMPARE(updated.name, QString("Mobile App v2.1"));

    // Delete Project
    m_repo->deleteProject(projId);
    QVERIFY(m_repo->getProjectById(projId).id.isNull());
}

void WorkspaceRepositoryTest::testTaskCrud()
{
    QUuid wsId = m_repo->createWorkspace("Task Space", "work", "");
    QUuid projId = QUuid::createUuid();

    Task task;
    task.workspaceId = wsId;
    task.projectId = projId;
    task.title = "Write Unit Tests";
    task.description = "Comprehensive domain coverage";
    task.priority = TaskPriority::High;
    task.status = TaskStatus::InProgress;

    QUuid taskId = m_repo->createTask(task);
    QVERIFY(!taskId.isNull());

    Task fetched = m_repo->getTaskById(taskId);
    QCOMPARE(fetched.id, taskId);
    QCOMPARE(fetched.title, QString("Write Unit Tests"));
    QCOMPARE(fetched.priority, TaskPriority::High);
    QCOMPARE(fetched.status, TaskStatus::InProgress);

    auto wsTasks = m_repo->getTasksByWorkspace(wsId);
    QVERIFY(!wsTasks.isEmpty());

    // Update
    fetched.title = "Write Unit Tests & Run CI";
    fetched.status = TaskStatus::Completed;
    m_repo->updateTask(fetched);

    Task updated = m_repo->getTaskById(taskId);
    QCOMPARE(updated.title, QString("Write Unit Tests & Run CI"));
    QCOMPARE(updated.status, TaskStatus::Completed);

    // Delete
    m_repo->deleteTask(taskId);
    QVERIFY(m_repo->getTaskById(taskId).id.isNull());
}

void WorkspaceRepositoryTest::testNoteCrud()
{
    QUuid wsId = m_repo->createWorkspace("Note Space", "personal", "");
    QUuid projId = QUuid::createUuid();

    Note note;
    note.workspaceId = wsId;
    note.projectId = projId;
    note.title = "Release Checklist";
    note.content = "## Checklist\n- [x] Fix build\n- [x] Add tests";
    note.preview = "Checklist preview";
    note.isPinned = true;

    QUuid noteId = m_repo->createNote(note);
    QVERIFY(!noteId.isNull());

    Note fetched = m_repo->getNoteById(noteId);
    QCOMPARE(fetched.id, noteId);
    QCOMPARE(fetched.title, QString("Release Checklist"));
    QCOMPARE(fetched.isPinned, true);

    // Update
    fetched.title = "Release Checklist v1.0";
    fetched.isPinned = false;
    m_repo->updateNote(fetched);

    Note updated = m_repo->getNoteById(noteId);
    QCOMPARE(updated.title, QString("Release Checklist v1.0"));
    QCOMPARE(updated.isPinned, false);

    // Delete
    m_repo->deleteNote(noteId);
    QVERIFY(m_repo->getNoteById(noteId).id.isNull());
}

void WorkspaceRepositoryTest::testAttachmentCrud()
{
    QUuid wsId = m_repo->createWorkspace("Attachment Space", "work", "");

    FileAttachment att;
    att.workspaceId = wsId;
    att.fileName = "diagram.png";
    att.relativePath = "attachments/diagram.png";
    att.fileSize = 1024;
    att.linkedEntityType = AttachmentEntityType::Workspace;

    QUuid attId = m_repo->createAttachment(att);
    QVERIFY(!attId.isNull());

    FileAttachment fetched = m_repo->getAttachmentById(attId);
    QCOMPARE(fetched.id, attId);
    QCOMPARE(fetched.fileName, QString("diagram.png"));
    QCOMPARE(fetched.fileSize, static_cast<qint64>(1024));

    auto list = m_repo->getAttachmentsByWorkspace(wsId);
    QVERIFY(!list.isEmpty());

    m_repo->deleteAttachment(attId);
    QVERIFY(m_repo->getAttachmentById(attId).id.isNull());
}

void WorkspaceRepositoryTest::testWorkspaceDeletionCascades()
{
    QUuid wsId = m_repo->createWorkspace("To Be Deleted", "temp", "");

    Project p;
    p.workspaceId = wsId;
    p.name = "Temp Project";
    QUuid projId = m_repo->createProject(p);

    Task t;
    t.workspaceId = wsId;
    t.projectId = projId;
    t.title = "Temp Task";
    QUuid taskId = m_repo->createTask(t);

    Note n;
    n.workspaceId = wsId;
    n.projectId = projId;
    n.title = "Temp Note";
    QUuid noteId = m_repo->createNote(n);

    // Delete workspace
    m_repo->deleteWorkspace(wsId);

    // Verify workspace and its children are cleaned up
    QVERIFY(m_repo->getWorkspaceById(wsId).id.isNull());
    QVERIFY(m_repo->getProjectById(projId).id.isNull());
    QVERIFY(m_repo->getTaskById(taskId).id.isNull());
    QVERIFY(m_repo->getNoteById(noteId).id.isNull());
}

QTEST_MAIN(WorkspaceRepositoryTest)
#include "WorkspaceRepository_test.moc"

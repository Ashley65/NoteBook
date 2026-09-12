#include <QTest>
#include <QCoreApplication>
#include <QStandardPaths>
#include <QTemporaryDir>
#include "Data/workspace/WorkspaceRepository.h"
#include "Data/workspace/Manager/TaskManager.h"

class TaskManagerTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    void testCreateAndFetchTask();
    void testTaskCompletionAndStatus();
    void testTodayAndOverdueFilters();
    void testSubtaskOperations();
    void testDeleteTask();

private:
    QTemporaryDir* m_tempDir = nullptr;
    WorkspaceRepository* m_repo = nullptr;
    TaskManager* m_taskManager = nullptr;
    QUuid m_workspaceId;
    QUuid m_projectId;
};

void TaskManagerTest::initTestCase()
{
    Q_INIT_RESOURCE(resources);
    QStandardPaths::setTestModeEnabled(true);

    m_tempDir = new QTemporaryDir();
    QVERIFY(m_tempDir->isValid());

    m_repo = new WorkspaceRepository();
    m_taskManager = new TaskManager(m_repo);

    m_workspaceId = m_repo->createWorkspace("Task Manager WS");
    Project proj;
    proj.workspaceId = m_workspaceId;
    proj.name = "Primary Project";
    m_projectId = m_repo->createProject(proj);
}

void TaskManagerTest::cleanupTestCase()
{
    delete m_taskManager;
    m_taskManager = nullptr;

    delete m_repo;
    m_repo = nullptr;

    delete m_tempDir;
    m_tempDir = nullptr;
}

void TaskManagerTest::testCreateAndFetchTask()
{
    TaskCreateRequest req;
    req.workspaceId = m_workspaceId;
    req.projectId = m_projectId;
    req.title = "Implement Security Check";
    req.description = "Audit dependencies and tokens";
    req.priority = TaskPriority::High;

    QUuid taskId = m_taskManager->createTask(req);
    QVERIFY(!taskId.isNull());

    Task task = m_taskManager->getTaskById(taskId);
    QCOMPARE(task.id, taskId);
    QCOMPARE(task.title, QString("Implement Security Check"));
    QCOMPARE(task.priority, TaskPriority::High);
    QCOMPARE(task.status, TaskStatus::Pending);

    auto all = m_taskManager->allTasks(m_workspaceId);
    bool found = false;
    for (const auto& t : all) {
        if (t.id == taskId) found = true;
    }
    QVERIFY(found);
}

void TaskManagerTest::testTaskCompletionAndStatus()
{
    TaskCreateRequest req;
    req.workspaceId = m_workspaceId;
    req.projectId = m_projectId;
    req.title = "Progress Tracking Task";
    QUuid taskId = m_taskManager->createTask(req);
    QVERIFY(!taskId.isNull());

    // Update status to InProgress
    m_taskManager->setTaskStatus(taskId, TaskStatus::InProgress);
    Task inProg = m_taskManager->getTaskById(taskId);
    QCOMPARE(inProg.status, TaskStatus::InProgress);

    // Complete task
    m_taskManager->setCompleted(taskId, true);
    Task completed = m_taskManager->getTaskById(taskId);
    QCOMPARE(completed.status, TaskStatus::Completed);
    QVERIFY(completed.completedAt.isValid());

    // Uncomplete task
    m_taskManager->setCompleted(taskId, false);
    Task pending = m_taskManager->getTaskById(taskId);
    QCOMPARE(pending.status, TaskStatus::Pending);
}

void TaskManagerTest::testTodayAndOverdueFilters()
{
    const QDateTime now = QDateTime::currentDateTime();

    // 1. Overdue task (yesterday)
    TaskCreateRequest overdueReq;
    overdueReq.workspaceId = m_workspaceId;
    overdueReq.projectId = m_projectId;
    overdueReq.title = "Overdue Report";
    overdueReq.dueDate = now.addDays(-2);
    QUuid overdueId = m_taskManager->createTask(overdueReq);
    QVERIFY(!overdueId.isNull());

    // 2. Today's task
    TaskCreateRequest todayReq;
    todayReq.workspaceId = m_workspaceId;
    todayReq.projectId = m_projectId;
    todayReq.title = "Today Meeting";
    todayReq.dueDate = now;
    QUuid todayId = m_taskManager->createTask(todayReq);
    QVERIFY(!todayId.isNull());

    auto overdueList = m_taskManager->overdueTasks(m_workspaceId);
    bool foundOverdue = false;
    for (const auto& t : overdueList) {
        if (t.id == overdueId) foundOverdue = true;
    }
    QVERIFY(foundOverdue);

    auto todayList = m_taskManager->todayTasks(m_workspaceId);
    bool foundToday = false;
    for (const auto& t : todayList) {
        if (t.id == todayId) foundToday = true;
    }
    QVERIFY(foundToday);
}

void TaskManagerTest::testSubtaskOperations()
{
    TaskCreateRequest req;
    req.workspaceId = m_workspaceId;
    req.projectId = m_projectId;
    req.title = "Parent Task with Subtasks";
    QUuid taskId = m_taskManager->createTask(req);
    QVERIFY(!taskId.isNull());

    // Add subtask
    m_taskManager->addSubtask(taskId, "Subtask 1");
    Task task = m_taskManager->getTaskById(taskId);
    QCOMPARE(task.subtasks.size(), 1);
    QCOMPARE(task.subtasks.first().title, QString("Subtask 1"));
    QCOMPARE(task.subtasks.first().isCompleted, false);

    QUuid subtaskId = task.subtasks.first().id;

    // Toggle subtask
    m_taskManager->toggleSubtask(taskId, subtaskId, true);
    task = m_taskManager->getTaskById(taskId);
    QCOMPARE(task.subtasks.first().isCompleted, true);

    // Update subtask title
    m_taskManager->updateSubtask(taskId, subtaskId, "Subtask 1 Updated", false);
    task = m_taskManager->getTaskById(taskId);
    QCOMPARE(task.subtasks.first().title, QString("Subtask 1 Updated"));
    QCOMPARE(task.subtasks.first().isCompleted, false);

    // Delete subtask
    m_taskManager->deleteSubtask(taskId, subtaskId);
    task = m_taskManager->getTaskById(taskId);
    QCOMPARE(task.subtasks.size(), 0);
}

void TaskManagerTest::testDeleteTask()
{
    TaskCreateRequest req;
    req.workspaceId = m_workspaceId;
    req.projectId = m_projectId;
    req.title = "Task To Delete";
    QUuid taskId = m_taskManager->createTask(req);
    QVERIFY(!taskId.isNull());

    m_taskManager->deleteTask(taskId);
    Task fetched = m_taskManager->getTaskById(taskId);
    QVERIFY(fetched.id.isNull());
}

QTEST_MAIN(TaskManagerTest)
#include "TaskManager_test.moc"

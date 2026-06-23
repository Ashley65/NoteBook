//
// Created by DevAccount on 27/11/2025.
//

#include <QEvent>
#include <QLabel>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QPushButton>
#include <QShowEvent>
#include <QTimer>
#include <QVariantMap>
#include <QWidget>
#include <QStringList>
#include <QDebug>
#include <QGridLayout>
#include <UI/mainWIndow.h>
#include <UI/components/SIde_Bar/sideBar.h>

#include "Data/workspace/WorkspaceCreateDialog.h"
#include "Data/workspace/WorkspaceDeleteDialog.h"
#include "Data/workspace/WorkspaceSettingsWindow.h"
#include "Data/workspace/WorkspaceSwitchDialog.h"
#include "Data/Project/ProjectCreateDialog.h"

#include "UI/components/SIde_Bar/componets/WorkspaceContextSection.h"

#ifdef _WIN32
#include <windows.h>
#include <windowsx.h>
#include <dwmapi.h>
#pragma comment(lib, "Dwmapi.lib")
#endif

// -- DWM helper --
#ifdef _WIN32
static void enableDarkTitleBar(HWND hwnd, const bool enable)
{
    constexpr DWORD DWMWA_USE_IMMERSIVE_DARK_MODE = 20; // Attribute for dark mode documented
    BOOL val = enable ? TRUE : FALSE; // Convert to BOOL type for Windows API call
    DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &val, sizeof(val)); // Set the attribute
}

static void setRoundedCorners(HWND hwnd, const int pref /*0=Default,1=DoNotRound,2=Round,3=RoundSmall*/)
{
    constexpr DWORD DWMWA_WINDOW_CORNER_PREFERENCE = 33; // Attribute for corner preference
    DwmSetWindowAttribute(hwnd, DWMWA_WINDOW_CORNER_PREFERENCE, &pref, sizeof(pref));
}

static void enableMica(HWND hwnd)
{
    // Windows 11 22H2+: System backdrop type
    const DWORD DWMWA_SYSTEMBACKDROP_TYPE = 38;
    const int DWMSBT_MAINWINDOW = 2; // Mica
    DwmSetWindowAttribute(hwnd, DWMWA_SYSTEMBACKDROP_TYPE, &DWMSBT_MAINWINDOW, sizeof(DWMSBT_MAINWINDOW));

    // Fallback for older Win11 builds
    const DWORD DWMWA_MICA_EFFECT = 1029; // legacy/undocumented on early builds
    const BOOL on = TRUE;
    DwmSetWindowAttribute(hwnd, DWMWA_MICA_EFFECT, &on, sizeof(on));
}

static void addStyle(HWND hwnd, const LONG_PTR add)
{
    LONG_PTR style = GetWindowLongPtr(hwnd, GWL_STYLE);
    style |= add;
    SetWindowLongPtr(hwnd, GWL_STYLE, style);
}


#endif

MainWindow::MainWindow(QWidget* parent)
{
    setWindowTitle("ChronoTasks");
    resize(1000, 700);

    m_workspaceRepo = new WorkspaceRepository(this);
    m_stateController = new AppStateController(this);
    m_tabManager = new TabManager(this);

    // Ensure the stylesheet allows transparency
    setStyleSheet("QMainWindow { background: transparent; }");

    // ============================================================
    // 1. MAIN CONTAINER (.container)
    // ============================================================

    auto *mainLayout = new QGridLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Initial stretches will be updated by resizeEvent/setupSideBar
    mainLayout->setColumnStretch(0, 0);
    mainLayout->setColumnStretch(1, 14);
    mainLayout->setColumnStretch(2, 10);

    mainLayout->setRowStretch(0, 2);
    mainLayout->setRowStretch(1, 18);
    mainLayout->setRowStretch(2, 10);

    // ============================================================
    // 2. TOP MENUBAR (.top_menubar)
    // ============================================================

    topBarFrame = new QFrame();
    topBarFrame->setObjectName("topBarFrame");
    mainLayout->addWidget(topBarFrame, 0, 0, 1, 3);

    topBarFrame->installEventFilter(this);
    setMouseTracking(true);

    // Internal Grid for Top Bar
    auto *topLayout = new QGridLayout(topBarFrame);
    topLayout->setContentsMargins(10, 8, 10, 8);
    topLayout->setSpacing(0);

    // Horizontal stretch: Left/Right fixed, Center expands
    topLayout->setColumnStretch(0, 0);
    topLayout->setColumnStretch(1, 1);
    topLayout->setColumnStretch(2, 0);

    // Vertical stretch: Two equal rows (TopLayer and ControlLayer)
    topLayout->setRowStretch(0, 1);
    topLayout->setRowStretch(1, 1);

    // --- WIDGET INITIALIZATION ---

    // InfoBar
    auto* infoBar = new InfoBar(topBarFrame);
    m_infoBar = infoBar;
    infoBar->setAppName("ChronoTasks");
    infoBar->setCurrentScreenLabel("");

    auto* infoContainer = new QFrame(topBarFrame);
    auto* infoLay = new QGridLayout(infoContainer);
    infoLay->setContentsMargins(0, 0, 0, 0);
    infoLay->setSpacing(0);
    infoLay->addWidget(infoBar, 0, 0);

#ifdef Q_OS_WIN
    // Let double-click on the InfoBar behave like a title bar toggle
    connect(infoBar, &InfoBar::maximiseRestoreRequested, this, [this]() {
        if (isMaximized()) {
            showNormal();
            if (haveNormalGeometry) {
                QTimer::singleShot(0, this, [this]{ setGeometry(lastNormalGeometry); });
            }
            if (m_windowActionsBar) m_windowActionsBar->setMaximised(false);
        } else {
            rememberNormalGeometry();
            showMaximized();
            if (m_windowActionsBar) m_windowActionsBar->setMaximised(true);
        }
    });
#else
    // On macOS/others, still allow double-click if it's implemented in InfoBar
    connect(infoBar, &InfoBar::maximiseRestoreRequested, this, [this]() {
        if (isMaximized() || isFullScreen()) {
            showNormal();
            if (m_windowActionsBar) m_windowActionsBar->setMaximised(false);
        } else {
            showFullScreen();
            if (m_windowActionsBar) m_windowActionsBar->setMaximised(true);
        }
    });
#endif

    // Window Actions Bar
    setupWindowActionsBar();

    // Navigation Bar
    m_navigationBar = new NavigationBar(topBarFrame);
    m_navigationBar->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    m_navigationBar->setMaximumWidth(100);
    connect(m_navigationBar, &NavigationBar::backClicked, this, &MainWindow::goBack);
    connect(m_navigationBar, &NavigationBar::forwardClicked, this, &MainWindow::goForward);
    connect(m_navigationBar, &NavigationBar::refreshClicked, this, &MainWindow::refreshPage);

    // Tab Bar
    m_tabBar = new TabBar(m_tabManager, topBarFrame);
    m_tabBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    // Menu Bar
    m_menuButtonBar = new MenuButtonBar(topBarFrame);

    // --- WIDGET PLACEMENT ---

    // ROW 0: The "TopLayer" (App Name & Window Controls)
#ifdef Q_OS_MAC
    // macOS: Controls left, Info right
    topLayout->addWidget(m_windowActionsBar, 0, 0, 1, 1, Qt::AlignLeft | Qt::AlignVCenter);
    topLayout->addWidget(infoContainer, 0, 1, 1, 1, Qt::AlignLeft | Qt::AlignVCenter);
    infoContainer->setAttribute(Qt::WA_TransparentForMouseEvents, false);
#else
    // Windows/Linux: Info left, Controls right
    topLayout->addWidget(infoContainer, 0, 0, 1, 2, Qt::AlignLeft | Qt::AlignVCenter);
    topLayout->addWidget(m_windowActionsBar, 0, 2, 1, 1, Qt::AlignRight | Qt::AlignTop);
#endif

    // ROW 1: The "controlLayer" (Navigation & Tabs)
    topLayout->addWidget(m_navigationBar, 1, 0, 1, 1, Qt::AlignLeft | Qt::AlignVCenter);
    topLayout->addWidget(m_tabBar, 1, 1, 1, 1);
    topLayout->addWidget(m_menuButtonBar, 1, 2, 1, 1, Qt::AlignRight | Qt::AlignVCenter);

    // ============================================================
    // 3. SIDE BAR (.Side_bar)
    // ============================================================

    sideBarFrame = new QFrame();
    mainLayout->addWidget(sideBarFrame, 1, 0, 2, 1);

    setupSideBar();
    setupSidebarConnections(); // (This now has the AppState forced-loading removed)

    // ============================================================
    // 4. CONTENT WINDOW (.Content_window)
    // ============================================================

    m_mainContent = new MainContentView(m_workspaceRepo, this);
    mainLayout->addWidget(m_mainContent, 1, 1, 2, 2);
    m_mainContent->installEventFilter(this);

    // ============================================================
    // 5. EVENT ROUTING & STARTUP
    // ============================================================

    // --- 5A. Tab Manager Routing (Must happen BEFORE adding initial tabs!) ---
    connect(m_tabManager, &TabManager::tabOpened, this, [this](const QString& viewType, const QUuid& contextId) {
        qDebug() << "Tab opened with viewType:" << viewType << "and contextId:" << contextId;

        if (viewType == "Home") {
            const Workspace ws = m_workspaceRepo->getWorkspaceById(contextId);
            if (!ws.id.isNull()) {
                m_mainContent->setActiveWorkspace(ws);
                m_stateController->setActiveWorkspace(ws.id, ws.name);
                if (m_sideBar) {
                    m_sideBar->setWorkspaceId(ws.id);
                    m_sideBar->setWorkspaceName(ws.name);
                    m_sideBar->setActiveProjectId(QUuid());
                }
            }
        } else if (viewType == "Project") {
            const Project p = m_workspaceRepo->getProjectById(contextId);
            if (!p.id.isNull()) {
                const Workspace ws = m_workspaceRepo->getWorkspaceById(p.workspaceId);
                if (!ws.id.isNull()) {
                    m_stateController->setActiveWorkspace(ws.id, ws.name);
                    if (m_sideBar) {
                        m_sideBar->setWorkspaceId(ws.id);
                        m_sideBar->setWorkspaceName(ws.name);
                    }
                }
                m_mainContent->setActiveProject(p);
                if (m_sideBar) {
                    m_sideBar->setActiveProjectId(p.id);
                }
            }
        }
    });

    // Handle Tab Destructions safely
    connect(m_tabManager, &TabManager::tabDiscarded, this, [this](const QUuid& contextId) {
        if (m_mainContent) {
            m_mainContent->discardView(contextId);
        }
    });

    // Optional: If you implemented the dynamic title signal in MainContentView!
    // connect(m_mainContent, &MainContentView::viewTitleChanged, m_tabManager, &TabManager::updateTabTitle);

    // --- 5B. Database Check / Seed Defaults ---
    auto workspaces = m_workspaceRepo->workspaces();
    if (workspaces.isEmpty()) {
        m_workspaceRepo->createWorkspace("Personal Workspace");
        workspaces = m_workspaceRepo->workspaces();
    }

    bool hasTestWorkspace = false;
    for (const Workspace& existingWorkspace : workspaces) {
        if (existingWorkspace.name == "Test Workspace") {
            hasTestWorkspace = true;
            break;
        }
    }
    if (!hasTestWorkspace) {
        m_workspaceRepo->createWorkspace("Test Workspace", "lab", "This is a test workspace.");
        workspaces = m_workspaceRepo->workspaces(); // Refresh list
    }

    // --- 5C. Add Initial Tab From State ---
    if (!workspaces.isEmpty()) {
        QUuid launchWorkspaceId = m_stateController->context().activeWorkspaceId;

        // 1. Fallback to first workspace if state is empty
        if (launchWorkspaceId.isNull()) {
            launchWorkspaceId = workspaces.first().id;
        }

        // 2. Fetch project ID AFTER ensuring the workspace ID is valid
        QUuid launchProjectId = m_stateController->lastProjectForWorkspace(launchWorkspaceId);

        const Workspace launchWs = m_workspaceRepo->getWorkspaceById(launchWorkspaceId);

        if (!launchProjectId.isNull()) {
            // Restore last open project
            const Project p = m_workspaceRepo->getProjectById(launchProjectId);
            if (!p.id.isNull()) {
                QString tabTitle = QString("%1 / %2").arg(launchWs.name, p.name);
                m_tabManager->addTab(tabTitle, "Project", p.id, "#81C784");
                m_tabManager->setActiveTabId(p.id); // Explicitly tell the manager this is the active tab
            } else {
                m_tabManager->addTab(launchWs.name, "Home", launchWs.id, "#3B82F6");
                m_tabManager->setActiveTabId(launchWs.id); // Explicitly mark as active
            }
        } else {
            // Restore home view
            m_tabManager->addTab(launchWs.name, "Home", launchWs.id, "#3B82F6");
            m_tabManager->setActiveTabId(launchWs.id); // Explicitly mark as active
        }
    }

    // ============================================================
    // 6. FLOATING UI AND THEME
    // ============================================================

    m_floatingToggleButton = new QPushButton(m_mainContent);
    m_floatingToggleButton->setText("☰");
    m_floatingToggleButton->setToolTip("Show Sidebar");
    m_floatingToggleButton->setFixedSize(36, 36);
    m_floatingToggleButton->setStyleSheet(R"(
        QPushButton {
            background-color: #2A2D3D;
            border: none;
            border-radius: 18px;
            color: white;
            font-size: 18px;
        }
        QPushButton:hover {
            background-color: #3A3D4D;
        }
    )");

    connect(m_floatingToggleButton, &QPushButton::clicked, this, [this]() {
        if (m_sideBar) {
            m_sideBar->setMode(SideBar::Mode::Default);
            updateFloatingToggleButtonVisibility();
        }
    });

    m_floatingToggleButton->hide();

    // Apply theme to topBarFrame on startup
    updateWindowTheme();
}

void MainWindow::goBack()
{
    // TODO: Implement navigation back action and intergate with history management
    // This is a placeholder for the actual implementation

}

void MainWindow::goForward()
{
    // TODO: Implement navigation forward action and intergate with history management
    // This is a placeholder for the actual implementation
}

void MainWindow::refreshPage()
{
    // TODO: Implement page refresh action and intergate with content reloading
    // This is a placeholder for the actual implementation
}

QFrame* MainWindow::createWidget(const QString& title, const QString& color,
                                 const QString& extraStyle, bool withBorder, bool transparent)
{
    auto *Frame = new QFrame();
    if (withBorder) {
        Frame->setFrameShape(QFrame::Box);
    } else {
        Frame->setFrameShape(QFrame::NoFrame);
    }

    QString style;
    if (transparent) {
        style = QString("QFrame { background-color: transparent; border: none; %1 }").arg(extraStyle);
    } else {
        style = QString("QFrame { background-color: %1; %2 %3 }")
                    .arg(color,
                         withBorder ? "border: 1px solid rgba(0,0,0,0.2);" : "border: none;",
                         extraStyle);
    }
    Frame->setStyleSheet(style);

    auto *layout = new QGridLayout(Frame);
    auto *label = new QLabel(title);
    label->setAlignment(Qt::AlignCenter);
    label->setStyleSheet("border: none; font-weight: bold; font-size: 14px;");
    layout->addWidget(label);
    return Frame;
}


void MainWindow::setupWindowActionsBar()
{
    m_windowActionsBar = new WindowsActionsBar(this);
    m_windowActionsBar->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
    m_windowActionsBar->setMaximumWidth(220); // keep the bar compact

    // Connect signals to appropriate slots
    connect(m_windowActionsBar, &WindowsActionsBar::minimiseRequested, this, [this]() {
        this->showMinimized();
    });

    connect(m_windowActionsBar, &WindowsActionsBar::maximiseRestoreRequested, this, [this]() {
    if (isMaximized() || isFullScreen()) {
        // Restore
        showNormal();
#ifdef Q_OS_WIN
        if (haveNormalGeometry) {
            // Defer setting the geometry until after the normal state is applied
            QTimer::singleShot(0, this, [this]{ setGeometry(lastNormalGeometry); });
        }
#endif
        m_windowActionsBar->setMaximised(false);
    } else {
        // Save current normal geometry before maximizing
#ifdef Q_OS_WIN
        rememberNormalGeometry();
        showMaximized();
#else
        showFullScreen();
#endif
        m_windowActionsBar->setMaximised(true);
    }
});


    connect(m_windowActionsBar, &WindowsActionsBar::closeRequested, this, [this]() {
        this->close();
    });
}

void MainWindow::setupInfoBar()
{
    // To be implemented: setup for InfoBar
    // The idea is to allow for the infor bar to read the current content window and display the name
}

void MainWindow::setupNavigationBar()
{
    // TODO: implemented: setup for NavigationBar
    // The idea is to allow for the navigation bar to control navigation within the content window
}

void MainWindow::updateFloatingToggleButtonVisibility()
{

    if (!m_sideBar || !m_floatingToggleButton) return;

    // Check if the sidebar is in Hidden mode
    bool isHidden = (m_sideBar->mode() == SideBar::Mode::Hidden);

    // Explicitly set visibility
    m_floatingToggleButton->setVisible(isHidden);

    if (isHidden) {
        QWidget* parent = m_floatingToggleButton->parentWidget();
        if (parent) {
            int margin = 15;
            int x = margin;
            int y = parent->height() - m_floatingToggleButton->height() - margin;

            m_floatingToggleButton->move(x, y);
            m_floatingToggleButton->raise(); // Critical: Ensure it's on top of content
        }
    }
}

void MainWindow::setupSideBar() {
    if (!sideBarFrame) return;

    sideBarFrame->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
    sideBarFrame->setStyleSheet("background-color: transparent;");

    auto* lay = new QVBoxLayout(sideBarFrame);
    lay->setContentsMargins(0, 0, 0, 0); // Removed large margins around the sidebar widget
    lay->setSpacing(0);

    m_sideBar = new SideBar(sideBarFrame);
    lay->addWidget(m_sideBar);
    lay->addWidget(m_sideBar, 0, Qt::AlignLeft);
    lay->setAlignment(m_sideBar, Qt::AlignLeft);

    SideBar::Mode m = SideBar::Mode::Default;
    const int w = width();
    if (w < 800)       m = SideBar::Mode::Hidden;
    else if (w < 1200) m = SideBar::Mode::Compact;
    m_sideBar->setMode(m);

    // Apply initial stretches based on mode
    auto* grid = qobject_cast<QGridLayout*>(layout());
    if (grid) {
        if (m != SideBar::Mode::Default) {
            grid->setColumnStretch(0, 0);
            grid->setColumnStretch(1, 14);
            grid->setColumnStretch(2, 10);
        } else {
            grid->setColumnStretch(0, 0);
            grid->setColumnStretch(1, 14);
            grid->setColumnStretch(2, 10);
        }
    }


    updateFloatingToggleButtonVisibility();
}

void MainWindow::setupSidebarConnections()
{
    if (!m_sideBar || !m_stateController || !m_workspaceRepo) return;

    const auto refreshSidebarProjects = [this]() {
        const QUuid workspaceId = m_stateController->context().activeWorkspaceId;
        QVariantList projectItems;

        if (!workspaceId.isNull()) {
            const QList<Project> projects = m_workspaceRepo->getProjectsByWorkspace(workspaceId);
            static const QStringList palette = {
                "#FFD700", "#9ACD32", "#20B2AA", "#FF69B4", "#64B5F6", "#BA68C8", "#FF8A65", "#81C784"
            };

            for (const Project& project : projects) {
                if (project.isArchived) {
                    continue;
                }

                QVariantMap item;
                item.insert("id", project.id.toString(QUuid::WithoutBraces));
                item.insert("name", project.name);
                item.insert("colorCode", palette.at(qAbs(qHash(project.id.toString())) % palette.size()));
                projectItems.append(item);
            }
        }

        m_sideBar->setProjects(projectItems);
    };

    // React to State Changes
    connect(m_stateController, &AppStateController::activeWorkspaceChanged,
            this, [this]() {
        const AppContext& ctx = m_stateController->context();
        // Update the InfoBar top label
        if (m_infoBar) {
            m_infoBar->setCurrentScreenLabel(ctx.activeWorkspaceName);
        }
        // Update the Sidebar label and restore last project if present
        if (m_sideBar) {
            m_sideBar->setWorkspaceName(ctx.activeWorkspaceName);
            m_sideBar->setWorkspaceId(ctx.activeWorkspaceId);

            // Try to restore the last project for this workspace
            if (!ctx.activeWorkspaceId.isNull() && m_stateController) {
                const QUuid lastProj = m_stateController->lastProjectForWorkspace(ctx.activeWorkspaceId);
                if (!lastProj.isNull() && m_workspaceRepo) {
                    const Project p = m_workspaceRepo->getProjectById(lastProj);
                    if (!p.id.isNull()) {
                        m_sideBar->setActiveProjectId(p.id);
                    } else {
                        m_sideBar->setActiveProjectId(QUuid());
                    }
                } else {
                    m_sideBar->setActiveProjectId(QUuid());
                }
            } else {
                m_sideBar->setActiveProjectId(QUuid());
            }
        }

        if (ctx.activeWorkspaceId.isNull()) {
            return;
        }

        const Workspace ws = m_workspaceRepo->getWorkspaceById(ctx.activeWorkspaceId);
        if (!ws.id.isNull()) {
            // Activate the workspace view
            m_mainContent->setActiveWorkspace(ws);

            // If there's a remembered last project for this workspace, show it
            const QUuid lastProj = m_stateController->lastProjectForWorkspace(ctx.activeWorkspaceId);
            if (!lastProj.isNull()) {
                const Project p = m_workspaceRepo->getProjectById(lastProj);
                if (!p.id.isNull()) {
                    m_mainContent->setActiveProject(p);
                }
            }

            return;
        }

        // Fallback for stale ids that may exist in persisted AppStateController settings.
    });

    connect(m_stateController, &AppStateController::activeWorkspaceChanged,
            this, [refreshSidebarProjects]() {
        refreshSidebarProjects();
    });

    // Handle Sidebar Intent
    connect(m_sideBar, &SideBar::workspaceSwitchRequested, this, [this](){
        WorkspaceSwitchDialog dlg(
                m_workspaceRepo->workspaces(),
                m_stateController->context().activeWorkspaceId,
                this
            );

        if (dlg.exec() == QDialog::Accepted) {
            const QUuid id = dlg.selectedWorkspaceId();
            const Workspace ws = m_workspaceRepo->getWorkspaceById(id);
            m_tabManager->addTab(ws.name, "Home", ws.id, "#3B82F6");
        }
    });

    // Handle Workspace Menu delete Request
    connect(m_sideBar, &SideBar::workspaceDeleteRequested, this, [this]()
    {
        WorkspaceDeleteDialog dlg(m_workspaceRepo->workspaces(), this);
        if (dlg.exec() == QDialog::Accepted) {
            const QUuid id = dlg.selectedWorkspaceId();
            m_workspaceRepo->deleteWorkspace(id);
            // Optionally refresh the UI or switch to another workspace here
        }
    });

    // Handle workspace setting window
    connect(m_sideBar, &SideBar::workspaceSettingsRequested, this, [this](const QUuid& activeId) {
        if (activeId.isNull())
            return;

        const Workspace ws = m_workspaceRepo->getWorkspaceById(activeId);
        WorkspaceSettingsWindow dlg(activeId, ws.name, m_workspaceRepo, this);

        const QString titleName = ws.name.isEmpty() ? activeId.toString(QUuid::WithoutBraces) : ws.name;
        dlg.setWindowTitle(tr("Workspace Settings: \"%1\"").arg(titleName));

        dlg.exec(); // modal settings window
    });

    // Handle Workspace Menu create Request
    connect(m_sideBar, &SideBar::workspaceCreateRequested, this, [this]() {
        WorkspaceCreateDialog dlg(this, m_workspaceRepo);
        if (dlg.exec() == QDialog::Accepted) {
            const QString newName = dlg.workspaceName().trimmed();
            const QString type = dlg.workspaceType();
            const QString description = dlg.workspaceDescription();

            if (newName.isEmpty()) return;

            const QUuid newId = m_workspaceRepo->createWorkspace(newName, type, description);
            if (!newId.isNull()) {
                m_stateController->setActiveWorkspace(newId, newName);
            }
        }
    });

    connect(m_sideBar, &SideBar::projectCreateRequested, this, [this]() {
        const QUuid activeWorkspaceId = m_stateController->context().activeWorkspaceId;
        if (activeWorkspaceId.isNull()) {
            return;
        }

        ProjectCreateDialog dlg(activeWorkspaceId, this, m_workspaceRepo);
        if (dlg.exec() != QDialog::Accepted) {
            return;
        }

        Project project;
        project.workspaceId = activeWorkspaceId;
        project.name = dlg.projectName();
        project.description = dlg.projectDescription();
        project.isArchived = false;

        const QUuid newProjectId = m_workspaceRepo->createProject(project);
        if (m_sideBar && !newProjectId.isNull()) {
            m_sideBar->setActiveProjectId(newProjectId);
        }
    });

    connect(m_sideBar, &SideBar::projectSelected, this, [this](const QUuid& projectId){

    if (projectId.isNull() || !m_workspaceRepo || !m_stateController) return;

    const Project project = m_workspaceRepo->getProjectById(projectId);
    if (project.id.isNull()) return;


    m_sideBar->setActiveProjectId(projectId);


    m_stateController->setLastProjectForWorkspace(project.workspaceId, project.id);


    const Workspace ws = m_workspaceRepo->getWorkspaceById(project.workspaceId);

    QString tabTitle = project.name;
    if (!ws.id.isNull()) {

        tabTitle = QString("%1 / %2").arg(ws.name, project.name);
    }

    m_tabManager->addTab(tabTitle, "Project", project.id, "#81C784");
});

    connect(m_workspaceRepo, &WorkspaceRepository::projectAdded, this, [refreshSidebarProjects](const Project&) {
        refreshSidebarProjects();
    });
    connect(m_workspaceRepo, &WorkspaceRepository::projectUpdated, this, [refreshSidebarProjects](const Project&) {
        refreshSidebarProjects();
    });
    connect(m_workspaceRepo, &WorkspaceRepository::projectDeleted, this, [refreshSidebarProjects](const QUuid&) {
        refreshSidebarProjects();
    });

    refreshSidebarProjects();
}
void MainWindow::resizeEvent(QResizeEvent* e) {
    QWidget::resizeEvent(e);

    if (m_sideBar) {
        SideBar::Mode m = SideBar::Mode::Default;
        const int w = width();
        if (w < 800)      m = SideBar::Mode::Hidden;
        else if (w < 1200) m = SideBar::Mode::Compact;
        m_sideBar->setMode(m);

        // Dynamically adjust column stretches to allow scaling in Default mode
        auto* grid = qobject_cast<QGridLayout*>(layout());
        if (grid) {
            if (m == SideBar::Mode::Default) {
                grid->setColumnStretch(0, 0);  // Give sidebar 20% stretch (6/30)
                grid->setColumnStretch(1, 14); 
                grid->setColumnStretch(2, 10);
            } else {
                grid->setColumnStretch(0, 0);  // Fixed width for Compact/Hidden
                grid->setColumnStretch(1, 14);
                grid->setColumnStretch(2, 10);
            }
        }
        updateFloatingToggleButtonVisibility();
    }

}
void MainWindow::updateWindowTheme()

{
    const bool darkMode = isDarkModeEnabled();

    // Style the topBarFrame based on dark/light mode
    if (topBarFrame) {
        if (darkMode) {

            topBarFrame->setStyleSheet(
                "QFrame#topBarFrame {"
                "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #1e1e1e, stop:1 #161616);"
                "  border-bottom: 1px solid rgba(255, 255, 255, 0.04);"
                "}"
            );
        } else {

            topBarFrame->setStyleSheet(
                "QFrame#topBarFrame {"
                "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #f7f7f7, stop:1 #efefef);"
                "  border-bottom: 1px solid rgba(0, 0, 0, 0.06);"
                "}"
            );
        }
    }

    // Update title bar and other elements based on dark mode status
    if (auto infoBar = topBarFrame->findChild<InfoBar*>())
    {
        if (darkMode)
        {
            infoBar->setStyleSheet("background-color: #1e1e1e; color: #ffffff;");
        }
        else
        {
            infoBar->setStyleSheet("background-color: #f0f0f0; color: #000000;");
        }
    }
    if (m_navigationBar)
    {
        const QString btnCss = darkMode
            ? "QPushButton { padding: 0px; border: none; border-radius: 4px; }"
              "QPushButton:hover { background: #a8c0ff; }"
              "QPushButton:pressed { background-color: rgba(255,255,255,0.32); }"
            : "QPushButton { padding: 0px; border: none; border-radius: 4px; }"
              "QPushButton:hover { background: #d2e3fc; }"
              "QPushButton:pressed { background-color: rgba(0,0,0,0.08); }";

        m_navigationBar->setButtonStyleSheet(btnCss);
        m_navigationBar->updateIcons(darkMode);
    }
    if (m_menuButtonBar) {
        const QString css = darkMode
            ? "QPushButton { padding: 0px; border: none; border-radius: 4px; }"
              "QPushButton:hover { background: #a8c0ff; }"
              "QPushButton:pressed { background-color: rgba(255,255,255,0.32); }"
            : "QPushButton { padding: 0px; border: none; border-radius: 4px; }"
              "QPushButton:hover { background: #d2e3fc; }"
              "QPushButton:pressed { background-color: rgba(0,0,0,0.08); }";
        m_menuButtonBar->setButtonStyleSheet(css);
    }
    // WindowsActionsBar
    if (m_windowActionsBar) {
        const QString compactCssDark =
            "QPushButton{ padding: 0 6px; min-width: 0; min-height: 0; border: none; }"
            "QPushButton:hover { background: #a8c0ff; }";

        const QString compactCssLight =
            "QPushButton{ padding: 0 6px; min-width: 0; min-height: 0; border: none; }"
            "QPushButton:hover { background: #d2e3fc; }";

        m_windowActionsBar->setButtonStyleSheet(
            darkMode ? compactCssDark : compactCssLight
        );
    }
}

bool MainWindow::isDarkModeEnabled() const
{
    // Implement platform-specific dark mode detection to help dected theme changes
    // Currently just a simple implementation based on palette lightness
    const QPalette pal = qApp->palette();
    const QColor base = pal.window().color();
    return base.lightness() < 128;
    
}

void MainWindow::debugTopBarGrid()
{
    if (!topBarFrame) return;

    // Colors for different widgets
    const QStringList colors = {
        "#FF6B6B", // Red
        "#4ECDC4", // Teal
        "#FFE66D", // Yellow
        "#95E1D3", // Mint
        "#C7CEEA", // Lavender
        "#FF9999", // Light Red
        "#FFCC99", // Peach
        "#99CCFF"  // Light Blue
    };

    qDebug() << "=== Top Bar Grid Debug ===";
    qDebug() << "Top bar frame:" << topBarFrame->metaObject()->className();

    const QRect topBarGeom = topBarFrame->geometry();
    qDebug() << QString("Geometry: QRect(%1,%2 %3x%4)")
        .arg(topBarGeom.x()).arg(topBarGeom.y()).arg(topBarGeom.width()).arg(topBarGeom.height());

    // Lambda to apply debug style recursively
    auto applyDebugStyle = [&colors](QWidget* widget, int& colorIdx) {
        const QString& borderColor = colors[colorIdx % colors.size()];

        // Create debug border style with !important to override existing styles
        QString debugStyle = QString(
            "%1 { "
            "  border: 3px solid %2 !important; "
            "  background-color: rgba(255, 255, 255, 0.05) !important; "
            "}"
        ).arg(widget->metaObject()->className(), borderColor);

        widget->setStyleSheet(debugStyle);

        const QRect geom = widget->geometry();
        qDebug() << QString("  ✓ %1 at QRect(%2,%3 %4x%5)")
            .arg(widget->metaObject()->className())
            .arg(geom.x()).arg(geom.y()).arg(geom.width()).arg(geom.height());

        colorIdx++;
    };

    // Get the layout
    auto* layout = qobject_cast<QGridLayout*>(topBarFrame->layout());
    if (layout) {
        qDebug() << "Grid layout:" << layout->rowCount() << "rows x" << layout->columnCount() << "cols";
    }

    qDebug() << "\n--- Applying Debug Borders ---";
    int colorIdx = 0;

    // Apply to direct grid items first
    if (layout) {
        for (int row = 0; row < layout->rowCount(); ++row) {
            for (int col = 0; col < layout->columnCount(); ++col) {
                QLayoutItem* item = layout->itemAtPosition(row, col);
                if (item && item->widget()) {
                    QWidget* widget = item->widget();
                    applyDebugStyle(widget, colorIdx);
                }
            }
        }
    }

    // Also style key components directly to ensure visibility
    qDebug() << "\n--- Key Components ---";
    if (m_windowActionsBar) {
        applyDebugStyle(m_windowActionsBar, colorIdx);
    } else {
        qDebug() << "  ✗ WindowsActionsBar is null";
    }

    if (m_navigationBar) {
        applyDebugStyle(m_navigationBar, colorIdx);
    } else {
        qDebug() << "  ✗ NavigationBar is null";
    }

    if (m_tabBar) {
        applyDebugStyle(m_tabBar, colorIdx);
    } else {
        qDebug() << "  ✗ TabBar is null";
    }

    if (m_menuButtonBar) {
        applyDebugStyle(m_menuButtonBar, colorIdx);
    } else {
        qDebug() << "  ✗ MenuButtonBar is null";
    }

    // Also apply debug style to all direct children
    qDebug() << "\n--- All Direct Children ---";
    const QObjectList& children = topBarFrame->children();
    for (QObject* obj : children) {
        QWidget* childWidget = qobject_cast<QWidget*>(obj);
        if (childWidget) {
            // Skip if already styled (to avoid duplicate color assignment)
            bool alreadyStyled = (childWidget == m_windowActionsBar ||
                                 childWidget == m_navigationBar ||
                                 childWidget == m_tabBar ||
                                 childWidget == m_menuButtonBar);

            if (!alreadyStyled) {
                applyDebugStyle(childWidget, colorIdx);
            }
        }
    }

    qDebug() << "\n=== Debug overlay applied ===";
    qDebug() << "Press Ctrl+Shift+G again or restart app to clear";
}

void MainWindow::setCursorForRegion(Region r)
{
    switch (r) {
    case TopLeft:
    case BottomRight:
        setCursor(Qt::SizeFDiagCursor); break;
    case TopRight:
    case BottomLeft:
        setCursor(Qt::SizeBDiagCursor); break;
    case Left:
    case Right:
        setCursor(Qt::SizeHorCursor); break;
    case Top:
    case Bottom:
        setCursor(Qt::SizeVerCursor); break;
    default:
        unsetCursor(); break;
    }
}

MainWindow::Region MainWindow::hitTest(const QPoint& p) const
{
    const int x = p.x();
    const int y = p.y();
    const int w = width();
    const int h = height();
    const int bw = borderWidth; // resize grip size in px

    const bool left   = (x <= bw);
    const bool right  = (x >= w - bw - 1);
    const bool top    = (y <= bw);
    const bool bottom = (y >= h - bw - 1);

    if (top && left)     return TopLeft;
    if (top && right)    return TopRight;
    if (bottom && left)  return BottomLeft;
    if (bottom && right) return BottomRight;
    if (left)            return Left;
    if (right)           return Right;
    if (top)             return Top;
    if (bottom)          return Bottom;
    return None;
}

void MainWindow::applyWinVisualStyle()
{
#ifdef _WIN32
    if (!windowHandle()) return;
    HWND hwnd = reinterpret_cast<HWND>(windowHandle()->winId()); // Get a native window handle

    // Enable the dark title bar
    enableDarkTitleBar(hwnd, false);

    // Set rounded corners
    setRoundedCorners(hwnd, 2); // 2 = Round corners

    // Enable Mica effect
    enableMica(hwnd);
#endif
}

void MainWindow::ensureSnapFriendlyStyle() const
{

#ifdef _WIN32
    if (!windowHandle()) return;
    const auto hwnd = reinterpret_cast<HWND>(windowHandle()->winId());

    // Ensure styles that allow system resize & Snap flyout
    addStyle(hwnd, WS_THICKFRAME | WS_CAPTION | WS_MAXIMIZEBOX | WS_MINIMIZEBOX);
#endif
}

void MainWindow::rememberNormalGeometry()
{
    if (!isMaximized() && !isFullScreen())
    {
        lastNormalGeometry = geometry();
        haveNormalGeometry = lastNormalGeometry.isValid();

    }
}


bool MainWindow::eventFilter(QObject* obj, QEvent* event)
{
    // Check if the event is a resize event on the content widget
    if (m_floatingToggleButton && obj == m_floatingToggleButton->parentWidget()) {
        if (event->type() == QEvent::Resize) {
            updateFloatingToggleButtonVisibility();
        }
    }

    if (obj == topBarFrame)
    {
        if (event->type() == QEvent::MouseButtonPress)
        {
            const auto *e = dynamic_cast<QMouseEvent*>(event);
            if (e->button() == Qt::LeftButton)
            {
                // If the press is on the actions bar (or its children), do not drag
                const QPoint topLocal = topBarFrame->mapFromGlobal(e->globalPosition().toPoint());
                QWidget *child = topBarFrame->childAt(topLocal);
                for (QWidget *w = child; w && w != topBarFrame; w = w->parentWidget()) {
                    if (w == m_windowActionsBar || w == m_tabBar  || qobject_cast<QPushButton*>(w)) {
                        return QWidget::eventFilter(obj, event); // let the button handle it
                    }
                }

                // Otherwise, only start drag if not on a resize border
                if (hitTest(mapFromGlobal(e->globalPosition().toPoint())) == None) {
                    dragging = true;
                    dragStartGlobal = e->globalPosition().toPoint();
                    startGeom = geometry();
                    return true; // handled
                }
            }
        } else if (event->type() == QEvent::MouseMove) {
            auto *e = dynamic_cast<QMouseEvent*>(event);
            if (dragging) {
                const QPoint delta = e->globalPosition().toPoint() - dragStartGlobal;
                setGeometry(QRect(startGeom.topLeft() + delta, startGeom.size()));
                return true; // handled
            }
        } else if (event->type() == QEvent::MouseButtonRelease) {
            auto *e = dynamic_cast<QMouseEvent*>(event);
            if (e->button() == Qt::LeftButton) {
                dragging = false;
                return true; // handled
            }
        }
    }
    return QWidget::eventFilter(obj, event);
}


void MainWindow::mousePressEvent(QMouseEvent* e)
{

    if (e->button() != Qt::LeftButton)
        return;

    currentRegion = hitTest(e->pos());
    if (currentRegion != None) {
        resizing = true;
        dragStartGlobal = e->globalPosition().toPoint();
        startGeom = geometry();
        e->accept();
        return;
    }
    QWidget::mousePressEvent(e);
}

void MainWindow::mouseMoveEvent(QMouseEvent* e)
{
    QWidget::mouseMoveEvent(e);
    const QPoint pos = e->pos();
    if (resizing) {
        const QPoint delta = e->globalPosition().toPoint() - dragStartGlobal;
        QRect g = startGeom;

        const int minW = minimumWidth() > 0 ? minimumWidth() : 100;
        const int minH = minimumHeight() > 0 ? minimumHeight() : 80;

        if (currentRegion & Left)   g.setLeft(g.left() + delta.x());
        if (currentRegion & Right)  g.setRight(g.right() + delta.x());
        if (currentRegion & Top)    g.setTop(g.top() + delta.y());
        if (currentRegion & Bottom) g.setBottom(g.bottom() + delta.y());

        if (g.width() < minW) {
            if (currentRegion & Left)   g.setLeft(g.right() - minW);
            else                        g.setRight(g.left() + minW);
        }
        if (g.height() < minH) {
            if (currentRegion & Top)    g.setTop(g.bottom() - minH);
            else                        g.setBottom(g.top() + minH);
        }
        setGeometry(g);
        e->accept();
        return;
    }
    if (dragging) {
        const QPoint delta = e->globalPosition().toPoint() - dragStartGlobal;
        setGeometry(QRect(startGeom.topLeft() + delta, startGeom.size()));
        e->accept();
        return;
    }

    if (!resizing && !dragging) {
        setCursorForRegion(hitTest(e->pos()));
    }
    // After any geometry change that you perform manually, update the saved normal geom
    if (!isMaximized() && (resizing || dragging)) {
        rememberNormalGeometry();
    }

    // Not resizing: update cursor based on proximity to edges
    setCursorForRegion(hitTest(pos));


}

void MainWindow::mouseReleaseEvent(QMouseEvent* e)
{
    if (e->button() == Qt::LeftButton) {
        resizing = false;
        currentRegion = None;
        rememberNormalGeometry();
        e->accept();
        return;
    }
    QWidget::mouseReleaseEvent(e);
}

void MainWindow::leaveEvent(QEvent* e)
{
    if (!resizing) unsetCursor();
    QWidget::leaveEvent(e);
}

void MainWindow::keyPressEvent(QKeyEvent* e)
{

    if (e->key() == Qt::Key_G &&
        (e->modifiers() & Qt::ControlModifier) &&
        (e->modifiers() & Qt::ShiftModifier))
    {
        debugTopBarGrid();
        e->accept();
        return;
    }

    QWidget::keyPressEvent(e);
}

void MainWindow::showEvent(QShowEvent* e)
{
    QWidget::showEvent(e);
#ifdef _WIN32
    // Defer to ensure the is fully created before applying styles
    QTimer::singleShot(0, this, [this]() {
        ensureSnapFriendlyStyle();
        applyWinVisualStyle();
        rememberNormalGeometry();
    });
#endif
}

void MainWindow::changeEvent(QEvent* e)
{
    if (e->type() == QEvent::WindowStateChange)
    {
        const bool maximised = isMaximized();
        const bool fullScreen = isFullScreen();

        if (m_windowActionsBar) {
            m_windowActionsBar->setMaximised(maximised || fullScreen);
            m_windowActionsBar->setVisible(!fullScreen);
        }

        if (!maximised && !fullScreen)
            rememberNormalGeometry();
    }
    else if (e->type() == QEvent::ApplicationPaletteChange ||
             e->type() == QEvent::PaletteChange)
    {
        updateWindowTheme();
    }
    QWidget::changeEvent(e);
}

#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
bool MainWindow::nativeEvent(const QByteArray &eventType, void *message, qintptr *result)
#else
bool MainWindow::nativeEvent(const QByteArray &eventType, void *message, long *result)
#endif
{
#ifdef _WIN32
    if (eventType == "windows_generic_MSG" || eventType == "windows_dispatcher_MSG")
    {
        MSG *msg = static_cast<MSG*>(message);
        if (msg->message == WM_NCHITTEST)
        {
            // setting resizing borders based on existing borderWidth
            RECT winRECT{};
            GetWindowRect(reinterpret_cast<HWND>(windowHandle()->winId()), &winRECT);
            const LONG x = GET_X_LPARAM(msg->lParam);
            const LONG y = GET_Y_LPARAM(msg->lParam);
            const int bw = borderWidth; // Same as the client Logic

            // Determine the hit region
            const bool left   = x <  winRECT.left   + bw;
            const bool right  = x >= winRECT.right  - bw;
            const bool top    = y <  winRECT.top    + bw;
            const bool bottom = y >= winRECT.bottom - bw;

            if (top && left)    { *result = HTTOPLEFT;    return true; }
            if (top && right)   { *result = HTTOPRIGHT;   return true; }
            if (bottom && left) { *result = HTBOTTOMLEFT; return true; }
            if (bottom && right){ *result = HTBOTTOMRIGHT;return true; }
            if (left)           { *result = HTLEFT;       return true; }
            if (right)          { *result = HTRIGHT;      return true; }
            if (top)            { *result = HTTOP;        return true; }
            if (bottom)         { *result = HTBOTTOM;     return true; }


            // Title area for dragging and snapping, excluding the actions bar region
            if (topBarFrame) {
                const QPoint wndLocal = mapFromGlobal(QPoint(x, y)); // coordinates in MainWindow
                if (topBarFrame->geometry().contains(wndLocal)) {
                    // Map the point into topBarFrame space to test against actions bar geometry
                    const QPoint wndLocal = mapFromGlobal(QPoint(GET_X_LPARAM(msg->lParam),
                                                             GET_Y_LPARAM(msg->lParam)));

                    if (topBarFrame->geometry().contains(wndLocal)) {
                        const QPoint topLocal = topBarFrame->mapFrom(this, wndLocal);

                        const bool overActionsBar =
                            (m_windowActionsBar &&
                             m_windowActionsBar->geometry().contains(topLocal));

                        const bool overMenuButtonBar =
                            (m_menuButtonBar &&
                             m_menuButtonBar->geometry().contains(topLocal));

                        const bool overNavigationBar =
                            (m_navigationBar &&
                             m_navigationBar->geometry().contains(topLocal));

                        const bool overTabBar =
                            (m_tabBar &&
                            m_tabBar->geometry().contains(topLocal));

                        if (!overActionsBar && !overMenuButtonBar && !overNavigationBar && !overTabBar) {
                            *result = HTCAPTION;   // drag & Snap flyout allowed
                            return true;
                        }
                    }
                }
            }

        }

    }
#endif
    return QWidget::nativeEvent(eventType, message, result);
}



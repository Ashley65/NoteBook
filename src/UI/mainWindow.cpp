//
// Created by DevAccount on 27/11/2025.
//

#include <QEvent>
#include <QLabel>
#include <QMouseEvent>
#include <QPushButton>
#include <QShowEvent>
#include <QTimer>
#include <QWidget>
#include <UI/mainWIndow.h>
#include <UI/components/SIde_Bar/sideBar.h>

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


    // 2. Ensure the stylesheet allows transparency
    setStyleSheet("QMainWindow { background: transparent; }");

    // ============================================================
    // 1. MAIN CONTAINER (.container)
    // ============================================================

    auto *mainLayout = new QGridLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // CSS: grid-template-columns: 0.2fr 1.8fr 1fr;
    // Map to integer stretches (×10): 2, 18, 10
    // Initial stretches will be updated by resizeEvent/setupSideBar
    mainLayout->setColumnStretch(0, 0); 
    mainLayout->setColumnStretch(1, 14);
    mainLayout->setColumnStretch(2, 10);

    // CSS: grid-template-rows: 0.2fr 1.8fr 1fr;
    mainLayout->setRowStretch(0, 2);
    mainLayout->setRowStretch(1, 18);
    mainLayout->setRowStretch(2, 10);

    // Dark background for the main container

    // ============================================================
    // 2. TOP MENUBAR (.top_menubar)
    // ============================================================
    // CSS: grid-area: 1 / 1 / 2 / 6; (Row 0, Col 0, Span 1, Span 3)

    topBarFrame = new QFrame();
    mainLayout->addWidget(topBarFrame, 0, 0, 1, 3);

    topBarFrame->installEventFilter(this);
    setMouseTracking(true);
    // Internal Grid for Top Bar
    auto *topLayout = new QGridLayout(topBarFrame);
    topLayout->setContentsMargins(10, 8, 10, 8);
    topLayout->setSpacing(0);

    // CSS: grid-template-columns: 1fr 1fr 1fr;
    topLayout->setColumnStretch(0, 1);
    topLayout->setColumnStretch(1, 8);  // middle expands
    topLayout->setColumnStretch(2, 0);  // right column stays tight

    // CSS: grid-template-rows: 1fr 0.5fr 2fr; -> (2 : 1 : 4)
    topLayout->setRowStretch(0, 2);
    topLayout->setRowStretch(1, 1);
    topLayout->setRowStretch(2, 4);


    // --- Application_info ---
    // Spans Row 0, Cols 0-1
    auto* infoBar = new InfoBar(topBarFrame);
    infoBar->setAppName("ChronoTasks");
    infoBar->setCurrentScreenLabel("TEST Home");

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

    auto* infoContainer = new QFrame(topBarFrame);
    auto* infoLay = new QGridLayout(infoContainer);
    infoLay->setContentsMargins(0, 0, 0, 0);
    infoLay->setSpacing(0);
    infoLay->addWidget(infoBar, 0, 0);

    topLayout->addWidget(infoContainer, 0, 0, 1, 1);
    // --- Menu_action_bar ---
    // Spans Row 2, Cols 0-2
    m_navigationBar = new NavigationBar(topBarFrame);
    topLayout->addWidget(m_navigationBar, 2, 0, 1, 2);
    connect(m_navigationBar, &NavigationBar::backClicked, this, &MainWindow::goBack);
    connect(m_navigationBar, &NavigationBar::forwardClicked, this, &MainWindow::goForward);
    connect(m_navigationBar, &NavigationBar::refreshClicked, this, &MainWindow::refreshPage);


    // -- Window_action_bar ---
    // Spans Rows 0-1, Col 2
    // using the window actions bar setup function
    setupWindowActionsBar();
    // Ensure the actions bar does not expand: prefer fixed/minimum width and align right
    if (m_windowActionsBar) {
        m_windowActionsBar->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
        m_windowActionsBar->setMaximumWidth(220); // tweak as needed for your buttons
    }
    topLayout->addWidget(m_windowActionsBar, 0, 2, 2, 1, Qt::AlignRight | Qt::AlignTop);

    // -- Menu_dropdown_bar ---
    auto* menuBar = new MenuButtonBar(topBarFrame);
    m_menuButtonBar = menuBar;
    topLayout->addWidget(m_menuButtonBar, 2, 2, 1, 1, Qt::AlignRight | Qt::AlignVCenter);



    // ============================================================
    // 3. SIDE BAR (.Side_bar)
    // ============================================================
    // CSS: grid-area: 2 / 1 / 4 / 2; (Row 1, Col 0, Span 2, Span 1)


    sideBarFrame = new QFrame();
    mainLayout->addWidget(sideBarFrame, 1, 0, 2, 1);

    // now this will see a non\-null sideBarFrame
    setupSideBar();




    // ============================================================
    // 4. CONTENT WINDOW (.Content_window)
    // ============================================================
    // CSS: grid-area: 2 / 2 / 4 / 6; (Row 1, Col 1, Span 2, Span 4)
    QFrame* content = createWidget("content", "#ecf0f1", "color: #333;", true, false);
    mainLayout->addWidget(content, 1, 1, 2, 2);
    content->installEventFilter(this); // Watch for resize events on content
    // Adding the floating toggle button for sidebar linked to content area
    m_floatingToggleButton = new QPushButton(content);
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
    if (isMaximized()) {
        // Restore
        showNormal();
        if (haveNormalGeometry) {
            // Defer setting the geometry until after the normal state is applied
            QTimer::singleShot(0, this, [this]{ setGeometry(lastNormalGeometry); });
        }
        m_windowActionsBar->setMaximised(false);
    } else {
        // Save current normal geometry before maximizing
        rememberNormalGeometry();
        showMaximized();
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
        if (m == SideBar::Mode::Default) {
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
                    if (w == m_windowActionsBar || qobject_cast<QPushButton*>(w)) {
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
        if (m_windowActionsBar)
            m_windowActionsBar->setMaximised(maximised);
        if (!maximised)
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

                        if (!overActionsBar && !overMenuButtonBar && !overNavigationBar) {
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



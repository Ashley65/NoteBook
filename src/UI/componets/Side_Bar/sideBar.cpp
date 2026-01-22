//
// Created by DevAccount on 18/12/2025.
//
#include <UI/components/SIde_Bar/sideBar.h>
#include <UI/components/SIde_Bar/componets/CoreNavigationSection.h>
#include <UI/components/SIde_Bar/componets/PrimaryActionSection.h>
#include <UI/components/SIde_Bar/componets/ProjectsSection.h>
#include <UI/components/SIde_Bar/componets/FiltersSection.h>
#include <UI/components/SIde_Bar/componets/FooterSection.h>

#include <QFrame>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QLabel>
#include <QDebug>

SideBar::SideBar(QWidget* parent)
    : QFrame(parent)
{
    setObjectName("SideBar");
    setFrameShape(QFrame::NoFrame);
    setStyleSheet(R"(
        QFrame#SideBar {
            background-color: #0F111A;
            border-right: 1px solid #2A2D3D; /* Subtle dark border */
            border-radius: 8px;
        }
        /* Ensure the scroll area doesn't draw a white background over our dark theme */
        QScrollArea { background: transparent; border: none; }
        QWidget#ScrollContainer { background: transparent; }
    )");

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding); // allow horizontal growth

    auto *root = new QVBoxLayout(this);
    // Reduced margins slightly to maximise space
    root->setContentsMargins(5, 15, 5, 15);
    root->setSpacing(10);

    // 1. Primary Action (Personal Workspace) - moved to top per image
    m_primary = new WorkspaceContextSection(this);
    root->addWidget(m_primary);

    // 2. Core Navigation (Inbox, Today...)
    m_core = new CoreNavigationSection(this);
    root->addWidget(m_core);

    // 3. Scrollable Area (Projects + Filters)
    auto *scrollContainer = new QWidget(this);
    scrollContainer->setObjectName("ScrollContainer"); // Named for CSS targeting
    auto *scrollLayout = new QVBoxLayout(scrollContainer);
    scrollLayout->setContentsMargins(0, 0, 0, 0);
    scrollLayout->setSpacing(10);

    // Note: If ProjectsSection inherits BaseNavTree, it IS a widget.
    // If it wraps one, pass scrollContainer as parent.
    m_projects = new ProjectsSection(scrollContainer);
    scrollLayout->addWidget(m_projects);

    m_filters = new FiltersSection(scrollContainer);
    scrollLayout->addWidget(m_filters);

    scrollLayout->addStretch(1); // Push everything up

    auto *scroll = new QScrollArea(this);
    scroll->setWidget(scrollContainer);
    scroll->setWidgetResizable(true);
    // These calls are critical for the transparent look
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->viewport()->setAutoFillBackground(false); // Important for transparency

    root->addWidget(scroll, 1); // This takes the remaining space

    // 4. Footer
    m_footer = new FooterSection(this);
    root->addWidget(m_footer);

    // --- NEW LOGIC START ---
    // Handle the button click from the footer
    connect(m_footer, &FooterSection::modeCycleRequested, this, [this]() {
        // Cycle: Default -> Compact -> Hidden -> Default
        if (m_mode == Mode::Default) {
            setMode(Mode::Compact);
        }
        else if (m_mode == Mode::Compact) {
            setMode(Mode::Hidden);
        }
        else {
            // Note: Use an external button (like in top bar) to trigger this
            // because the footer is invisible in Hidden mode!
            setMode(Mode::Default);
        }
    });

    applyMode();
}


void SideBar::setMode(Mode m) {
    if (m_mode == m) return;
    m_mode = m;
    applyMode();
}

void SideBar::applyMode() {
    const bool hidden = (m_mode == Mode::Hidden);
    setVisible(!hidden);

    const bool compact = (m_mode == Mode::Compact);

    // Propagate mode to child components
    if (m_primary) {
        m_primary->setProperty("compact", compact);
        m_primary->setVisible(!hidden);
    }
    if (m_core) { m_core->setProperty("compact", compact);    m_core->setVisible(!hidden); }

    // Projects and Filters
    if (!hidden) {
        if (m_projects) {
            m_projects->setVisible(true);
            m_projects->setCompact(compact); // <--- Direct Call
        }
        if (m_filters) {
            m_filters->setVisible(true);
            m_filters->setCompact(compact);  // <--- Direct Call
        }
    }

    if (m_footer) {
        m_footer->setMode(m_mode);
        m_footer->setVisible(!hidden);
    }
    // Adjust margins based on mode
    if (auto* root = qobject_cast<QVBoxLayout*>(layout())) {
        if (compact) {
            root->setContentsMargins(2, 10, 2, 10);
            root->setSpacing(5);

        } else {
            root->setContentsMargins(10, 10, 10, 10);
            root->setSpacing(10);

        }
    }

    if (hidden) {
        setMinimumWidth(0);
        setMaximumWidth(0);
        // Debug line
        qDebug() << "Sidebar hidden";
    } else if (compact) {
        // Compact: fixed narrow width
        setFixedWidth(48);
        setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
        qDebug() << "Sidebar compact";
    } else {
        // Default: allow expansion with the layout instead of clamping with a small maximum
        setMinimumWidth(200); // sensible minimum
        setMaximumWidth(QWIDGETSIZE_MAX); // allow the layout to give it space
        setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
        qDebug() << "Sidebar default";
    }


}


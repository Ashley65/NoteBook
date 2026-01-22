#include <UI/components/Top_Menubar/windowsAction.h>

#include <QPushButton>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QEvent>


WindowsActionsBar::WindowsActionsBar(QWidget* parent, Style style)
    : QWidget(parent), m_style(style)
{
    initUi();
    connectSignals();
}

void WindowsActionsBar::setStyle(Style style)
{
    if (m_style == style) return;
    m_style = style;
    
    // Clear layout and re-init
    QLayoutItem* item;
    while ((item = m_layout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }
    initUi();
}

void WindowsActionsBar::setButtonStyleSheet(const QString& css)
{
    if (m_minimiseBtn) m_minimiseBtn->setStyleSheet(css);
    if (m_maximiseBtn) m_maximiseBtn->setStyleSheet(css);
    if (m_closeBtn)    m_closeBtn->setStyleSheet(
        css + "QPushButton:hover { background: #ff4444; color: white; }"
    );
}

void WindowsActionsBar::initUi()
{
    // Layout
    if (!m_layout) {
        m_layout = new QHBoxLayout(this);
    }
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(m_style == Style::MacOS ? 8 : 0);

    // Buttons
    m_minimiseBtn = new QPushButton(this);
    m_minimiseBtn->setObjectName("winActionMinimise");
    m_minimiseBtn->setFlat(true);

    m_maximiseBtn = new QPushButton(this);
    m_maximiseBtn->setObjectName("winActionMaximise");
    m_maximiseBtn->setFlat(true);

    m_closeBtn = new QPushButton(this);
    m_closeBtn->setObjectName("winActionClose");
    m_closeBtn->setFlat(true);

    if (m_style == Style::MacOS) {
        // macOS order: close, minimise, maximise
        m_layout->addWidget(m_closeBtn);
        m_layout->addWidget(m_minimiseBtn);
        m_layout->addWidget(m_maximiseBtn);

        m_closeBtn->setToolTip("Close");
        m_minimiseBtn->setToolTip("Minimise");
        m_maximiseBtn->setToolTip("Maximise");

        const QString baseStyle =
            "QPushButton {"
            "  border-radius: 6px;"
            "  border: none;"
            "  width: 12px;"
            "  height: 12px;"
            "  min-width: 12px;"
            "  min-height: 12px;"
            "}"
            "QPushButton:hover { opacity: 0.8; }";

        m_closeBtn->setStyleSheet(baseStyle + "QPushButton { background-color: #ff5f56; border: 1px solid #e0443e; }");
        m_minimiseBtn->setStyleSheet(baseStyle + "QPushButton { background-color: #ffbd2e; border: 1px solid #dea123; }");
        m_maximiseBtn->setStyleSheet(baseStyle + "QPushButton { background-color: #27c93f; border: 1px solid #1aab29; }");

        const int btnSize = 12;
        m_closeBtn->setFixedSize(btnSize, btnSize);
        m_minimiseBtn->setFixedSize(btnSize, btnSize);
        m_maximiseBtn->setFixedSize(btnSize, btnSize);

        setFixedHeight(btnSize + 8);
    } else {
        // Windows order: minimise, maximise, close
        m_minimiseBtn->setText("-");
        m_minimiseBtn->setToolTip("Minimise");

        m_maximiseBtn->setText(m_isMaximised ? "❐" : "□");
        m_maximiseBtn->setToolTip(m_isMaximised ? "Restore" : "Maximise");

        m_closeBtn->setText("×");
        m_closeBtn->setToolTip("Close");

        m_layout->addWidget(m_minimiseBtn);
        m_layout->addWidget(m_maximiseBtn);
        m_layout->addWidget(m_closeBtn);

        QFont smallFont = font();
        smallFont.setPointSizeF(15.0);
        m_minimiseBtn->setFont(smallFont);
        m_maximiseBtn->setFont(smallFont);
        m_closeBtn->setFont(smallFont);

        QString compactCss =
        "QPushButton{"
        "  padding: 0 6px;"
        "  min-width: 0;"
        "  min-height: 0;"
        "  border: none;"
        "}"
        "QPushButton:hover {"
        "    background: #a8c0ff;"
        "}";

        m_minimiseBtn->setStyleSheet(compactCss);
        m_maximiseBtn->setStyleSheet(compactCss);
        m_closeBtn->setStyleSheet(compactCss + "QPushButton:hover { background: #ff4444; color: white; }");

        const int btnW = 25;
        const int btnH = 20;
        m_minimiseBtn->setFixedSize(btnW, btnH);
        m_maximiseBtn->setFixedSize(btnW, btnH);
        m_closeBtn->setFixedSize(btnW, btnH);

        setFixedHeight(btnH);
    }
}


void WindowsActionsBar::connectSignals()
{
    // Relay button clicks as high-level signals
    connect(m_minimiseBtn, &QPushButton::clicked, this, [this]() {
        emit minimiseRequested();
    });

    connect(m_maximiseBtn, &QPushButton::clicked, this, [this]() {
        emit maximiseRestoreRequested();
    });

    connect(m_closeBtn, &QPushButton::clicked, this, [this]() {
        emit closeRequested();
    });
}

void WindowsActionsBar::setMaximised(bool maximised)
{
    if (m_isMaximised == maximised)
        return;

    m_isMaximised = maximised;
    if (m_style == Style::Windows) {
        if (m_isMaximised) {
            // When maximised, show a RESTORE glyph and tooltip
            m_maximiseBtn->setText("❐");
            m_maximiseBtn->setToolTip("Restore");
        } else {
            // When normal, show a MAXIMISE glyph and tooltip
            m_maximiseBtn->setText("□");
            m_maximiseBtn->setToolTip("Maximise");
        }
    }
}

void WindowsActionsBar::mousePressEvent(QMouseEvent* event)
{
    // Treat double-click on the bar as maximise/restore toggle
    if (event && event->type() == QEvent::MouseButtonDblClick && event->button() == Qt::LeftButton) {
        emit maximiseRestoreRequested();
        event->accept();
        return;
    }
    QWidget::mousePressEvent(event);
}

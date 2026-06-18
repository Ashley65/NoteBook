//
// Created by DevAccount on 04/12/2025.
//

#include "UI/components/Top_Menubar/NavigationBar.h"

#include <QFrame>
#include <QPushButton>
#include <QHBoxLayout>
#include <QSizePolicy>
#include <QStyle>
#include <QIcon>
#include <QSize>
#include <QFile>
#include <QDebug>
#include <QDir>
#include <QLabel>


NavigationBar::NavigationBar(QWidget* parent) : QWidget(parent)
{
    initUi();
}

void NavigationBar::setButtonStyleSheet(const QString& css)
{
    if (m_backBtn)    m_backBtn->setStyleSheet(css);
    if (m_forwardBtn) m_forwardBtn->setStyleSheet(css);
    if (m_refreshBtn) m_refreshBtn->setStyleSheet(css);
}

void NavigationBar::updateIcons(bool darkMode)
{
    auto loadIcon = [darkMode](const QString &nameLight, const QString &nameDark) -> QIcon
    {
        const QString &name = darkMode ? nameDark : nameLight;
        QIcon ic = QIcon::fromTheme(name);
        if (!ic.isNull())               // if theme icon exists use it
            return ic;
        // otherwise load from resources
        return QIcon(QStringLiteral(":/icons/") + name + QStringLiteral(".svg"));
    };

    if (m_backBtn)
    {
        m_backBtn->setIcon(loadIcon(
            QStringLiteral("arrow_back_24dp_000000_FILL0_wght400_GRAD0_opsz24"),
            QStringLiteral("arrow_back_24dp_E3E3E3_FILL0_wght400_GRAD0_opsz24")
        ));
    }
    if (m_forwardBtn)
    {
        m_forwardBtn->setIcon(loadIcon(
            QStringLiteral("arrow_forward_24dp_000000_FILL0_wght400_GRAD0_opsz24"),
            QStringLiteral("arrow_forward_24dp_E3E3E3_FILL0_wght400_GRAD0_opsz24")
        ));
    }
    if (m_refreshBtn)
    {
        m_refreshBtn->setIcon(loadIcon(
            QStringLiteral("refresh_24dp_000000_FILL0_wght400_GRAD0_opsz24"),
            QStringLiteral("refresh_24dp_E3E3E3_FILL0_wght400_GRAD0_opsz24")
        ));
    }
}

void NavigationBar::initUi()
{
    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(4);

    constexpr int buttonSize = 28;

    constexpr QSize iconSize(16, 16);




    auto loadIcon = [](const QString &name) -> QIcon
    {
        QIcon ic = QIcon::fromTheme(name);
        if (!ic.isNull())               // if theme icon exists use it
            return ic;
        // otherwise load from resources
        return QIcon(QStringLiteral(":/icons/") + name + QStringLiteral(".svg"));
    };
    // Back button
    m_backBtn = new QPushButton(this);
    m_backBtn->setToolTip("Back");
    m_backBtn->setAccessibleName("Back");
    m_backBtn->setFlat(true);
    m_backBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_backBtn->setFixedSize(buttonSize, buttonSize);
    m_backBtn->setIcon(loadIcon(QStringLiteral("arrow_back_24dp_E3E3E3_FILL0_wght400_GRAD0_opsz24")));
    m_backBtn->setIconSize(iconSize);
    m_layout->addWidget(m_backBtn);

    // Forward
    m_forwardBtn = new QPushButton(this);
    m_forwardBtn->setToolTip("Forward");
    m_forwardBtn->setAccessibleName("Forward");
    m_forwardBtn->setFlat(true);
    m_forwardBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_forwardBtn->setFixedSize(buttonSize, buttonSize);
    m_forwardBtn->setIcon(loadIcon(QStringLiteral("arrow_forward_24dp_E3E3E3_FILL0_wght400_GRAD0_opsz24")));
    m_forwardBtn->setIconSize(iconSize);
    m_layout->addWidget(m_forwardBtn);

    QLabel *sep = new QLabel("|", this);
    sep->setFixedSize(4, buttonSize);
    m_layout->addWidget(sep);

    // Refresh button
    m_refreshBtn = new QPushButton(this);
    m_refreshBtn->setToolTip("Refresh");
    m_refreshBtn->setAccessibleName("Refresh");
    m_refreshBtn->setFlat(true);
    m_refreshBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_refreshBtn->setFixedSize(buttonSize, buttonSize);
    m_refreshBtn->setIcon(loadIcon(QStringLiteral("refresh_24dp_E3E3E3_FILL0_wght400_GRAD0_opsz24")));
    m_refreshBtn->setIconSize(iconSize);
    m_layout->addWidget(m_refreshBtn);

    // Add stretch to push buttons to the left
    m_layout->addStretch();

    const QString css =
        "QPushButton {"
        "  padding: 0px;"
        "  border: none;"
        "  border-radius: 4px;"
        "}"
        "QPushButton:hover {"
        "    background: #a8c0ff;"
        "}"
        "QPushButton:pressed {"
        "  background-color: rgba(255, 255, 255, 0.32);"
        "}";
    const QString sepCSS =
        "QLabel{"
        "  color: #888;"
        "}";




    m_backBtn->setStyleSheet(css);
    m_forwardBtn->setStyleSheet(css);
    m_refreshBtn->setStyleSheet(css);
    sep->setStyleSheet(sepCSS);

    // Direct Signal Connections
    connect(m_backBtn, &QPushButton::clicked, this, &NavigationBar::backClicked);
    connect(m_forwardBtn, &QPushButton::clicked, this, &NavigationBar::forwardClicked);
    connect(m_refreshBtn, &QPushButton::clicked, this, &NavigationBar::refreshClicked);
}


void NavigationBar::setBackEnabled(bool enabled)
{
    if (m_backBtn) m_backBtn->setEnabled(enabled);
}

void NavigationBar::setForwardEnabled(bool enabled)
{
    if (m_forwardBtn) m_forwardBtn->setEnabled(enabled);
}

void NavigationBar::setRefreshEnabled(bool enabled)
{
    if (m_refreshBtn) m_refreshBtn->setEnabled(enabled);
}

void NavigationBar::onBackClicked()
{
    emit backClicked();
}

void NavigationBar::onForwardClicked()
{
    emit forwardClicked();
}

void NavigationBar::onRefreshClicked()
{
    emit refreshClicked();
}
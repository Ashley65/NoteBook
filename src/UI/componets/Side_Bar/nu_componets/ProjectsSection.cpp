//
// Created by DevAccount on 15/08/2026.
//

#include <UI/components/SIde_Bar/nu_componets/ProjectSection.h>
#include <helpers/IconHelper.h>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QToolButton>
#include <QLabel>
#include <QVariantMap>

nu_ProjectsSection::nu_ProjectsSection(QWidget *parent)
    : QWidget(parent)
{
    projectLayout = new QVBoxLayout(this);
    projectLayout->setContentsMargins(0, 0, 0, 0);
    projectLayout->setSpacing(2);

    projectHLayout = new QHBoxLayout();
    projectHLayout->setContentsMargins(12, 6, 10, 4);
    projectHLayout->setSpacing(4);

    projectTitle = new QLabel("Projects :", this);
    projectTitle->setObjectName("ProjectTitle");
    projectTitle->setStyleSheet("color: #A0A0A0; font-size: 13px; font-weight: 500; padding: 0px; margin: 0px; background: transparent;");

    projectAddButton = new QToolButton(this);
    projectAddButton->setObjectName("ProjectAddButton");
    projectAddButton->setText("+");
    projectAddButton->setToolTip(tr("Create New Project"));
    projectAddButton->setCursor(Qt::PointingHandCursor);
    projectAddButton->setStyleSheet(R"(
        QToolButton#ProjectAddButton {
            border: none;
            background: transparent;
            color: #8C92A4;
            font-size: 16px;
            font-weight: bold;
            border-radius: 4px;
            padding: 0px 4px;
            min-width: 20px;
            min-height: 20px;
        }
        QToolButton#ProjectAddButton:hover {
            background-color: rgba(255, 255, 255, 0.1);
            color: #FFFFFF;
        }
        QToolButton#ProjectAddButton:pressed {
            background-color: rgba(255, 255, 255, 0.2);
        }
    )");

    connect(projectAddButton, &QToolButton::clicked, this, &nu_ProjectsSection::projectCreateRequested);

    projectHLayout->addWidget(projectTitle);
    projectHLayout->addStretch(1);
    projectHLayout->addWidget(projectAddButton);
    projectLayout->addLayout(projectHLayout);

    treeList = new nu_BaseNavTree(this);
    treeList->setObjectName("ProjectTree");
    treeList->setHeaderHidden(true);
    treeList->setRootIsDecorated(false);
    treeList->setIndentation(0);
    treeList->setAnimated(true);
    treeList->setFocusPolicy(Qt::NoFocus);
    treeList->setIconSize(QSize(20, 20));
    treeList->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    treeList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    treeList->setStyleSheet(R"(
        QTreeWidget#ProjectTree {
            background: transparent;
            border: none;
            outline: none;
        }
        QTreeWidget#ProjectTree::item {
            padding: 6px 12px;
            margin: 1px 8px;
            border-radius: 6px;
            background: transparent;
            border: none;
            color: #A0A0A0;
            font-size: 13px;
            font-weight: 500;
        }
        QTreeWidget#ProjectTree::item:hover {
            background: rgba(255, 255, 255, 0.05);
            color: #FFFFFF;
        }
        QTreeWidget#ProjectTree::item:selected {
            background: rgba(255, 255, 255, 0.12);
            color: #ffffff;
        }
    )");

    projectLayout->addWidget(treeList);

    connect(treeList, &nu_BaseNavTree::navItemClicked, this, [this](const QString& routeId) {
        Q_UNUSED(routeId);
        if (auto* current = treeList->currentItem()) {
            const QUuid projectId = QUuid::fromString(current->data(0, Qt::UserRole).toString());
            if (!projectId.isNull()) {
                m_activeProjectId = projectId;
                emit projectSelected(projectId);
            }
        }
    });
}

void nu_ProjectsSection::setProjects(const QVariantList& projects)
{
    m_projects = projects;
    treeList->clear();

    for (const QVariant& pVar : projects) {
        QVariantMap p = pVar.toMap();
        const QString idStr = p.value("id").toString();
        const QString name = p.value("name").toString();
        const QString colorCode = p.value("colorCode", "#64B5F6").toString();
        const QString initials = name.isEmpty() ? "P" : name.left(2).toUpper();

        QIcon avatar = IconHelper::generateInitialsIcon(initials, QColor(colorCode), Qt::white, 24);

        auto* item = new QTreeWidgetItem(treeList);
        item->setData(0, Qt::UserRole, idStr);
        item->setData(0, Qt::UserRole + 1, name);
        item->setData(0, Qt::UserRole + 2, colorCode);
        item->setIcon(0, avatar);

        if (m_isCompact) {
            item->setText(0, "");
            item->setTextAlignment(0, Qt::AlignCenter);
            item->setToolTip(0, name);
        } else {
            item->setText(0, name);
            item->setTextAlignment(0, Qt::AlignLeft | Qt::AlignVCenter);
            item->setToolTip(0, "");
        }

        if (QUuid::fromString(idStr) == m_activeProjectId) {
            treeList->setCurrentItem(item);
        }
    }
}

void nu_ProjectsSection::setActiveProjectId(const QUuid& projectId)
{
    m_activeProjectId = projectId;
    const QString idStr = projectId.toString(QUuid::WithoutBraces);
    for (int i = 0; i < treeList->topLevelItemCount(); ++i) {
        auto* item = treeList->topLevelItem(i);
        if (item->data(0, Qt::UserRole).toString() == idStr) {
            treeList->setCurrentItem(item);
            break;
        }
    }
}

void nu_ProjectsSection::setCompact(bool compact)
{
    m_isCompact = compact;
    projectLayout->setContentsMargins(compact ? 8 : 0, 0, compact ? 8 : 0, 0);
    projectTitle->setVisible(!compact);
    projectAddButton->setVisible(!compact);
    treeList->setIndentation(0);
    treeList->setIconSize(compact ? QSize(24, 24) : QSize(20, 20));

    if (compact) {
        treeList->setStyleSheet(R"(
            QTreeWidget#ProjectTree {
                background: transparent;
                border: none;
                outline: none;
            }
            QTreeWidget#ProjectTree::item {
                padding: 6px 0px;
                margin: 2px 0px;
                border-radius: 8px;
                background: transparent;
                border: none;
                color: #A0A0A0;
            }
            QTreeWidget#ProjectTree::item:hover {
                background: rgba(255, 255, 255, 0.08);
                color: #FFFFFF;
            }
            QTreeWidget#ProjectTree::item:selected {
                background: rgba(255, 255, 255, 0.15);
                color: #ffffff;
            }
        )");
    } else {
        treeList->setStyleSheet(R"(
            QTreeWidget#ProjectTree {
                background: transparent;
                border: none;
                outline: none;
            }
            QTreeWidget#ProjectTree::item {
                padding: 6px 12px;
                margin: 1px 8px;
                border-radius: 6px;
                background: transparent;
                border: none;
                color: #A0A0A0;
                font-size: 13px;
                font-weight: 500;
            }
            QTreeWidget#ProjectTree::item:hover {
                background: rgba(255, 255, 255, 0.05);
                color: #FFFFFF;
            }
            QTreeWidget#ProjectTree::item:selected {
                background: rgba(255, 255, 255, 0.12);
                color: #ffffff;
            }
        )");
    }

    for (int i = 0; i < treeList->topLevelItemCount(); ++i) {
        auto* it = treeList->topLevelItem(i);
        if (compact) {
            it->setText(0, "");
            it->setTextAlignment(0, Qt::AlignCenter);
            it->setToolTip(0, it->data(0, Qt::UserRole + 1).toString());
        } else {
            it->setText(0, it->data(0, Qt::UserRole + 1).toString());
            it->setTextAlignment(0, Qt::AlignLeft | Qt::AlignVCenter);
            it->setToolTip(0, "");
        }
    }
}

void nu_ProjectsSection::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::DynamicPropertyChange)
    {
        auto* propEvent = dynamic_cast<QDynamicPropertyChangeEvent*>(event);
        if (propEvent && propEvent->propertyName() == "compact")
        {
            setCompact(property("compact").toBool());
        }
    }
    QWidget::changeEvent(event);
}

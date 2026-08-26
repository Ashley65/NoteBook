//
// Created by DevAccount on 28/07/2026.
//

#include <UI/components/SIde_Bar/nu_componets/CoreNavigationSection.h>
#include <QVBoxLayout>
#include <QListWidgetItem>
#include <QShortcut>
#include <QVariant>
#include <QDynamicPropertyChangeEvent>
#include <QLabel>

nu_CoreNavigationSection::nu_CoreNavigationSection(QWidget* parent)
    : QWidget(parent)
{
    auto* mainlayout = new QVBoxLayout(this);
    mainlayout->setContentsMargins(0, 4, 0, 0);
    mainlayout->setSpacing(0);

    navTitle = new QLabel(this);
    navTitle->setObjectName("NavTitle");
    navTitle->setText("Navigation");
    navTitle->setStyleSheet("color: #A0A0A0; font-size: 13px; font-weight: 500; padding: 8px 12px 4px 12px; background: transparent;");

    navTree = new nu_BaseNavTree(this);
    navTree->setObjectName("navTree");
    navTree->setSelectionMode(QAbstractItemView::SingleSelection);
    navTree->setRootIsDecorated(false);
    navTree->setIndentation(0);
    navTree->setIconSize(QSize(18, 18));
    navTree->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    navTree->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    auto add = [this](const QString& text, Item itemType, const QString& iconPath) -> QTreeWidgetItem* {
        auto* item = navTree->addNavItem(text, QString::number(static_cast<int>(itemType)), QIcon(iconPath));
        item->setData(0, Qt::UserRole, QVariant::fromValue(static_cast<int>(itemType)));
        item->setData(0, Qt::UserRole + 1, text);
        return item;
    };

    auto* dashboardItem = add("Dashboard", Item::Dashboard, ":/icons/dashboard.svg");
    add("Projects", Item::Projects, ":/icons/projects.svg");
    add("Task Board", Item::TaskBoard, ":/icons/taskboard.svg");
    add("Notes", Item::Notes, ":/icons/notes.svg");
    add("AI Assistant", Item::AI_Assistant, ":/icons/aiassistant.svg");

    mainlayout->addWidget(navTitle);
    mainlayout->addWidget(navTree);

    connect(navTree, &nu_BaseNavTree::navItemClicked, this, [this](const QString& routeId) {
        Q_UNUSED(routeId);
        if (auto* current = navTree->currentItem()) {
            int enumVal = current->data(0, Qt::UserRole).toInt();
            auto item = static_cast<Item>(enumVal);
            emit itemSelected(item);
            emit itemColorSelected(colorForItem(item));
            updateStyles();
        }
    });

    navTree->setCurrentItem(dashboardItem);
    updateStyles();
}

QString nu_CoreNavigationSection::colorForItem(Item item)
{
    switch (item) {
        case Item::Dashboard:    return "#6366F1"; // Indigo
        case Item::Projects:     return "#38BDF8"; // Sky Blue
        case Item::TaskBoard:    return "#34D399"; // Emerald Green
        case Item::Notes:        return "#C084FC"; // Purple
        case Item::AI_Assistant: return "#FBBF24"; // Amber Gold
        default:                 return "#6366F1";
    }
}

void nu_CoreNavigationSection::setActiveItem(Item item)
{
    for (int i = 0; i < navTree->topLevelItemCount(); ++i) {
        auto* treeItem = navTree->topLevelItem(i);
        if (treeItem->data(0, Qt::UserRole).toInt() == static_cast<int>(item)) {
            navTree->setCurrentItem(treeItem);
            emit itemColorSelected(colorForItem(item));
            updateStyles();
            break;
        }
    }
}

void nu_CoreNavigationSection::setCompact(bool compact)
{
    m_isCompact = compact;
    navTree->setIndentation(0);
    navTree->setIconSize(compact ? QSize(20, 20) : QSize(18, 18));

    auto* lay = qobject_cast<QVBoxLayout*>(layout());
    if (lay) {
        lay->setContentsMargins(compact ? 8 : 0, compact ? 6 : 4, compact ? 8 : 0, 0);
    }

    navTitle->setVisible(!compact);

    for (int i = 0; i < navTree->topLevelItemCount(); ++i) {
        auto* it = navTree->topLevelItem(i);
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

    updateStyles();
}

void nu_CoreNavigationSection::updateStyles()
{
    QString activeColor = "#6366F1";
    if (auto* curr = navTree->currentItem()) {
        int enumVal = curr->data(0, Qt::UserRole).toInt();
        activeColor = colorForItem(static_cast<Item>(enumVal));
    }

    if (m_isCompact) {
        navTree->setStyleSheet(QString(R"(
            QTreeWidget#navTree {
                background: transparent;
                border: none;
                outline: none;
            }
            QTreeWidget#navTree::item {
                padding: 8px 0px;
                margin: 2px 0px;
                border-radius: 8px;
                background: transparent;
                border: none;
                color: #A0A0A0;
            }
            QTreeWidget#navTree::item:hover {
                background: rgba(255, 255, 255, 0.08);
                color: #FFFFFF;
            }
            QTreeWidget#navTree::item:selected {
                background: %1;
                color: #ffffff;
            }
        )").arg(activeColor));
    } else {
        navTree->setStyleSheet(QString(R"(
            QTreeWidget#navTree {
                background: transparent;
                border: none;
                outline: none;
            }
            QTreeWidget#navTree::item {
                padding: 8px 12px;
                margin: 2px 8px;
                border-radius: 6px;
                background: transparent;
                border: none;
                color: #A0A0A0;
                font-size: 13px;
                font-weight: 500;
            }
            QTreeWidget#navTree::item:hover {
                background: rgba(255, 255, 255, 0.05);
                color: #FFFFFF;
            }
            QTreeWidget#navTree::item:selected {
                background: %1;
                color: #ffffff;
            }
        )").arg(activeColor));
    }
}

void nu_CoreNavigationSection::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::DynamicPropertyChange) {
        auto* propEvent = static_cast<QDynamicPropertyChangeEvent*>(event);
        if (propEvent->propertyName() == "compact") {
            setCompact(property("compact").toBool());
        }
    }
    QWidget::changeEvent(event);
}

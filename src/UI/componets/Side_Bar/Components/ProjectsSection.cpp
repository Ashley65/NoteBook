//
// Created by DevAccount on 23/12/2025.
//
#include <UI/components/SIde_Bar/componets/ProjectsSection.h>
#include <UI/helpers/IconHelper.h>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QRandomGenerator>


ProjectsSection::ProjectsSection(QWidget *parent) : QWidget(parent)
{
    auto* lay = new QVBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);

    treeList = new QTreeWidget(this);
    treeList->setObjectName("ProjectTree");
    treeList->setHeaderHidden(true);
    treeList->setIndentation(15);
    treeList->setAnimated(true);
    treeList->setFocusPolicy(Qt::NoFocus);

    // Scrollbar policies
    treeList->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    treeList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // EXACT SAME GLOW STYLE as CoreNavigation, adapted for QTreeWidget
    treeList->setStyleSheet(R"(
        QTreeWidget#ProjectTree {
            background: transparent;
            border: none;
            outline: none;
            color: #A0A0A0;
        }
        QTreeWidget::item {
            padding: 6px;
            border-radius: 4px;
            border-left: 3px solid transparent;
            margin-bottom: 1px;
        }
        QTreeWidget::item:hover {
            background-color: #1A1D2D;
            color: white;
        }
        QTreeWidget::item:selected {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 rgba(50, 150, 255, 40), stop:1 rgba(0, 0, 0, 0));
            border-left: 3px solid #64B5F6;
            color: white;
        }
        /* Branch Icons (arrows) - Optional: add your own icons here */
        QTreeView::branch:has-children:!has-siblings:closed,
        QTreeView::branch:closed:has-children:has-siblings { image: url(:/icons/chevron-right.svg); }
        QTreeView::branch:open:has-children:!has-siblings,
        QTreeView::branch:open:has-children:has-siblings { image: url(:/icons/chevron-down.svg); }
    )");

    // Define Root "Projects" Header (Non-selectable)
    auto* root = new QTreeWidgetItem(treeList);
    root->setText(0, "Projects");
    root->setFlags(Qt::ItemIsEnabled); // Disable selection for header
    root->setExpanded(true);
#ifdef Q_OS_WIN
    root->setFont(0, QFont("Segoe UI", 9, QFont::Bold)); // Make header distinct
#else
    root->setFont(0, QFont(".AppleSystemUIFont", 9, QFont::Bold)); // Make header distinct
#endif

    // Add Items
    addProject(root, "System Design", 2);
    addProject(root, "Fitness Plan");
    addProject(root, "Dissertation", 8);
    addProject(root, "Task Monitoring App");

    lay->addWidget(treeList);

    connect(treeList, &QTreeWidget::itemClicked, this, [this](QTreeWidgetItem* item, int col){
        // Only emit if it's a leaf node (actual project), not the header
        if(item->parent()) {
            emit projectSelected(item->text(0));
        }
    });
}

void ProjectsSection::setCompact(bool compact) {
    // 1. Flatten Tree
    treeList->setIndentation(compact ? 0 : 15);

    QTreeWidgetItem* root = treeList->topLevelItem(0);
    if (!root) return;

    if (compact) {
        // --- COMPACT MODE ---
        root->setText(0, "");
        root->setSizeHint(0, QSize(0, 0));

        for(int i = 0; i < root->childCount(); ++i) {
            QTreeWidgetItem* item = root->child(i);
            QString fullName = item->data(0, Qt::UserRole).toString();

            item->setText(0, "");
            item->setTextAlignment(0, Qt::AlignCenter);
            item->setToolTip(0, fullName);

            // FORCE Generate Initials (Overwrite the folder icon)
            QString initials = fullName.left(2).toUpper();
            // Generate a random color using HSV for more pleasant, saturated colors
            int hue = QRandomGenerator::global()->bounded(360);
            QColor randomColor = QColor::fromHsv(hue, 200, 200);
            item->setIcon(0, IconHelper::generateInitialsIcon(initials, randomColor));
        }
    } else {
        // --- DEFAULT MODE ---
        root->setText(0, "Projects");
        root->setSizeHint(0, QSize(0, 30));

        for(int i = 0; i < root->childCount(); ++i) {
            QTreeWidgetItem* item = root->child(i);
            QString fullName = item->data(0, Qt::UserRole).toString();

            item->setText(0, fullName);
            item->setTextAlignment(0, Qt::AlignLeft | Qt::AlignVCenter);
            item->setToolTip(0, "");

            // RESTORE the Folder Icon
            // Since we overwrote it in compact mode, we must put it back now.
            item->setIcon(0, QIcon(":/icons/folder_24dp.svg"));
        }
    }
}

void ProjectsSection::addProject(QTreeWidgetItem* parent, const QString& name, int count) {
    auto* item = new QTreeWidgetItem(parent);

    // 1. Setup Text
    QString text = name;
    if(count > 0) text += QString(" (%1)").arg(count);

    item->setText(0, text);
    item->setData(0, Qt::UserRole, text); // Store full text for restoring later

    // 2. ADD DEFAULT ICON (Critical for Compact Mode)
    item->setIcon(0, QIcon(":/icons/folder_24dp.svg"));

    item->setSizeHint(0, QSize(0, 32));
}

void ProjectsSection::changeEvent(QEvent *event) {
    if (event->type() == QEvent::DynamicPropertyChange) {
        auto* propEvent = dynamic_cast<QDynamicPropertyChangeEvent*>(event);
        if (propEvent->propertyName() == "compact") {
            bool compact = property("compact").toBool();

            // --- FIX 1: Toggle Indentation ---
            // 0 removes the "tree" look (flattening the list)
            // 15 restores the hierarchy for the expanded view
            treeList->setIndentation(compact ? 0 : 15);

            // --- FIX 2: Hide Root Header ---
            QTreeWidgetItem* root = treeList->topLevelItem(0);
            if (root) {
                root->setHidden(compact);

                // Update children items
                for(int i = 0; i < root->childCount(); ++i) {
                    QTreeWidgetItem* item = root->child(i);
                    if (compact) {
                        // COMPACT MODE
                        item->setText(0, "");
                        item->setToolTip(0, item->data(0, Qt::UserRole).toString());
                        item->setTextAlignment(0, Qt::AlignCenter);

                        // Ensure the icon is visible! (Check your resource path)
                        // If you don't have the svg, try a standard fallback for now:
                        if (item->icon(0).isNull()) {
                            item->setIcon(0, style()->standardIcon(QStyle::SP_DirIcon));
                        }
                    } else {
                        // NORMAL MODE
                        item->setText(0, item->data(0, Qt::UserRole).toString());
                        item->setToolTip(0, "");
                        item->setTextAlignment(0, Qt::AlignLeft | Qt::AlignVCenter);
                    }
                }
            }
        }
    }
    QWidget::changeEvent(event);
}
//
// Created by DevAccount on 23/12/2025.
//
#include <UI/components/SIde_Bar/componets/FiltersSection.h>
#include <QVBoxLayout>
#include <UI/helpers/IconHelper.h>
#include <QVariant>
#include <QPainter>
#include <QPixmap>


const QString kFilterStyle = R"(
    QTreeWidget#FilterTree {
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
        /* The Glow Effect */
        background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 rgba(50, 150, 255, 40), stop:1 rgba(0, 0, 0, 0));
        border-left: 3px solid #64B5F6;
        color: white;
    }
    /* Arrow Icons - matching your theme */
    QTreeView::branch:has-children:!has-siblings:closed,
    QTreeView::branch:closed:has-children:has-siblings { image: url(:/icons/chevron-right.svg); }
    QTreeView::branch:open:has-children:!has-siblings,
    QTreeView::branch:open:has-children:has-siblings { image: url(:/icons/chevron-down.svg); }
)";

FiltersSection::FiltersSection(QWidget *parent) : BaseNavTree(parent)
{
    auto* lay = new QVBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);

    treeList = new QTreeWidget(this);
    treeList->setObjectName("FilterTree");
    treeList->setHeaderHidden(true);
    treeList->setIndentation(15);
    treeList->setAnimated(true);
    treeList->setFocusPolicy(Qt::NoFocus);

    // Scrollbar policies - Filters usually don't need scrollbars if the list is short
    treeList->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    treeList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    treeList->setStyleSheet(kFilterStyle);

    // --- Create the Root Header "Filters" ---
    auto* root = new QTreeWidgetItem(treeList);
    root->setText(0, "Filters");
    // Make the header non-selectable so the "Glow" doesn't apply to it
    root->setFlags(Qt::ItemIsEnabled);
    root->setExpanded(true);
#ifdef Q_OS_WIN
    root->setFont(0, QFont("Segoe UI", 9, QFont::Bold));
#else
    root->setFont(0, QFont(".AppleSystemUIFont", 9, QFont::Bold));
#endif

    // --- Add Filter Items ---
    // You can use IDs (1, 2, 3...) to identify them in your main logic
    addFilter(root, "High Priority", 1);
    addFilter(root, "Overdue", 2, 3); // Example count: 3
    addFilter(root, "No Due Date", 3);
    addFilter(root, "Completed", 4);

    lay->addWidget(treeList);

    // Height calculation: Header + (Items * RowHeight) + Buffer
    // This helps the Sidebar layout know how much space to reserve
    treeList->setFixedHeight(32 + (4 * 32) + 10);

    // Signal connection
    connect(treeList, &QTreeWidget::itemClicked, this, [this](QTreeWidgetItem* item, int col){
        // Only emit if it is a child item (has a parent)
        if(item->parent()) {
            // Retrieve the ID we stored in UserRole
            int id = item->data(0, Qt::UserRole).toInt();
            emit filterSelected(id);
        }
    });
}

void FiltersSection::setCompact(bool compact) {
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

            // --- LOGIC FOR INITIALS ---
            QString initials;
            QColor color;

            if (fullName.contains("High Priority", Qt::CaseInsensitive)) {
                initials = "HP"; color = "#E74C3C"; // Red
            }
            else if (fullName.contains("Overdue", Qt::CaseInsensitive)) {
                initials = "OV"; color = "#E67E22"; // Orange
            }
            else if (fullName.contains("No Due", Qt::CaseInsensitive)) {
                initials = "ND"; color = "#95A5A6"; // Grey
            }
            else if (fullName.contains("Completed", Qt::CaseInsensitive)) {
                initials = "OK"; color = "#27AE60"; // Green
            }
            else {
                // Default: First 2 letters
                initials = fullName.left(2).toUpper();
                color = "#6c757d";
            }

            // USE THE HELPER
            item->setIcon(0, IconHelper::generateInitialsIcon(initials, color));
        }
    } else {
        // --- DEFAULT MODE ---
        root->setText(0, "Filters");
        root->setSizeHint(0, QSize(0, 30));

        for(int i = 0; i < root->childCount(); ++i) {
            QTreeWidgetItem* item = root->child(i);
            QString fullName = item->data(0, Qt::UserRole).toString();

            item->setText(0, fullName);
            item->setTextAlignment(0, Qt::AlignLeft | Qt::AlignVCenter);
            item->setToolTip(0, "");

            // Restore original SVGs if desired, or keep the generated ones
            // Example restoration:
            if (fullName.contains("Priority")) item->setIcon(0, QIcon(":/icons/flag.svg"));
            else if (fullName.contains("Overdue")) item->setIcon(0, QIcon(":/icons/alert.svg"));
            else if (fullName.contains("Completed")) item->setIcon(0, QIcon(":/icons/check.svg"));
            else if (fullName.contains("No Due")) item->setIcon(0, QIcon(":/icons/calendar.svg"));
            else item->setIcon(0, QIcon(":/icons/filter.svg"));
        }
    }
}

void FiltersSection::addFilter(QTreeWidgetItem* parent, const QString& name, int id, int count) {
    auto* item = new QTreeWidgetItem(parent);

    QString text = name;
    if(count > 0) text += QString(" (%1)").arg(count);

    item->setText(0, text);
    item->setData(0, Qt::UserRole, text); // Store text
    item->setData(0, Qt::UserRole + 1, id); // Store ID

    // 1. ASSIGN DISTINCT ICONS
    // This allows the user to know which filter is which just by looking at the icon
    if (name.contains("Priority", Qt::CaseInsensitive))
        item->setIcon(0, QIcon(":/icons/flag.svg"));
    else if (name.contains("Overdue", Qt::CaseInsensitive))
        item->setIcon(0, QIcon(":/icons/alert.svg"));
    else if (name.contains("Completed", Qt::CaseInsensitive))
        item->setIcon(0, QIcon(":/icons/check.svg"));
    else if (name.contains("No Due", Qt::CaseInsensitive))
        item->setIcon(0, QIcon(":/icons/calendar.svg"));
    else
        item->setIcon(0, QIcon(":/icons/filter.svg")); // Fallback

    item->setSizeHint(0, QSize(0, 32));
}

void FiltersSection::changeEvent(QEvent *event) {
    if (event->type() == QEvent::DynamicPropertyChange) {
        auto* propEvent = dynamic_cast<QDynamicPropertyChangeEvent*>(event);
        if (propEvent->propertyName() == "compact") {
            bool compact = property("compact").toBool();

            // --- FIX: Toggle Indentation ---
            treeList->setIndentation(compact ? 0 : 15);

            QTreeWidgetItem* root = treeList->topLevelItem(0);
            if (root) {
                root->setHidden(compact);

                for(int i = 0; i < root->childCount(); ++i) {
                    QTreeWidgetItem* item = root->child(i);
                    if (compact) {
                        item->setText(0, "");
                        item->setToolTip(0, item->data(0, Qt::UserRole).toString());
                        item->setTextAlignment(0, Qt::AlignCenter);

                        // Fallback icon check
                        if (item->icon(0).isNull()) {
                            item->setIcon(0, style()->standardIcon(QStyle::SP_FileIcon));
                        }
                    } else {
                        item->setText(0, item->data(0, Qt::UserRole).toString());
                        item->setToolTip(0, "");
                        item->setTextAlignment(0, Qt::AlignLeft | Qt::AlignVCenter);
                    }
                }
            }
        }
    }
    QTreeView::changeEvent(event);
}
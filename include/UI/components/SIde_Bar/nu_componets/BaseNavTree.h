//
// Created by DevAccount on 27/07/2026.
//

#ifndef TASKHELPER_nuBASENAVTREE_H
#define TASKHELPER_nuBASENAVTREE_H
#pragma once
#include <QWidget>
#include <QListWidget>
#include <QTreeWidget>
#include <QDynamicPropertyChangeEvent>

class nu_BaseNavTree : public QTreeWidget
{
    Q_OBJECT
public:
    explicit nu_BaseNavTree(QWidget* parent = nullptr);

    /**
     * @brief Adds a top-level navigation item.
     * @param text Display label.
     * @param routeId Internal identifier for routing/switching views.
     * @param icon Optional icon.
     * @return QTreeWidgetItem* Pointer to the created item for adding sub-items.
     */
    QTreeWidgetItem* addNavItem(const QString& text, const QString& routeId, const QIcon& icon = QIcon()) {
        auto* item = new QTreeWidgetItem(this);
        item->setText(0, text);
        item->setIcon(0, icon);
        item->setData(0, Qt::UserRole, routeId);
        return item;
    }

    /**
     * @brief Adds a nested sub-navigation item.
     * @param parent Parent tree item.
     * @param text Display label.
     * @param routeId Internal identifier.
     * @return QTreeWidgetItem* Pointer to the child item.
     */
    static QTreeWidgetItem* addSubNavItem(QTreeWidgetItem* parent, const QString& text, const QString& routeId) {
        auto* item = new QTreeWidgetItem(parent);
        item->setText(0, text);
        item->setData(0, Qt::UserRole, routeId);
        return item;
    }

    signals:
        void navItemClicked(const QString& routeId);
};

inline nu_BaseNavTree::nu_BaseNavTree(QWidget* parent): QTreeWidget(parent)
{
    setHeaderHidden(true);
    setIndentation(15);
    setFrameShape(QFrame::NoFrame);
    setFocusPolicy(Qt::NoFocus); // Removes dotted border
    setAnimated(true);

    // Fix for standard Qt height issues
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    setStyleSheet(R"(
            QTreeWidget {
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
            /* Tree Arrows */
            QTreeView::branch:has-children:!has-siblings:closed,
            QTreeView::branch:closed:has-children:has-siblings { image: url(:/icons/chevron-right.svg); }
            QTreeView::branch:open:has-children:!has-siblings,
            QTreeView::branch:open:has-children:has-siblings { image: url(:/icons/chevron-down.svg); }
        )");


    connect(this, &QTreeWidget::itemClicked, this, [this](QTreeWidgetItem* item, int column)
    {
        Q_UNUSED(column);

        QString routeId = item->data(0, Qt::UserRole).toString();
        if (!routeId.isEmpty()) {
            emit navItemClicked(routeId);
        }
    });

}
#endif //TASKHELPER_BASENAVTREE_H

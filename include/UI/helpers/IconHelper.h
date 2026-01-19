//
// Created by DevAccount on 02/01/2026.
//

#ifndef TASKHELPER_ICONHELPER_H
#define TASKHELPER_ICONHELPER_H

#include <QIcon>
#include <QPainter>
#include <QPixmap>
#include <QColor>
#include <QFont>
#include <QString>

namespace IconHelper {

    // Generates a circular "Avatar" icon with initials
    inline QIcon generateInitialsIcon(const QString& text, const QColor& bg, const QColor& fg = Qt::white, int size = 24) {
        QPixmap pixmap(size, size);
        pixmap.fill(Qt::transparent);

        QPainter painter(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing);

        // 1. Draw Background Circle
        painter.setBrush(bg);
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(0, 0, size, size);

        // 2. Draw Initials
        painter.setPen(fg);
        // Scale font slightly based on size (approx 40% of icon size)
#ifdef Q_OS_WIN
        painter.setFont(QFont("Segoe UI", size * 0.5, QFont::Bold));
#else
        painter.setFont(QFont(".AppleSystemUIFont", size * 0.5, QFont::Bold));
#endif
        painter.drawText(pixmap.rect(), Qt::AlignCenter, text);

        return QIcon(pixmap);
    }
}

#endif //TASKHELPER_ICONHELPER_H
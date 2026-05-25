//
// Created by DevAccount on 16/05/2026.
//

#include "UI/components/Dialogs/DropAreaWidget.h"
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QUrl>
#include <QFileInfo>
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QDebug>
#include <QTimer>
#include <QFileDialog>

DropAreaWidget::DropAreaWidget(QWidget* parent)
    : QWidget(parent),
      m_flashTimer(new QTimer(this))
{
    setAcceptDrops(true);
    setMinimumHeight(100);
    setStyleSheet("background: transparent; border: 2px dashed #aaa; border-radius: 10px;");

    m_flashTimer->setInterval(440);
    connect(m_flashTimer, &QTimer::timeout, this, &DropAreaWidget::onFlashTimerTimeout);
}

void DropAreaWidget::resetToIdleState()
{
    m_isDragActive = false;
    m_flashState = false;
    if (m_flashTimer->isActive())
    {
        m_flashTimer->stop();
    }
    update();
}

void DropAreaWidget::setAllowedExtensions(const QStringList& extensions)
{
    m_allowedExtensions.clear();
    for (const QString& ext : extensions)
    {
        QString e =ext;
        if (e.startsWith('.'))
        {
            e.remove(0, 1);
        }
        m_allowedExtensions.append(e.toLower());
    }
}

void DropAreaWidget::dragEnterEvent(QDragEnterEvent* event)
{
    const QMimeData* md = event->mimeData();
    if (!md)
    {
        event->ignore();
        return;
    }

    const QList<QUrl> urls = md->urls();
    bool anyAcceptable = false;
    for (const QUrl& url : urls)
    {
        if (!url.isLocalFile()) continue;
        const QFileInfo fi(url.toLocalFile());
        if (!fi.exists() || fi.isDir()) continue;
        if (!m_allowedExtensions.isEmpty()) {
            const QString ext = fi.suffix().toLower();
            if (!m_allowedExtensions.contains(ext)) continue;
        }
        anyAcceptable = true;
        break;
    }

    if (anyAcceptable)
    {
        event->acceptProposedAction();
        m_isDragActive = true;
        m_flashTimer->start();
        update();
    }else
    {
        event->ignore();
    }
}

void DropAreaWidget::dragLeaveEvent(QDragLeaveEvent* event)
{
    Q_UNUSED(event);
    resetToIdleState();
}

void DropAreaWidget::dropEvent(QDropEvent* event)
{
    const QMimeData* md = event->mimeData();
    QStringList acceptedFiles;

    if (md && md->hasUrls())
    {
        const QList<QUrl> urls = md->urls();
        for (const QUrl& url : urls)
        {
            if (!url.isLocalFile()) continue;
            const QString local = url.toLocalFile();
            QFileInfo fi(local);
            if (!fi.exists() || fi.isDir()) continue;
            if (!m_allowedExtensions.isEmpty()) {
                const QString ext = fi.suffix().toLower();
                if (!m_allowedExtensions.contains(ext)) continue;
            }
            acceptedFiles << fi.absoluteFilePath();
        }
    }

    resetToIdleState();

    if (!acceptedFiles.isEmpty())
    {
        emit filesDropped(acceptedFiles);
        event->acceptProposedAction();
    }else
    {
        event->ignore();
    }
}

void DropAreaWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const QRect r = rect().adjusted(6, 6, -6, -6);
    QPen pen;
    pen.setWidth(2);

    // Grab dynamic colors from the current OS/App theme
    QColor themeTextColor = palette().color(QPalette::WindowText);
    QColor themeIdleBorder = palette().color(QPalette::Mid);

    if (m_isDragActive) {
        // Keep your awesome green flash for the active state
        pen.setStyle(Qt::SolidLine); // Solid line looks better when active
        QColor color = m_flashState ? QColor(0, 170, 0, 200) : QColor(0, 170, 0, 120);
        pen.setColor(color);

        QBrush brush(color.lighter(200));
        brush.setStyle(Qt::SolidPattern);
        p.setBrush(brush); // Actually fill the background!
    } else {
        // Use the dynamic theme color for the idle dashed line
        pen.setStyle(Qt::DashLine);
        pen.setColor(themeIdleBorder);
        p.setBrush(Qt::NoBrush);
    }

    p.setPen(pen);
    p.drawRoundedRect(r, 10, 10);

    const QString hint = tr("Drop files here to attach");
    QFont f = font();
    f.setPointSize(10);
    p.setFont(f);

    // Switch text color based on state
    QColor textColor = m_isDragActive ? QColor(0, 120, 0) : themeTextColor;
    p.setPen(textColor);
    p.drawText(r, Qt::AlignCenter, hint);
}

void DropAreaWidget::onFlashTimerTimeout()
{
    m_flashState = !m_flashState;
    update();
}



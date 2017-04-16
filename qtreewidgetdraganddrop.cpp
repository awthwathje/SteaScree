#include "qtreewidgetdraganddrop.h"

#include <QDragEnterEvent>
#include <QMimeData>
#include <QDebug>

QTreeWidgetDragAndDrop::QTreeWidgetDragAndDrop(QWidget *parent) : QTreeWidget(parent)
{
}


void QTreeWidgetDragAndDrop::dragEnterEvent(QDragEnterEvent *event)
{
    event->acceptProposedAction();
}


void QTreeWidgetDragAndDrop::dragMoveEvent(QDragMoveEvent *event)
{
    event->acceptProposedAction();
}


void QTreeWidgetDragAndDrop::dropEvent(QDropEvent *event)
{
        const QList<QUrl> urls = event->mimeData()->urls();
        QStringList screenshotsDropped;

        QListIterator<QUrl> i(urls);
        while ( i.hasNext() ) {
            QUrl current = i.next();
            screenshotsDropped << current.toLocalFile();
        }

        emit sendDroppedScreenshots(screenshotsDropped);
}

#ifndef QTREEWIDGETDRAGANDDROP_H
#define QTREEWIDGETDRAGANDDROP_H

#include <QTreeWidget>

class QTreeWidgetDragAndDrop : public QTreeWidget
{
    Q_OBJECT

public:
    explicit QTreeWidgetDragAndDrop(QWidget *parent = 0);

protected:
    void dragEnterEvent(QDragEnterEvent *event) Q_DECL_OVERRIDE;
    void dragMoveEvent(QDragMoveEvent *event) Q_DECL_OVERRIDE;
    void dropEvent(QDropEvent *event) Q_DECL_OVERRIDE;

signals:
    void sendDroppedScreenshots(QStringList droppedScreenshots);

};

#endif // QTREEWIDGETDRAGANDDROP_H

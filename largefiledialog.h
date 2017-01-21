#ifndef LARGEFILEDIALOG_H
#define LARGEFILEDIALOG_H

#include "screenshot.h"

#include <QDialog>
#include <QPushButton>


namespace Ui {
class LargeFileDialog;
}


class LargeFileDialog : public QDialog
{
    Q_OBJECT


public:
    explicit LargeFileDialog(QWidget *parent = 0);
    ~LargeFileDialog();


protected:
    void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;


private:
    Ui::LargeFileDialog *ui;
    QStringList currentScreenshot;
    QImage currentImage;
    QList<quint32> currentGeometry;
    QList<Screenshot> screenshotListWithDecisions;
    qint32 currentIndex;
    QString steamMaxSideSize;
    QString steamMaxResolution;

    void prepareWindow(QPoint center);
    void processDecision(quint32 decision, bool all);
    void updateInfoInWindow(Screenshot screenshot);


signals:
    void sendButtonList(QList<QPushButton*> buttonList);
    void returnScreenshotListWithDecisions(QList<Screenshot>);


public slots:
    void getDecisions(QList<Screenshot> screenshotList, QPoint center, QStringList steamLimits);


private slots:
    void on_pushButton_resize_clicked();
    void on_pushButton_resizeAll_clicked();
    void on_pushButton_skip_clicked();
    void on_pushButton_skipAll_clicked();
    void on_pushButton_try_clicked();
    void on_pushButton_tryAll_clicked();
};

#endif // LARGEFILEDIALOG_H

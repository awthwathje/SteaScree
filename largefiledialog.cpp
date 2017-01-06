#include "largefiledialog.h"
#include "ui_largefiledialog.h"


#include <QCloseEvent>
#include <QDebug>


LargeFileDialog::LargeFileDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LargeFileDialog)
{
    ui->setupUi(this);
}


LargeFileDialog::~LargeFileDialog()
{
    delete ui;
}


void LargeFileDialog::prepareWindow(QPoint center)
{
    QList<QPushButton*> buttonList;

    buttonList << ui->pushButton_resize << ui->pushButton_resizeAll << ui->pushButton_skip << ui->pushButton_skipAll
               << ui->pushButton_try << ui->pushButton_tryAll;

    emit sendButtonList(buttonList);    // adjust button padding

    QSize size(750, 290);               // this dialog window size, hardcoded to prevent resizing
    this->setFixedSize(size);
    this->move(center.x() - (size.width() / 2), center.y() - (size.height() / 2)); // this window will show approximately in the center of the main window, covering and blocking it
    this->show();
    this->setAttribute(Qt::WA_DeleteOnClose);
}


void LargeFileDialog::getDecisions(QList<Screenshot> screenshotList, QPoint center, QStringList steamLimits)
{
    screenshotListWithDecisions = screenshotList;
    prepareWindow(center);
    currentIndex = 0;
    steamMaxSideSize = steamLimits[0];
    steamMaxResolution = steamLimits[1];
    updateInfoInWindow(screenshotList[currentIndex]);
}


void LargeFileDialog::updateInfoInWindow(Screenshot screenshot)
{
    QString width = QString::number(screenshot.geometry[0]);
    QString height = QString::number(screenshot.geometry[1]);
    QString resolution = QString::number(screenshot.geometry[0] * screenshot.geometry[1]);

    QString info = "File <b>" + screenshot.screenshot[1] + "</b> is too large and most likely will be refused by the Steam Cloud.<br><br>" +
            QString("Steam Cloud typically accepts images with a size of any side of up to <b>" + steamMaxSideSize + "</b> pixels and maximum resolution of <b>" + steamMaxResolution + "</b> pixels.<br><br>") +
            QString("Your file is <b>" + width + "×" + height + "</b> with a resolution of <b>" + resolution + "</b> pixels and thus exceeds Steam Cloud limit.<br><br>") +
            QString("You can resize it and rest of such files in the queue to the maximum allowed resolution, skip this file or even try to upload it to the Steam Cloud anyway.<br><br>") +
            QString("What is you are up to?");

    ui->label_image->setPixmap(screenshot.thumbnail);
    ui->label_details->setText(info);
}


void LargeFileDialog::processDecision(quint32 decision, bool all)
{
    screenshotListWithDecisions[currentIndex++].decision = decision;

    if (currentIndex < screenshotListWithDecisions.length()) {

        if (!all)
            updateInfoInWindow(screenshotListWithDecisions[currentIndex]);
        else {
            for (qint32 i = currentIndex; i < screenshotListWithDecisions.length(); i++)
                screenshotListWithDecisions[i].decision = decision;
            this->accept();
            emit returnScreenshotListWithDecisions(screenshotListWithDecisions);
        }

    } else {
        this->accept();
        emit returnScreenshotListWithDecisions(screenshotListWithDecisions);
    }
}


void LargeFileDialog::on_pushButton_resize_clicked()
{
    processDecision(1, false);
}


void LargeFileDialog::on_pushButton_resizeAll_clicked()
{
    processDecision(1, true);
}


void LargeFileDialog::on_pushButton_skip_clicked()
{
    processDecision(2, false);
}


void LargeFileDialog::on_pushButton_skipAll_clicked()
{
    processDecision(2, true);
}


void LargeFileDialog::on_pushButton_try_clicked()
{
    processDecision(3, false);
}


void LargeFileDialog::on_pushButton_tryAll_clicked()
{
    processDecision(3, true);
}


void LargeFileDialog::closeEvent(QCloseEvent *event)
{
    processDecision(1, true);
    event->accept();
}

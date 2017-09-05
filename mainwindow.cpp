#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFile>
#include <QFileInfo>
#include <QFileDialog>
#include <QDateTime>
#include <QList>
#include <QString>
#include <QStringList>
#include <QTreeWidgetItem>
#include <QSize>
#include <QRegularExpression>
#include <QMessageBox>
#include <QTimer>
#include <QCloseEvent>
#include <QCheckBox>
#include <QMovie>
#include <QDesktopServices>
#include <QShortcut>
#include <QDebug>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}


MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::bootStrap()
{
    setLabelsVisible(QStringList() << "progressBar_status" << "label_progress", false);              // initial widget states setting
    setWidgetsDisabled(QStringList() << "pushButton_clearQueue" << "pushButton_copyScreenshots" << "pushButton_prepare", true);
    setDirStatusLabelsVisible(false);

    ui->label_progress->setMovie(gifLoader);
    gifLoader->start();

    QSizePolicy spRetain = ui->label_infoScreenshots->sizePolicy();     // hack to prevent layout size change on a widget visibility changing events
    spRetain.setRetainSizeWhenHidden(true);
    ui->label_infoScreenshots->setSizePolicy(spRetain);
    ui->progressBar_status->setSizePolicy(spRetain);

    emit sendButtonList(QList<QPushButton*>() << ui->pushButton_clearQueue << ui->pushButton_copyScreenshots    // buttons should have specific padding
                        << ui->pushButton_addScreenshots << ui->pushButton_prepare);                            // ...in each OS

    userIDComboBox = ui->comboBox_userID;
    QObject::connect(userIDComboBox, static_cast<void(QComboBox::*)(const QString &)>(&QComboBox::activated),
                     this, &MainWindow::reactToComboBoxActivation);

    QTreeWidgetDragAndDrop *treeWidget = ui->treeWidget_screenshotList;
    emit sendTreeWidgetPointer(treeWidget);
}


void MainWindow::reactToComboBoxActivation(QString userID)
{
    emit sendNewlySelectedUserID(userID.remove(QRegularExpression(" <.+>$")));
}


void MainWindow::addWidgetItemToScreenshotList(QTreeWidgetItem *item)
{
    ui->treeWidget_screenshotList->addTopLevelItem(item);
}


void MainWindow::resizeScreenshotListColumns()
{
    ui->treeWidget_screenshotList->resizeColumnToContents(0); // when all is added, resize columns for a better appearance
    ui->treeWidget_screenshotList->resizeColumnToContents(1);
}


void MainWindow::makeWideMessageBox(QMessageBox *msgBox, quint32 width) // hack to make wide message boxes
{
    QGridLayout* layout = (QGridLayout*)msgBox->layout();
    layout->addItem(new QSpacerItem(width, 0, QSizePolicy::Minimum, QSizePolicy::Expanding), layout->rowCount(), 0, 1, layout->columnCount());
}


void MainWindow::locateSteamDir(QString steamDir)
{
    QString steamDirLocated = QFileDialog::getExistingDirectory(this,
                                                                "Locate Steam directory",
                                                                steamDir,
                                                                QFileDialog::ShowDirsOnly | QFileDialog::ReadOnly);
    if ( !steamDirLocated.isEmpty() ) {

        steamDirLocated.remove(QRegularExpression("/userdata$"));
        emit sendUserDataPaths(steamDirLocated);
    }
}


void MainWindow::offerUpdate(QString version, QString link)
{
    QMessageBox msgBox(this);
    msgBox.setIcon(QMessageBox::Question);
    QString text = "New version available.";
    QString info = "SteaScree version " + version + " is available online. Would you like to download it?";
    msgBox.setText(text);
    msgBox.setInformativeText(info);
    QPushButton *never = msgBox.addButton("Never", QMessageBox::RejectRole);
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::Yes);

    makeWideMessageBox(&msgBox, 400);
    int ret = msgBox.exec();

    if ( ret == QMessageBox::Yes )
        QDesktopServices::openUrl(QUrl(link));
    else if ( msgBox.clickedButton() == never )
        emit sendNeverOfferUpdate();
}


void MainWindow::setLabelsText(QStringList list, QString text)
{
    QStringListIterator i(list);
    while ( i.hasNext() ) {
        QString current = i.next();
        QLabel *label = this->findChild<QLabel*>(current);
        label->setText(text);
    }
}


void MainWindow::setWidgetsDisabled(QStringList list, bool disable)
{
    QStringListIterator i(list);
    while ( i.hasNext() ) {
        QString current = i.next();
        QWidget *widget = this->findChild<QWidget*>(current);
        widget->setDisabled(disable);
    }
}


void MainWindow::setLabelsVisible(QStringList list, bool visible)
{
    QStringListIterator i(list);
    while ( i.hasNext() ) {
        QString current = i.next();
        QWidget *widget = this->findChild<QWidget*>(current);
        widget->setVisible(visible);
    }
}


void MainWindow::setDirStatusLabelsVisible(bool visible) // info labels show/hide toggle
{
    QStringList labelInfoList = QStringList() << "label_infoScreenshots" << "label_infoScreenshotsCopied"
                                              << "label_infoDirectories" << "label_infoDirectoriesCopied";
    setLabelsVisible(labelInfoList, visible);
}


void MainWindow::setComboBoxesCleared(QStringList list)
{
    QStringListIterator i(list);
    while ( i.hasNext() ) {
        QString current = i.next();
        QComboBox *widget = this->findChild<QComboBox*>(current);
        widget->clear();
    }
}


void MainWindow::setLabelsCleared(QStringList list)
{
    QStringListIterator i(list);
    while ( i.hasNext() ) {
        QString current = i.next();
        QLabel *label = this->findChild<QLabel*>(current);
        label->clear();
    }
}


void MainWindow::setProgressBarLength(quint32 length)
{
    ui->progressBar_status->setMaximum(length);
}


void MainWindow::setProgressBarValue(quint32 value)
{
    ui->progressBar_status->setValue(value);
}


void MainWindow::moveWindow(QSize geometry, QPoint moveToPoint)
{
    resize(geometry);
    move(moveToPoint);
}


void MainWindow::insertIntoComboBox(QString name, QStringList items)
{
    QComboBox *comboBox = this->findChild<QComboBox*>(name);
    comboBox->insertItems(0, items);
}


void MainWindow::setIndexOfComboBox(QString name, QString text)
{
    QComboBox *comboBox = this->findChild<QComboBox*>(name);
    comboBox->setCurrentIndex(comboBox->findText(text, Qt::MatchStartsWith));
}


void MainWindow::setLabelsOnMissingStuff(bool userDataMissing, QString vdfFilename)
{
    ui->label_status->setVisible(true);
    if ( userDataMissing )
        setStatusLabelText("Steam userdata directory is not found", warningColor);
    else
        setStatusLabelText("Steam userdata directory is found, but " + vdfFilename + " is missing", warningColor);

    ui->label_steamDirValue->setText("not found");
    ui->label_steamDirValue->setStyleSheet("color: gray;");
}


void MainWindow::returnScreenshotsSelected(QString lastSelectedScreenshotDir)
{
    QStringList screenshotsSelected = QFileDialog::getOpenFileNames(this,
                                                                    "Select one or several screenshots",
                                                                    lastSelectedScreenshotDir,
                                                                    "Images (*.jpg *.jpeg *.png *.bmp *.tif *.tiff)");
    emit sendScreenshotsSelected(screenshotsSelected);
}


void MainWindow::deleteCopiedWidgetItem(QString path)
{
    QFile file(path);
    QTreeWidgetItem *item = ui->treeWidget_screenshotList->findItems(QFileInfo(file).lastModified()
                                                                     .toString("yyyy/MM/dd hh:mm:ss"), Qt::MatchExactly, 1)[0];
    delete item;
}


void MainWindow::disableAllControls()
{
    setWidgetsDisabled(QStringList() << "pushButton_clearQueue" << "pushButton_addScreenshots" << "pushButton_copyScreenshots"
                       << "pushButton_locateSteamDir" << "comboBox_userID" << "comboBox_gameID" << "pushButton_prepare", true);
}


void MainWindow::setStatusLabelText(QString text, QString color)
{
    ui->label_status->setText(text);
    if ( color.isEmpty() )
        color = "black";
    ui->label_status->setStyleSheet("QLabel {color: " + color + "};");
}


void MainWindow::setJpegQualityValue(quint32 jpegQualityValue)
{
    ui->spinBox_jpegQuality->setValue(jpegQualityValue);
}


void MainWindow::prepareScreenshots(quint32 addedLines)
{
    if ( addedLines > 0 ) {

        QMessageBox msgBox(this);
        msgBox.setIcon(QMessageBox::Warning);

        QString text = "Steam has to be quitted.";
        QString info = "This program only works when Steam exited. It will not try to determine if Steam is running or not, so you should be sure it is quitted. " +
                QString("If it is not, it is safe to exit Steam now. <br><br>Is Steam exited now?");
        msgBox.setText(text);
        msgBox.setInformativeText(info);
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::Yes);

        makeWideMessageBox(&msgBox, 500);

        int ret = msgBox.exec();

        if ( ret == QMessageBox::Yes ) {

            setDirStatusLabelsVisible(false);
            setLabelsText(QStringList() << "label_infoScreenshots" << "label_infoDirectories", "0");
            ui->pushButton_prepare->setDisabled(true);

            emit writeVDF();

            emit clearScreenshotPathsPool();
            emit clearState();

            setStatusLabelText("ready", "");

            QMessageBox msgBox(this);
            msgBox.setIcon(QMessageBox::Information);
            msgBox.setText("SteaScree has updated the VDF-file.");
            msgBox.setInformativeText("Now you can start Steam as usual and upload screenshots to the Steam Cloud.");
            msgBox.exec();
        }

    } else {

        QMessageBox msgBox(this);
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText("All screenshots from the upload queue are already in the screenshots.vdf file.");
        msgBox.setInformativeText("Nothing has been added. Please add new screenshots and try again.");
        msgBox.exec();

        setDirStatusLabelsVisible(false);
        ui->pushButton_prepare->setDisabled(true);
    }

    emit clearCopyingStatusLabels();
}


void MainWindow::on_pushButton_locateSteamDir_clicked()
{
    emit getSteamDir();
}


void MainWindow::on_pushButton_addScreenshots_clicked()
{
    emit pushButton_addScreenshots_clicked();
}


void MainWindow::on_pushButton_clearQueue_clicked()
{  
    ui->treeWidget_screenshotList->clear();

    setWidgetsDisabled(QStringList() << "pushButton_clearQueue" << "pushButton_copyScreenshots", true);

    setWidgetsDisabled(QStringList() << "label_userID" << "comboBox_gameID" << "label_gameID"
                       << "comboBox_userID" << "pushButton_locateSteamDir", false);

    emit clearScreenshotPathsPool();
}


void MainWindow::on_pushButton_copyScreenshots_clicked()
{
    QString selectedUserID = ui->comboBox_userID->currentText();
    QString selectedGameID = ui->comboBox_gameID->currentText();
    quint32 jpegQuality = ui->spinBox_jpegQuality->value();

    QRegularExpression re("^[0-9]+( <.+>)?$");

    if ( !selectedGameID.contains(re) )
        setStatusLabelText("invalid game ID, only numbers allowed", warningColor);
    else {  // valid game ID

        disableAllControls();
        ui->label_status->clear();
        setLabelsVisible(QStringList() << "label_status" << "label_progress" << "progressBar_status", true);
        setWidgetsDisabled( QStringList() << "pushButton_addScreenshots" << "pushButton_locateSteamDir", true);
        setStatusLabelText("analyzing queued screenshots", "");
        ui->progressBar_status->setValue(0);

        emit sendSelectedIDs(selectedUserID, selectedGameID, jpegQuality, this);
    }
}


void MainWindow::on_pushButton_prepare_clicked()
{
    emit pushButton_prepare_clicked();
}


void MainWindow::closeEvent(QCloseEvent *event)
{
    emit sendSettings(size(), pos(),
                      ui->comboBox_userID->currentText().remove(QRegularExpression(" <.+>$")),
                      ui->comboBox_gameID->currentText().remove(QRegularExpression(" <.+>$")),
                      ui->spinBox_jpegQuality->value());
    event->accept();
}

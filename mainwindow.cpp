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
    ui->progressBarScreenshotsUploading->setVisible(false); // initial widget states setting
    ui->labelStatusError->setVisible(false);
    setWidgetsDisabled(QStringList() << "pushButtonClearQueue" << "pushButtonCopyScreenshots" << "pushButtonPrepare", true);

    QSizePolicy sp_retain = ui->labelInfoScreenshots->sizePolicy(); // hack to prevent layout size change on a widget visibility changing events
    sp_retain.setRetainSizeWhenHidden(true);
    ui->labelInfoScreenshots->setSizePolicy(sp_retain);
    ui->progressBarScreenshotsUploading->setSizePolicy(sp_retain);

    toggleLabelInfo(false); // information labels are hidden at start

    emit getOS();
}


void MainWindow::setButtonPadding(QString os)
{
    QList<QPushButton*> buttonList; // list of buttons for setting a different padding for each OS
    buttonList << ui->pushButtonClearQueue << ui->pushButtonCopyScreenshots << ui->pushButtonAddScreenshots << ui->pushButtonPrepare;

    if (os == "Linux")
        foreach (QPushButton *button, buttonList)
            button->setStyleSheet("padding: 3px 13px");
    else if (os == "Windows")
        foreach (QPushButton *button, buttonList)
            button->setStyleSheet("padding: 4px 14px");
}


void MainWindow::checkVDF()
{
    emit getVDFStatus();
}


void MainWindow::toggleLabelInfo(bool isVisible) // info labels show/hide toggle
{
    QList<QLabel*> labelInfoList;
    labelInfoList << ui->labelInfoScreenshots << ui->labelInfo1 << ui->labelInfo2 << ui->labelInfoDirectories;
    foreach (QLabel *label, labelInfoList)
        label->setVisible(isVisible);
}


void MainWindow::addWidgetItemToScreenshotList(QTreeWidgetItem *item)
{
    ui->treeWidgetScreenshotList->addTopLevelItem(item);
}


void MainWindow::resizeScreenshotListColumns()
{
    ui->treeWidgetScreenshotList->resizeColumnToContents(0); // after all has been added, resize columns for a better appearance
    ui->treeWidgetScreenshotList->resizeColumnToContents(1);
}


void MainWindow::warnOnMissingVDF(bool userDataExists, QString vdfFilename)
{
    QMessageBox msgBox(this);
    msgBox.setIcon(QMessageBox::Warning);

    if ( !userDataExists ) {
        msgBox.setText("SteaScree has been unable to find a Steam userdata directory in the current location.");
        msgBox.setInformativeText("Please choose an existing Steam directory.");
    } else {
        msgBox.setText("Steam userdata directory is found, but there is no " + vdfFilename);
        msgBox.setInformativeText("Please start Steam, make some screenshots with it and try again.");
    };

    msgBox.exec();
}


void MainWindow::locateSteamDir(QString steamDir)
{
    QString steamDirLocated = QFileDialog::getExistingDirectory(this,
                                                         "Locate Steam directory",
                                                         steamDir,
                                                         QFileDialog::ShowDirsOnly | QFileDialog::ReadOnly);
    if ( !steamDirLocated.isEmpty() ) {

        steamDirLocated.remove(QRegularExpression("/userdata$"));
        emit setUserDataPaths(steamDirLocated);
    };
}


void MainWindow::setLabelsText(QStringList list, QString text)
{
    QStringListIterator i(list);
    while ( i.hasNext() ) {
        QString current = i.next();
        QLabel *label = this->findChild<QLabel*>(current);
        label->setText(text);
    };
}

void MainWindow::setWidgetsDisabled(QStringList list, bool disable)
{
    QStringListIterator i(list);
    while ( i.hasNext() ) {
        QString current = i.next();
        QWidget *widget = this->findChild<QWidget*>(current);
        widget->setDisabled(disable);
    };
}


void MainWindow::setComboBoxesCleared(QStringList list)
{
    QStringListIterator i(list);
    while ( i.hasNext() ) {
        QString current = i.next();
        QComboBox *widget = this->findChild<QComboBox*>(current);
        widget->clear();
    };
}


void MainWindow::setVisibleProgressBar(int length)
{
    ui->progressBarScreenshotsUploading->setVisible(true);
    ui->progressBarScreenshotsUploading->setMinimum(0);
    ui->progressBarScreenshotsUploading->setMaximum(length);
}


void MainWindow::setLabelStatusErrorVisible(bool visible)
{
    ui->labelStatusError->setVisible(visible);
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


void MainWindow::setIndexOfComboBoxUserID(QString item)
{
    ui->comboBoxUserID->setCurrentIndex(ui->comboBoxUserID->findText(item));
}


void MainWindow::setIndexOfComboBoxGameID(QString lastSelectedGameID)
{
    ui->comboBoxGameID->setCurrentIndex(ui->comboBoxGameID->findText(lastSelectedGameID, Qt::MatchStartsWith));
}


void MainWindow::setLabelsOnMissingStuff(bool userDataMissing, QString vdfFilename)
{
    ui->labelStatusError->setVisible(true);
    if ( userDataMissing )
        ui->labelStatusError->setText("Steam userdata directory is missing. Please locate correct Steam directory.");
    else
        ui->labelStatusError->setText("Steam userdata directory is found, but " + vdfFilename + " is missing.");

    ui->labelSteamDirValue->setText("not found");
    ui->labelSteamDirValue->setStyleSheet("color: gray;");
}


void MainWindow::returnComboBoxUserIDCurrentText()
{
    emit sendComboBoxUserIDCurrentText(ui->comboBoxUserID->currentText());
}


void MainWindow::returnScreenshotsSelected(QString lastSelectedScreenshotDir)
{
    QStringList screenshotsSelected = QFileDialog::getOpenFileNames(this,
                                                            "Select one or more screenshots",
                                                            lastSelectedScreenshotDir,
                                                            "Images (*.jpg *.jpeg *.png *.bmp *.tif *.tiff)");
    emit sendScreenshotsSelected(screenshotsSelected);
}


void MainWindow::setProgressBarValue(int value)
{
   ui->progressBarScreenshotsUploading->setValue(value);
}


void MainWindow::deleteCopiedWidgetItem(QString path)
{
    QFile file(path);
    QTreeWidgetItem *item = ui->treeWidgetScreenshotList->findItems(QFileInfo(file).lastModified()
                                                                    .toString("yyyy/MM/dd hh:mm:ss"), Qt::MatchExactly, 1)[0];
    delete item;
}


void MainWindow::prepareScreenshots(int addedLines)
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

        QGridLayout* layout = (QGridLayout*)msgBox.layout(); // hack to make wide message boxes
        layout->addItem(new QSpacerItem(500, 0, QSizePolicy::Minimum, QSizePolicy::Expanding), layout->rowCount(), 0, 1, layout->columnCount());

        int ret = msgBox.exec();

        if ( ret == QMessageBox::Yes ) {

            toggleLabelInfo(false);
            setLabelsText(QStringList() << "labelInfoScreenshots" << "labelInfoDirectories", "0");
            emit clearScreenshotPathsPool();

            ui->pushButtonPrepare->setDisabled(true);

            emit writeVDF();

            QMessageBox msgBox(this);
            msgBox.setIcon(QMessageBox::Information);
            msgBox.setText("SteaScree has updated the VDF-file.");
            msgBox.setInformativeText("Now you can start Steam as usual and upload screenshots to the Steam Cloud.");
            msgBox.exec();
        };

    } else {

        QMessageBox msgBox(this);
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText("All screenshots from the upload queue are already in the screenshots.vdf file.");
        msgBox.setInformativeText("Nothing has been added. Please add new screenshots and try again.");
        msgBox.exec();

        toggleLabelInfo(false);
        ui->pushButtonPrepare->setDisabled(true);
    };

    emit clearCopyingStatusLabels();
}


void MainWindow::on_pushButtonLocateSteamDir_clicked()
{
    emit getSteamDir();
}


void MainWindow::on_pushButtonAddScreenshots_clicked()
{
    emit pushButtonAddScreenshots_clicked();
}


void MainWindow::on_pushButtonClearQueue_clicked()
{
    ui->treeWidgetScreenshotList->clear();

    setWidgetsDisabled(QStringList() << "pushButtonClearQueue" << "pushButtonCopyScreenshots", true);

    setWidgetsDisabled(QStringList() << "labelUserID" << "comboBoxGameID" << "labelGameID"
                       << "comboBoxUserID" << "pushButtonLocateSteamDir", false);

    emit clearScreenshotPathsPool();
}


void MainWindow::on_pushButtonCopyScreenshots_clicked()
{
    QString selectedUserID = ui->comboBoxUserID->currentText();
    QString selectedGameID = ui->comboBoxGameID->currentText();

    QRegularExpression re("^[0-9]+( <.+>)?$");

    if ( !selectedGameID.contains(re) )

        ui->labelStatusError->setText("Invalid Game ID, only numbers allowed");

    else {

        ui->labelStatusError->clear();

        emit getScreenshotPathsPoolLength();
        setWidgetsDisabled( QStringList() << "pushButtonClearQueue" << "pushButtonAddScreenshots" << "pushButtonCopyScreenshots", true );
        toggleLabelInfo(true);
        emit pushScreenshots(selectedUserID, selectedGameID);
        setWidgetsDisabled( QStringList() << "pushButtonAddScreenshots" << "pushButtonPrepare", false);
        ui->pushButtonLocateSteamDir->setDisabled(true);
        ui->progressBarScreenshotsUploading->setVisible(false);
        ui->progressBarScreenshotsUploading->reset();
        ui->treeWidgetScreenshotList->clear();
        emit clearScreenshotPathsPool();
    };
}


void MainWindow::on_pushButtonPrepare_clicked()
{
    emit pushButtonPrepare_clicked();
}


void MainWindow::showEvent(QShowEvent *event) // hack to show message boxes only after the main window is shown
{
    QMainWindow::showEvent(event);
    QTimer::singleShot(50, this, SLOT(checkVDF()));
}


void MainWindow::closeEvent(QCloseEvent *event)
{
    emit sendSettings(size(), pos(),
                      ui->comboBoxUserID->currentText(),
                      ui->comboBoxGameID->currentText().remove(QRegularExpression(" <.+>$")));
    event->accept();
}

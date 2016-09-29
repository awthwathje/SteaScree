#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFile>
#include <QFileInfo>
#include <QFileDialog>
#include <QDir>
#include <QDirIterator>
#include <QListIterator>
#include <QList>
#include <QString>
#include <QStringList>
#include <QTreeWidgetItem>
#include <QDateTime>
#include <QSize>
#include <QImage>
#include <QIODevice>
#include <QTextStream>
#include <QRegularExpression>
#include <QMessageBox>
#include <QObject>
#include <QImageWriter>
#include <QUrl>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonArray>
#include <QTimer>
#include <QSettings>
#include <QCloseEvent>
#include <QRect>
#include <QDesktopWidget>


MainWindow::MainWindow(QWidget *parent) :

    QMainWindow(parent),
    ui(new Ui::MainWindow)

{
    ui->setupUi(this);

    if ( (os == "Linux") | (os == "macOS") ) {
        isUnixLikeOS = true;
        if ( os == "Linux" ) {
            settings = new QSettings(QSettings::NativeFormat, QSettings::UserScope, "Foyl", "SteaScree");
            defaultSteamDir = QDir::homePath() + "/.steam/steam";
        } else {
            settings = new QSettings(QSettings::NativeFormat, QSettings::UserScope, "foyl.io", "SteaScree");
            defaultSteamDir = QDir::homePath() + "/Library/Application Support/Steam";
        };
    } else {
        isUnixLikeOS = false;
        settings = new QSettings(QSettings::IniFormat, QSettings::UserScope, "Foyl", "SteaScree");
        if ( QSysInfo::currentCpuArchitecture() == "x86_64" )
            defaultSteamDir = "C:/Program Files (x86)/Steam";
        else
            defaultSteamDir = "C:/Program Files/Steam";
    };

    ui->progressBarScreenshotsUploading->setVisible(false); // initial widget states setting
    ui->pushButtonClearQueue->setDisabled(true);
    ui->pushButtonCopyScreenshots->setDisabled(true);
    ui->pushButtonPrepare->setDisabled(true);

    QSizePolicy sp_retain = ui->labelInfoScreenshots->sizePolicy(); // hack to prevent layout size change on a widget visibility changing events
    sp_retain.setRetainSizeWhenHidden(true);
    ui->labelInfoScreenshots->setSizePolicy(sp_retain);
    ui->progressBarScreenshotsUploading->setSizePolicy(sp_retain);

    toggleLabelInfo(false);

    readSettings();

    if ( !screenshotPathsPool.isEmpty() ) {
        populateScreenshotQueue(screenshotPathsPool);
        ui->pushButtonClearQueue->setDisabled(false);
        ui->pushButtonCopyScreenshots->setDisabled(false);
    };

    if ( !steamDir.isNull() ) {
        userDataDir = steamDir + "/userdata";
        setUserDataPaths(steamDir);
    };

    QObject::connect(this, &MainWindow::vdfIsMissing,
                     this, &MainWindow::warnOnMissingVDF);
}


MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::showEvent(QShowEvent *event) // hack to show message boxes only after the main window is shown
{
    QMainWindow::showEvent(event);
    QTimer::singleShot(50, this, SLOT(warnOnMissingVDF()));    
}


void MainWindow::closeEvent(QCloseEvent *event)
{
    writeSettings();
    event->accept();
}


void MainWindow::readSettings()
{
    settings->beginGroup("Main");
    isFirstStart = settings->value("FirstStart", true).toBool();
    settings->endGroup();

    settings->beginGroup("WindowGeometry");
    QRect rec = QApplication::desktop()->availableGeometry();
    resize(settings->value("Size", QSize(800, 800)).toSize());
    move(settings->value("Position", QPoint((rec.width()-800)/2, (rec.height()-800)/2)).toPoint());
    settings->endGroup();

    settings->beginGroup("LastSelection");
    steamDir = settings->value("SteamDir", defaultSteamDir).toString();

    if ( QDir(steamDir).exists() )
        ui->labelSteamDirValue->setText(convertSlashes(steamDir));
    else
        ui->labelSteamDirValue->setText("Not found, please locate manually");

    lastSelectedScreenshotDir = settings->value("Screenshots", QDir::currentPath()).toString();
    lastSelectedUserID = settings->value("UserID").toString();
    lastSelectedGameID = settings->value("GameID").toString();

    settings->endGroup();

    settings->beginGroup("Screenshots");
    screenshotPathsPool = settings->value("Queue").toStringList();
    settings->endGroup();
}


void MainWindow::writeSettings()
{
    settings->beginGroup("Main");
    settings->setValue("FirstStart", isFirstStart);
    settings->endGroup();

    settings->beginGroup("WindowGeometry");
    settings->setValue("Size", size());
    settings->setValue("Position", pos());
    settings->endGroup();

    settings->beginGroup("LastSelection");
    settings->setValue("SteamDir", steamDir.replace("\\", "/"));
    settings->setValue("Screenshots", lastSelectedScreenshotDir.replace("\\", "/"));
    if ( !ui->comboBoxUserID->currentText().isEmpty() )
        settings->setValue("UserID", ui->comboBoxUserID->currentText());
    if ( !ui->comboBoxGameID->currentText().isEmpty() )
        settings->setValue("GameID", ui->comboBoxGameID->currentText().remove(QRegularExpression(" <.+>$")));
    settings->endGroup();

    settings->beginGroup("Screenshots");
    settings->setValue("Queue", screenshotPathsPool);
    settings->endGroup();
}


void MainWindow::toggleLabelInfo(bool isVisible)
{
    QList<QLabel*> labelInfoList;
    labelInfoList << ui->labelInfoScreenshots << ui->labelInfo1 << ui->labelInfo2 << ui->labelInfoDirectories;
    foreach (QLabel *label, labelInfoList)
        label->setVisible(isVisible);
}


void MainWindow::populateScreenshotQueue(QStringList screenshotPathsList)
{
    if ( !screenshotPathsList.isEmpty() ) {

        QTreeWidgetItem *item = NULL;
        QListIterator<QString> i(screenshotPathsList);
        while ( i.hasNext() ) {
            QString current = i.next();
            if ( QFile(current).exists() ) {
                item = new QTreeWidgetItem;
                item->setText(0, current.section('/', -1));
                item->setText(1, QFileInfo(QFile(current)).lastModified().toString("yyyy/MM/dd hh:mm:ss"));
                QString path;
                path = convertSlashes(current.section('/', 0, -2));
                item->setText(2, path);
                ui->treeWidgetScreenshotList->addTopLevelItem(item);
            } else
                screenshotPathsPool.removeOne(current);
        };

        ui->treeWidgetScreenshotList->resizeColumnToContents(0);
        ui->treeWidgetScreenshotList->resizeColumnToContents(1);

    };
}


void MainWindow::setUserDataPaths(QString dir)
{
    QStringList userIDsCombined;

    vdfPaths.clear();
    userID.clear();
    someID.clear();
    gameIDs.clear();

    ui->comboBoxUserID->clear();
    ui->comboBoxGameID->clear();
    ui->labelStatusError->clear();

    QList<QWidget*> widgetList;
    widgetList << ui->labelUserID << ui->comboBoxUserID << ui->labelGameID << ui->comboBoxGameID
               << ui->groupBoxScreenshotQueue;

    foreach (QWidget *widget, widgetList)
        widget->setDisabled(true);

    if ( QDir(dir + "/userdata").exists() ) {

        QDirIterator i(userDataDir, QStringList() << vdfFilename, QDir::Files, QDirIterator::Subdirectories);
        while ( i.hasNext() ) {
            vdfPaths << i.next();
        };

        if ( !vdfPaths.isEmpty() ) {
            ui->labelUserID->setDisabled(false);
            ui->comboBoxUserID->setDisabled(false);
            ui->labelGameID->setDisabled(false);
            ui->comboBoxGameID->setDisabled(false);
            ui->groupBoxScreenshotQueue->setDisabled(false);
            ui->treeWidgetScreenshotList->setDisabled(false);
            ui->labelSteamDirValue->setText(convertSlashes(dir));
            steamDir = dir;

            QListIterator<QString> i(vdfPaths);
            while ( i.hasNext() ) {
                QString current = i.next();
                QStringList splitted = current.split('/');
                userID = splitted.takeAt(splitted.length() - 3);
                someID = splitted.takeAt(splitted.length() - 2);
                userIDsCombined << userID + "/" + someID;
            };

            QStringList items;
            if ( isUnixLikeOS )
                items = userIDsCombined;
            else
                items = userIDsCombined.replaceInStrings("/", "\\");
            ui->comboBoxUserID->insertItems(0, items);

            if ( !isFirstStart )
                ui->comboBoxUserID->setCurrentIndex(ui->comboBoxUserID->findText(lastSelectedUserID));

            isFirstStart = false;

            ui->comboBoxGameID->insertItem(0, "loading...");

            QNetworkAccessManager *nam = new QNetworkAccessManager(this);

            QObject::connect(nam, &QNetworkAccessManager::finished,
                             this, &MainWindow::getGameNames);

            nam->get(QNetworkRequest(QUrl("http://api.steampowered.com/ISteamApps/GetAppList/v2")));

        } else {
            ui->labelStatusError->setText("Steam userdata directory is found, but " + vdfFilename + " is missing.");
            ui->labelSteamDirValue->setText("not found");
            ui->labelSteamDirValue->setStyleSheet("color: gray;");
        };

    } else {
        ui->labelStatusError->setText("Steam userdata directory is missing. Please locate correct Steam directory.");
        ui->labelSteamDirValue->setText("not found");
        ui->labelSteamDirValue->setStyleSheet("color: gray;");
    };
}


void MainWindow::warnOnMissingVDF()
{
    if ( vdfPaths.isEmpty() ) {

        QMessageBox msgBox(this);
        msgBox.setIcon(QMessageBox::Warning);

        if ( !QDir(steamDir + "/userdata").exists() ) {
            msgBox.setText("SteaScree has been unable to find a Steam userdata directory in the current location.");
            msgBox.setInformativeText("Please choose an existing Steam directory.");
        } else {
            msgBox.setText("Steam userdata directory is found, but there is no " + vdfFilename);
            msgBox.setInformativeText("Please start Steam, make some screenshots with it and try again.");
        };

        msgBox.exec();

    };
}


void MainWindow::getGameNames(QNetworkReply *reply)
{
    if ( games.isEmpty() ) {

        if ( reply->error() == QNetworkReply::NoError ) {

            QByteArray raw = reply->readAll();
            QJsonDocument doc(QJsonDocument::fromJson(raw));
            QJsonArray apps = doc.object().value("applist").toObject().value("apps").toArray();

            foreach (QJsonValue app, apps) {
                QJsonObject obj = app.toObject();
                QString appID = QString::number(static_cast<int>(obj.value("appid").toDouble()));
                QString name = obj.value("name").toString();
                games[appID] = name;
            };
        };

        selectedUserID = ui->comboBoxUserID->currentText();
        QStringList lines = readVDF();
        int shortcutNamesHeaderPos = lines.indexOf("\t\"shortcutnames\"");
        int shortcutNamesEndPos = lines.indexOf("\t}", shortcutNamesHeaderPos);
        QStringList shortcutNamesSection = lines.mid(shortcutNamesHeaderPos, shortcutNamesEndPos - shortcutNamesHeaderPos);
        QRegularExpression re("^\t\t\"[0-9]+\"\t\t\".+\"$");

        if ( shortcutNamesSection.indexOf(re) != -1 ) {

            int entryPos = 0;

            while ( (entryPos <= shortcutNamesSection.length() - 1) && (entryPos != -1) ) {

                entryPos = shortcutNamesSection.indexOf(re, entryPos + 1);

                if ( entryPos != -1 ) {
                    QString gameID = shortcutNamesSection[entryPos].section("\t\t", 1, 1).remove(QRegularExpression("(^\")|(\"$)"));
                    QString gameName = shortcutNamesSection[entryPos].section("\t\t", 2, 2).remove(QRegularExpression("(^\")|(\"$)"));
                    games[gameID] = gameName;
                };
            };
        };
    };

    QDirIterator i(userDataDir + "/" + userID + "/" + someID + "/remote", QDir::Dirs | QDir::NoDotAndDotDot);
    while ( i.hasNext() ) {

        QString gameID = i.next().section('/', -1);

        if ( !games[gameID].isEmpty() )
            gameIDs << gameID + " <" + games[gameID] + ">";
        else
            gameIDs << gameID;

    };

    ui->comboBoxGameID->clear();

    if ( !gameIDs.isEmpty() )
        ui->comboBoxGameID->insertItems(0, gameIDs);

    if ( !lastSelectedGameID.isEmpty() )
        ui->comboBoxGameID->setCurrentIndex(ui->comboBoxGameID->findText(lastSelectedGameID, Qt::MatchStartsWith));

    ui->pushButtonAddScreenshots->setDisabled(false);
}


QString MainWindow::convertSlashes(QString str)
{
    QString converted;

    if ( isUnixLikeOS )
        converted = str;
    else
        converted = str.replace("/", "\\");

    return converted;
}


QStringList MainWindow::readVDF()
{
    QFile vdf(userDataDir + "/" + selectedUserID + "/" + vdfFilename);
    vdf.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream text(&vdf);
    QStringList lines;

    while ( !text.atEnd() ) {
        QString line = text.readLine();
        lines << line;
    };

    vdf.close();
    return lines;
}


void MainWindow::writeVDF(QStringList lines)
{
    QFile vdf(userDataDir + "/" + selectedUserID + "/" + vdfFilename);
    vdf.open(QIODevice::WriteOnly | QIODevice::Truncate);
    QTextStream text(&vdf);

    QListIterator<QString> i(lines);
    while ( i.hasNext() ) {
        QString current = i.next();
        text << current + "\n";
    };

    vdf.close();
}


void MainWindow::pushScreenshots()
{
    if ( !QDir(userDataDir + "/" + selectedUserID + "/remote/" + selectedGameID + "/screenshots/thumbnails").exists() )
        QDir().mkpath(userDataDir + "/" + selectedUserID + "/remote/" + selectedGameID + "/screenshots/thumbnails");

    QRegularExpression re("^\t\"" + selectedGameID + "\"$");
    int header = lines.indexOf(re, 0);

    int opening, closing;

    if ( header == -1 ) {

        QList<int> headers;

        int pos = 0;
        re.setPattern("^\t\"[0-9]+\"$");
        while ( lines.indexOf(re, pos) != -1 ) {
            pos = lines.indexOf(re, pos + 1);
            if ( pos == -1 )
                break;
            QString h = lines[pos].section('"', 1, -2);
            headers.append(h.toInt());
        };

        int before = -1;

        if ( !headers.isEmpty() ) {

            bool unorderedHeaders = false;

            for ( int i = 0; i < headers.length() - 1; i++ ) {
                if (headers[i] > headers[i + 1])
                    unorderedHeaders = true;
            };

            if ( !unorderedHeaders ) {
                QListIterator<int> i(headers);
                while ( i.hasNext() ) {
                    int current = i.next();
                    if ( selectedGameID.toInt() < current) {
                        before = current;
                        break;
                    };
                };
            };
        };

        int insertPos;
        if ( before != -1 ) {
            QRegularExpression re("^\t\"" + QString::number(before) + "\"$");
            insertPos = lines.indexOf(re, 0);
        } else
            insertPos = lines.indexOf("\t\"shortcutnames\"");

        lines.insert(insertPos, "\t}");
        lines.insert(insertPos, "\t{");
        lines.insert(insertPos, "\t\"" + selectedGameID +"\"");
        header = insertPos;
        opening = insertPos + 1;
        closing = insertPos + 2;

    };

    if ( lines[header + 1] == "\t{" ) {

        opening = header + 1;
        closing = lines.indexOf("\t}", opening);

        if ( (header != -1) & (opening != -1) & (closing != -1) ) {

            int lastEntryPos = -1;
            int lastEntryValue;

            if ( closing - opening != 1 ) {
                QRegularExpression re("^\t\t\"[0-9]+\"$");
                lastEntryPos = lines.lastIndexOf(re, closing);
            } else
                lastEntryValue = -1;

            if ( lastEntryPos == -1 )
                lastEntryValue = -1;

            if ( lastEntryValue != -1 )
                lastEntryValue = lines[lastEntryPos].section('"', 1, -2).toInt();

            QString copyDest = userDataDir + "/" + selectedUserID + "/remote/" + selectedGameID + "/screenshots/";

            if ( !copiedGames.contains(selectedGameID) )
                ui->labelInfoDirectories->setText(QString::number(++copiedDirsToNum));

            nothingAddedToVDF = true;

            QListIterator<QString> i(screenshotPathsPool);
            while ( i.hasNext() ) {

                QString current = i.next();

                // files
                QImage screenshot(current);
                QFile file(current);
                QString extension = current.section('.', -1).toLower();
                QString filename = QFileInfo(file).lastModified().toString("yyyyMMddhhmmss");

                if ( !(QFile(copyDest + filename + "_1.jpg").exists()) ) {

                    if ( (extension == "jpg") | (extension == "jpeg") )
                        file.copy(copyDest + filename + "_1.jpg");
                    else
                        screenshot.save(copyDest + filename + "_1.jpg", "jpg", 95);

                    ui->labelInfoScreenshots->setText(QString::number(++copiedScreenshotsNum));

                };


                int width = QImage(screenshot).size().width();
                int heigth = QImage(screenshot).size().height();

                int tnWidth = 200;
                int tnHeigth = (tnWidth * heigth) / width;

                screenshot.scaled(QSize(tnWidth, tnHeigth), Qt::IgnoreAspectRatio).save(copyDest + "/thumbnails/" +
                                                                                        filename + "_1.jpg", "jpg", 95);
                QDateTime lm = QFileInfo(file).lastModified();
                qint64 epoch = lm.toMSecsSinceEpoch();
                QString creation = QString::number(epoch/1000);


                // vdf
                if ( lines.mid(opening, closing - opening )
                     .contains("\t\t\t\"filename\"\t\t\"" + selectedGameID + "/screenshots/" + filename + "_1.jpg\"") )
                    continue;

                lines.insert(closing++, "\t\t\"" + QString::number(++lastEntryValue) + "\"");
                lines.insert(closing++, "\t\t{");
                lines.insert(closing++, "\t\t\t\"type\"\t\t\"1\"");
                lines.insert(closing++, "\t\t\t\"filename\"\t\t\"" + selectedGameID + "/screenshots/" + filename + "_1.jpg\"");
                lines.insert(closing++, "\t\t\t\"thumbnail\"\t\t\"0\"");
                lines.insert(closing++, "\t\t\t\"vrfilename\"\t\t\"0\"");
                lines.insert(closing++, "\t\t\t\"imported\"\t\t\"0\"");
                lines.insert(closing++, "\t\t\t\"width\"\t\t\"" + QString::number(width) + "\"");
                lines.insert(closing++, "\t\t\t\"heigth\"\t\t\"" + QString::number(heigth) + "\"");
                lines.insert(closing++, "\t\t\t\"gameid\"\t\t\"" + selectedGameID + "\"");
                lines.insert(closing++, "\t\t\t\"creation\"\t\t\"" + creation + "\"");
                lines.insert(closing++, "\t\t\t\"caption\"\t\t\"\"");
                lines.insert(closing++, "\t\t\t\"Permissions\"\t\t\"\"");
                lines.insert(closing++, "\t\t\t\"hscreenshot\"\t\t\"\"");
                lines.insert(closing++, "\t\t}");

                ui->progressBarScreenshotsUploading->setValue(screenshotPathsPool.indexOf(current));

                QTreeWidgetItem *item = ui->treeWidgetScreenshotList->findItems(QFileInfo(file).lastModified()
                                                                                .toString("yyyy/MM/dd hh:mm:ss"), Qt::MatchExactly, 1)[0];
                delete item;

                if ( !copiedGames.contains(selectedGameID) )
                    copiedGames << selectedGameID;

                nothingAddedToVDF = false;

                QCoreApplication::processEvents();

            };
        };
    };
}


void MainWindow::on_pushButtonLocateSteamDir_clicked()
{
    QString steamDirLocated = QFileDialog::getExistingDirectory(this,
                                                         "Locate Steam directory",
                                                         steamDir,
                                                         QFileDialog::ShowDirsOnly | QFileDialog::ReadOnly);
    if ( !steamDirLocated.isEmpty() ) {

        steamDirLocated.remove(QRegularExpression("/userdata$"));
        userDataDir = steamDirLocated + "/userdata";
        setUserDataPaths(steamDirLocated);

    };
}


void MainWindow::on_pushButtonAddScreenshots_clicked()
{
    QStringList screenshotsSelected = QFileDialog::getOpenFileNames(this,
                                                            "Select one or more screenshots",
                                                            lastSelectedScreenshotDir,
                                                            "Images (*.jpg *.jpeg *.png *.bmp *.tif *.tiff)");
    QListIterator<QString> i(screenshotsSelected);
    while ( i.hasNext() ) {
        QString current = i.next();
        if ( screenshotPathsPool.contains(current) )
            screenshotsSelected.removeOne(current);
    };

    if ( !screenshotsSelected.isEmpty() ) {

        lastSelectedScreenshotDir = screenshotsSelected.last().section('/', 0, -2);

        populateScreenshotQueue(screenshotsSelected);

        screenshotPathsPool << screenshotsSelected;
        ui->pushButtonClearQueue->setDisabled(false);
        ui->pushButtonCopyScreenshots->setDisabled(false);

    };
}


void MainWindow::on_pushButtonClearQueue_clicked()
{
    ui->treeWidgetScreenshotList->clear();
    screenshotPathsPool.clear();
    ui->pushButtonClearQueue->setDisabled(true);
    ui->pushButtonCopyScreenshots->setDisabled(true);
    ui->labelUserID->setDisabled(false);
    ui->comboBoxGameID->setDisabled(false);
    ui->labelGameID->setDisabled(false);
    ui->comboBoxUserID->setDisabled(false);
    ui->pushButtonLocateSteamDir->setDisabled(false);
}


void MainWindow::on_pushButtonCopyScreenshots_clicked()
{
    selectedUserID = ui->comboBoxUserID->currentText();
    selectedGameID = ui->comboBoxGameID->currentText();

    QRegularExpression re("^[0-9]+( <.+>)?$");

    if ( !selectedGameID.contains(re) )

        ui->labelStatusError->setText("Invalid Game ID, only numbers allowed");

    else {

        ui->labelStatusError->clear();

        selectedGameID = selectedGameID.remove(QRegularExpression(" <.+>$"));

        if ( screenshotPathsPool.length() >= 10 ) {

            ui->progressBarScreenshotsUploading->setVisible(true);
            ui->progressBarScreenshotsUploading->setMinimum(0);
            ui->progressBarScreenshotsUploading->setMaximum(screenshotPathsPool.length());

        };

        ui->pushButtonClearQueue->setDisabled(true);
        ui->pushButtonAddScreenshots->setDisabled(true);
        ui->pushButtonCopyScreenshots->setDisabled(true);


        toggleLabelInfo(true);

        if ( lines.isEmpty() )
            lines = readVDF();

        pushScreenshots();

        ui->progressBarScreenshotsUploading->setVisible(false);
        ui->pushButtonAddScreenshots->setDisabled(false);
        ui->pushButtonLocateSteamDir->setDisabled(true);
        ui->progressBarScreenshotsUploading->reset();
        ui->treeWidgetScreenshotList->clear();
        screenshotPathsPool.clear();

        ui->pushButtonPrepare->setDisabled(false);

    };
}


void MainWindow::on_pushButtonPrepare_clicked()
{  
    if ( !nothingAddedToVDF ) {

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
            ui->labelInfoScreenshots->setText("0");
            ui->labelInfoDirectories->setText("0");
            copiedScreenshotsNum = 0;
            copiedDirsToNum = 0;
            screenshotPathsPool.clear();
            ui->pushButtonPrepare->setDisabled(true);

            QString vdfPath = userDataDir + "/" + selectedUserID + "/" + vdfFilename;
            QFile(vdfPath).copy(vdfPath + ".bak");

            writeVDF(lines);

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
        copiedScreenshotsNum = 0;
        copiedDirsToNum = 0;
        ui->pushButtonPrepare->setDisabled(true);

    };
}

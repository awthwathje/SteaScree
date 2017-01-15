#include "controller.h"

#include <QFile>
#include <QFileInfo>
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
#include <QObject>
#include <QImageWriter>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonArray>
#include <QSettings>
#include <QRect>
#include <QDesktopWidget>
#include <QApplication>
#include <QtMath>
#include <QVersionNumber>
#include <QDebug>


Controller::Controller(QObject *parent) : QObject(parent)
{    
}


void Controller::bootStrap()
{
    if ( (os == "Linux") || (os == "macOS") ) {
        isUnixLikeOS = true;
        if ( os == "Linux" ) {
            settings = new QSettings(QSettings::NativeFormat, QSettings::UserScope, "Foyl", "SteaScree");
            defaultSteamDir = QDir::homePath() + "/.steam/steam";
        } else {
            settings = new QSettings(QSettings::NativeFormat, QSettings::UserScope, "foyl.io", "SteaScree");
            defaultSteamDir = QDir::homePath() + "/Library/Application Support/Steam";
        }
    } else {
        isUnixLikeOS = false;
        settings = new QSettings(QSettings::IniFormat, QSettings::UserScope, "Foyl", "SteaScree");
        if ( QSysInfo::currentCpuArchitecture() == "x86_64" )
            defaultSteamDir = "C:/Program Files (x86)/Steam";
        else
            defaultSteamDir = "C:/Program Files/Steam";
    }

    readSettings(); // read settings from the file, if any
    if ( offerUpdateSetting != "Never" )
        checkForUpdates();

    if ( !screenshotPathsPool.isEmpty() ) {
        populateScreenshotQueue(screenshotPathsPool);
        emit sendWidgetsDisabled(QStringList() << "pushButton_clearQueue" << "pushButton_copyScreenshots", false);
    }

    if ( !steamDir.isNull() ) {
        userDataDir = steamDir + "/userdata";
        setUserDataPaths(steamDir);
    }
}


void Controller::readSettings()
{
    settings->beginGroup("WindowGeometry");
        QRect rec = QApplication::desktop()->availableGeometry();
        QSize geometry = settings->value("Size", QSize(685, 450)).toSize();
        QPoint moveToPoint = settings->value("Position", QPoint((rec.width()-685)/2, (rec.height()-450)/2)).toPoint();
        emit moveWindow(geometry, moveToPoint);
    settings->endGroup();    

    settings->beginGroup("LastSelection");
        steamDir = settings->value("SteamDir", defaultSteamDir).toString();
        if ( QDir(steamDir).exists() )
            emit sendLabelsText(QStringList() << "label_steamDirValue", convertSlashes(steamDir));
        else
            emit sendLabelsText(QStringList() << "label_steamDirValue", "Not found, please locate manually");
        lastSelectedScreenshotDir = settings->value("Screenshots", QDir::currentPath()).toString();
        lastSelectedUserID = settings->value("UserID").toString();
        lastSelectedGameID = settings->value("GameID").toString();
    settings->endGroup();

    settings->beginGroup("Screenshots");
        screenshotPathsPool = settings->value("Queue").toStringList();
    settings->endGroup();

    settings->beginGroup("Update");
        offerUpdateSetting = settings->value("Offer").toString();
    settings->endGroup();
}


void Controller::writeSettings(QSize size, QPoint pos, QString userID, QString gameID)
{
    settings->beginGroup("WindowGeometry");
        settings->setValue("Size", size);
        settings->setValue("Position", pos);
    settings->endGroup();

    settings->beginGroup("LastSelection");
        settings->setValue("SteamDir", steamDir.replace("\\", "/"));
        settings->setValue("Screenshots", lastSelectedScreenshotDir.replace("\\", "/"));
        if ( !userID.isEmpty() )
            settings->setValue("UserID", userID);
        if ( !gameID.isEmpty() )
            settings->setValue("GameID", gameID);
    settings->endGroup();

    settings->beginGroup("Screenshots");
       settings->setValue("Queue", screenshotPathsPool);
    settings->endGroup();
}


void Controller::checkForUpdates()
{
    QNetworkAccessManager *nam = new QNetworkAccessManager(this);
    QObject::connect(nam, &QNetworkAccessManager::finished,
                     this, &Controller::handleUpdate);
    nam->get(QNetworkRequest(QUrl("https://steascree.download/latest/")));
}


void Controller::handleUpdate(QNetworkReply *reply)
{
    if ( reply->error() == QNetworkReply::NoError ) {

        QByteArray raw = reply->readAll();
        QJsonDocument doc(QJsonDocument::fromJson(raw));
        QJsonObject obj = doc.object();

        QString appVersion = QCoreApplication::applicationVersion();
        QString latestVersion = obj.value("version").toString();
        QVersionNumber appVersionNumber = QVersionNumber::fromString(appVersion);
        QVersionNumber latestVersionNumber = QVersionNumber::fromString(latestVersion);

        if (QVersionNumber::compare(latestVersionNumber, appVersionNumber) > 0) {
            QString link = obj.value(os.toLower()).toString();
            emit sendUpdateInfo(latestVersion, link);
        }
    }
}


void Controller::writeSettingNeverOfferUpdate()
{
    settings->beginGroup("Update");
        settings->setValue("Offer", "Never");
    settings->endGroup();
}


void Controller::setUserDataPaths(QString dir)  // function to validate and set data paths and IDs
{
    userDataDir = dir + "/userdata";
    QStringList userIDsCombined;

    vdfPaths.clear();                           // there may be multiple Steam installations in the system and thus multiple VDFs
    userID.clear();
    someID.clear();
    gameIDs.clear();

    emit sendComboBoxesCleared( QStringList() << "comboBox_userID" << "comboBox_gameID");

    QStringList widgetList;
    widgetList << "label_userID" << "comboBox_userID" << "label_gameID" << "comboBox_gameID" << "groupBox_screenshotQueue";

    emit sendWidgetsDisabled(widgetList, true);

    if ( QDir(dir + "/userdata").exists() ) {

        QDirIterator i(userDataDir, QStringList() << vdfFilename, QDir::Files, QDirIterator::Subdirectories);
        while ( i.hasNext() ) {
            vdfPaths << i.next();
        }

        if ( !vdfPaths.isEmpty() ) {

            emit sendWidgetsDisabled(widgetList << "groupBox_screenshotQueue" << "treeWidget_screenshotList", false);
            emit sendLabelsText(QStringList() << "label_steamDirValue", convertSlashes(dir));
            emit sendLabelsVisible(QStringList() << "label_status", false);

            steamDir = dir;

            QListIterator<QString> i(vdfPaths);
            while ( i.hasNext() ) {
                QString current = i.next();
                QStringList splitted = current.split('/');
                userID = splitted.takeAt(splitted.length() - 3);
                someID = splitted.takeAt(splitted.length() - 2);
                userIDsCombined << userID + "/" + someID;
            }

            QStringList items;
            if ( isUnixLikeOS )
                items = userIDsCombined;
            else
                items = userIDsCombined.replaceInStrings("/", "\\");
            emit sendToComboBox("comboBox_userID", items);

            emit sendToComboBox("comboBox_gameID", QStringList() << "loading...");

            QNetworkAccessManager *nam = new QNetworkAccessManager(this);

            QObject::connect(nam, &QNetworkAccessManager::finished,
                             this, &Controller::getGameNames);

            nam->get(QNetworkRequest(QUrl("http://api.steampowered.com/ISteamApps/GetAppList/v2")));

        } else
            emit sendLabelsOnMissingStuff(false, vdfFilename);

    } else
        emit sendLabelsOnMissingStuff(true, vdfFilename);
}


void Controller::getGameNames(QNetworkReply *reply)
{
    if ( games.isEmpty() ) {
        
        if ( reply->error() == QNetworkReply::NoError ) {

            QByteArray raw = reply->readAll();
            QJsonDocument doc(QJsonDocument::fromJson(raw));
            QJsonArray apps = doc.object().value("applist").toObject().value("apps").toArray();
            
            foreach (QJsonValue app, apps) {
                QJsonObject obj = app.toObject();
                QString appID = QString::number(static_cast<quint32>(obj.value("appid").toDouble()));
                QString name = obj.value("name").toString();
                games[appID] = name;
            }
        }

        emit getComboBoxUserIDCurrentText();

        QStringList lines = readVDF();
        qint32 shortcutNamesHeaderPos = lines.indexOf("\t\"shortcutnames\"");       // if there are any non-Steam games, get names for them too, from the VDF
        qint32 shortcutNamesEndPos = lines.indexOf("\t}", shortcutNamesHeaderPos);
        QStringList shortcutNamesSection = lines.mid(shortcutNamesHeaderPos, shortcutNamesEndPos - shortcutNamesHeaderPos);
        QRegularExpression re("^\t\t\"[0-9]+\"\t\t\".+\"$");

        if ( shortcutNamesSection.indexOf(re) != -1 ) {

            qint32 entryPos = 0;

            while ( (entryPos <= shortcutNamesSection.length() - 1) && (entryPos != -1) ) {

                entryPos = shortcutNamesSection.indexOf(re, entryPos + 1);

                if ( entryPos != -1 ) {
                    QString gameID = shortcutNamesSection[entryPos].section("\t\t", 1, 1).remove(QRegularExpression("(^\")|(\"$)"));
                    QString gameName = shortcutNamesSection[entryPos].section("\t\t", 2, 2).remove(QRegularExpression("(^\")|(\"$)"));
                    games[gameID] = gameName;
                }
            }
        }
    }

    QDirIterator i(userDataDir + "/" + userID + "/" + someID + "/remote", QDir::Dirs | QDir::NoDotAndDotDot);
    while ( i.hasNext() ) {

        QString gameID = i.next().section('/', -1);

        if ( !games[gameID].isEmpty() )
            gameIDs << gameID + " <" + games[gameID] + ">";
        else
            gameIDs << gameID;

    }

    emit sendComboBoxesCleared( QStringList() << "comboBox_gameID" );

    if ( !gameIDs.isEmpty() )
        emit sendToComboBox("comboBox_gameID", gameIDs);

    if ( !lastSelectedGameID.isEmpty() )
        emit sendIndexOfComboBoxGameID(lastSelectedGameID);

    emit sendWidgetsDisabled( QStringList() << "pushButton_addScreenshots", false);
}


void Controller::populateScreenshotQueue(QStringList screenshotPathsList) // function to populate screenshot queue with entries
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
                emit addWidgetItemToScreenshotList(item);
            } else
                screenshotPathsPool.removeOne(current);
        }

        emit resizeScreenshotListColumns();
    }
}


void Controller::addScreenshotsToPool(QStringList screenshotsSelected)
{
    QListIterator<QString> i(screenshotsSelected);
    while ( i.hasNext() ) {

        QString current = i.next();

        if ( screenshotPathsPool.contains(current) )
            screenshotsSelected.removeOne(current); // copies are removed from the list of selected screenshots about to add
    }

    if ( !screenshotsSelected.isEmpty() ) {

        lastSelectedScreenshotDir = screenshotsSelected.last().section('/', 0, -2);
        populateScreenshotQueue(screenshotsSelected);
        screenshotPathsPool << screenshotsSelected;
        emit sendWidgetsDisabled( QStringList() << "pushButton_clearQueue" << "pushButton_copyScreenshots", false );
    }
}


QStringList Controller::readVDF()   // read text from the VDF and return it in the form of list of strings for easy manipulating
{
    QFile vdf(userDataDir + "/" + selectedUserID + "/" + vdfFilename);
    vdf.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream text(&vdf);
    QStringList lines;

    while ( !text.atEnd() ) {
        QString line = text.readLine();
        lines << line;
    }

    vdf.close();
    return lines;
}


void Controller::writeVDF()         // write to VDF from list of strings. previous contents are discarded
{
    QString vdfPath = userDataDir + "/" + selectedUserID + "/" + vdfFilename;
    QFile(vdfPath).copy(vdfPath + ".bak"); // backup VDF just in case

    QFile vdf(vdfPath);
    vdf.open(QIODevice::WriteOnly | QIODevice::Truncate);
    QTextStream text(&vdf);

    QListIterator<QString> i(lines);
    while ( i.hasNext() ) {
        QString current = i.next();
        text << current + "\n";
    }

    vdf.close();
}


void Controller::prepareScreenshots(QString userID, QString gameID, MainWindow *mainWindow)   // this routine copies screenshots to the respective folders
{                                                                   // ...and manipulates a string list copy of the VDF (file is not written yet)
    selectedUserID = userID.replace("\\", "/");
    selectedGameID = gameID.remove(QRegularExpression(" <.+>$"));   // it's possible to enter game ID by hand or left what was auto-generated (with <...>)

    if ( lines.isEmpty() )
        lines = readVDF();

    QString path = userDataDir + "/" + selectedUserID + "/remote/" + selectedGameID + "/screenshots/thumbnails";
    if ( !QDir().exists(path) )
        QDir().mkpath(path);

    QRegularExpression re("^\t\"" + selectedGameID + "\"$");        // calculate a location for inserting new screenshot metadata
    qint32 header = lines.indexOf(re, 0);

    opening = 0;

    if ( header == -1 ) {

        QList<qint32> headers;

        qint32 pos = 0;
        re.setPattern("^\t\"[0-9]+\"$");
        while ( lines.indexOf(re, pos) != -1 ) {
            pos = lines.indexOf(re, pos + 1);
            if ( pos == -1 )
                break;
            QString h = lines[pos].section('"', 1, -2);
            headers.append(h.toInt());
        }

        qint32 before = -1;

        if ( !headers.isEmpty() ) {

            bool unorderedHeaders = false;

            for ( qint32 i = 0; i < headers.length() - 1; i++ ) {
                if (headers[i] > headers[i + 1])
                    unorderedHeaders = true;
            }

            if ( !unorderedHeaders ) {              // normally, all headers in the VDF are ordered, e.g. game ID 1000 is situated before 2000 in the file
                QListIterator<qint32> i(headers);   // ...otherwise VDF is considered dirty and will not be manipulated, to prevent accidental messing things up
                while ( i.hasNext() ) {
                    qint32 current = i.next();
                    if ( selectedGameID.toInt() < current) {
                        before = current;
                        break;
                    }
                }
            }
        }

        quint32 insertPos;
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
    }

    if ( lines[header + 1] == "\t{" ) {

        opening = header + 1;
        closing = lines.indexOf("\t}", opening);

        if ( (header != -1) && (opening != -1) && (closing != -1) ) {

            qint32 lastEntryPos = -1;

            if ( closing - opening != 1 ) {
                QRegularExpression re("^\t\t\"[0-9]+\"$");
                lastEntryPos = lines.lastIndexOf(re, closing);
            } else
                lastEntryValue = -1;

            if ( lastEntryPos == -1 )
                lastEntryValue = -1;

            if ( lastEntryValue != -1 )
                lastEntryValue = lines[lastEntryPos].section('"', 1, -2).toInt();

            // routine to detect timestamp overlaps
            QMap<QString, quint32> repeatingTimestamps;
            QList<QStringList> screenshotPool;
            {
                QListIterator<QString> i(screenshotPathsPool);
                while ( i.hasNext() ) {

                    QString path = i.next();
                    QString timestamp = QFileInfo(QFile(path)).lastModified().toString("yyyyMMddhhmmss");
                    QString filename;
                    quint32 inc = 1;

                    for ( qint32 j = screenshotPathsPool.indexOf(path); j < screenshotPathsPool.length(); ++j ) {

                        QString comparedTimestamp = QFileInfo(QFile(screenshotPathsPool[j])).lastModified().toString("yyyyMMddhhmmss");

                        if ( timestamp == comparedTimestamp ) {

                            if ( !repeatingTimestamps.contains(timestamp) )
                                repeatingTimestamps[timestamp] = 0;
                            else
                                ++repeatingTimestamps[timestamp];   // if timestamp is non-unique for this set, increment the integer in the end of a filename

                            inc = repeatingTimestamps[timestamp] + 1;

                            break;
                        }
                    }

                    filename = timestamp + "_" + QString::number(inc) + ".jpg";
                    screenshotPool << ( QStringList() << path << filename );
                }
            }

            copyDest = userDataDir + "/" + selectedUserID + "/remote/" + selectedGameID + "/screenshots/";
            quint32 id = 0;
            bool thereAreLargeScreenshots = false;

            emit sendProgressBarLength(screenshotPool.length());

            QListIterator<QStringList> i(screenshotPool);
            while ( i.hasNext() ) {

                QStringList current = i.next();
                QImage image(current[0]);
                quint32 width = QImage(image).size().width();
                quint32 height = QImage(image).size().height();

                QList<quint32> geometry = QList<quint32>() << width << height;

                if ( (width > steamMaxSideSize) || (height > steamMaxSideSize) || ((width * height) > steamMaxResolution) ) {
                    QPixmap thumbnail = QPixmap::fromImage(image).scaled(320, 180, Qt::KeepAspectRatio);
                    preparedScreenshotList << Screenshot({id, current, thumbnail, geometry, true, 0});
                    thereAreLargeScreenshots = true;
                }
                else {
                    preparedScreenshotList << Screenshot({id, current, QPixmap(), geometry, false, 0});
                }

                emit sendProgressBarValue(id + 1);
                id++;
                QCoreApplication::processEvents();
            }

            emit sendLabelsVisible(QStringList() << "label_progress" << "label_status" << "progressBar_status", false);

            if (thereAreLargeScreenshots) {
                emit sendLabelsVisible(QStringList() << "label_progress" << "label_status", true);
                emit sendStatusLabelText("waiting for user decision", "");
                getUserDecisionAboutLargeScreenshots(preparedScreenshotList, mainWindow);
            }
            else
                pushScreenshots(preparedScreenshotList);
        }
    }
}


void Controller::getUserDecisionAboutLargeScreenshots(QList<Screenshot> screenshotList, MainWindow *mainWindow)
{
    QList<Screenshot> largeScreenshotList;

    QListIterator<Screenshot> i(screenshotList);
    while ( i.hasNext() ) {

        Screenshot current = i.next();

        if (current.isLarge) {
            largeScreenshotList << current;
        }
    }

    if (largeScreenshotList.length() > 0) {

        largeFileDialog = new LargeFileDialog(mainWindow);

        // set some connections here beforehand, because it will be harder to do in the window's own object
        QObject::connect(this,              &Controller::sendScreenshotList,
                         largeFileDialog,   &LargeFileDialog::getDecisions);

        QObject::connect(largeFileDialog,   &LargeFileDialog::sendButtonList,
                         this,              &Controller::getButtonList);

        QObject::connect(largeFileDialog,   &LargeFileDialog::returnScreenshotListWithDecisions,
                         this,              &Controller::prepareScreenshotListWithDecisions);

        emit sendScreenshotList(largeScreenshotList, mainWindow->frameGeometry().center(),
                                QStringList() << QString::number(steamMaxSideSize) << QString::number(steamMaxResolution));
    }
}


void Controller::prepareScreenshotListWithDecisions(QList<Screenshot> screenshotList)
{
    emit sendLabelsVisible(QStringList() << "label_progress" << "label_status", false);

    QListIterator<Screenshot> i(screenshotList);
    while ( i.hasNext() ) {

        Screenshot current = i.next();

        for (qint32 j=0; j < preparedScreenshotList.length(); j++)
            if (current.id == preparedScreenshotList[j].id)
                preparedScreenshotList[j].decision = current.decision;
    }

    pushScreenshots(preparedScreenshotList);
}


void Controller::resizeAndSaveLargeScreenshot(Screenshot screenshot)
{
    QString filename = screenshot.screenshot[1];
    QImage image(screenshot.screenshot[0]);
    QImage resizedImage;
    quint32 width = screenshot.geometry[0];
    quint32 height = screenshot.geometry[1];

    bool rectangular = true;
    quint32 newWidth;
    quint32 newHeight;

    if (width == height) {                                                      // square, large side

        newWidth = qRound(qSqrt(steamMaxResolution));
        newHeight = newWidth;
        rectangular = false;

    } else if ( (width > steamMaxSideSize) || (height > steamMaxSideSize) ) {   // rectangle, large side(s)

        if (width > height) {   // horizontal
            newWidth = steamMaxSideSize;
            newHeight = (steamMaxSideSize * height) / width;
        } else {                // vertical
            newHeight = steamMaxSideSize;
            newWidth = (steamMaxSideSize * width) / height;
        }

        width = newWidth;
        height = newHeight;

    }

    if (rectangular && (width * height > steamMaxResolution)) {                 // rectangle, large resolution

        qreal ratio = width / qreal(height);
        newHeight = qFloor(qSqrt(steamMaxResolution / ratio));
        newWidth = qFloor(newHeight * ratio);

    }

    saveThumbnail(filename, image, newWidth, newHeight);

    resizedImage = image.scaled(QSize(newWidth, newHeight), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    resizedImage.save(copyDest + filename, "jpg", 95);
}


void Controller::saveThumbnail(QString filename, QImage image, quint32 width, quint32 height)
{
    quint32 tnWidth = 200;
    quint32 tnHeight = (tnWidth * height) / width;

    image.scaled(QSize(tnWidth, tnHeight), Qt::IgnoreAspectRatio, Qt::SmoothTransformation)
            .save(copyDest + "/thumbnails/" + filename, "jpg", 95);
}


void Controller::pushScreenshots(QList<Screenshot> screenshotList)
{
    emit sendProgressBarLength(screenshotList.length());
    emit sendLabelsVisible(QStringList() << "label_progress" << "label_status" << "progressBar_status", true);
    emit sendStatusLabelText("copying screenshots", "");

    QListIterator<Screenshot> i(screenshotList);
    while ( i.hasNext() ) {

        Screenshot current = i.next();

        QString path = current.screenshot[0];

        if (current.isLarge && current.decision == 2) {
            emit deleteCopiedWidgetItem(path);
            continue;                                               // skip file if it is large and user decided not to upload it
        }

        QString filename = current.screenshot[1];

        QFile file(path);
        QString extension = path.section('.', -1).toLower();

        quint32 width = current.geometry[0];
        quint32 height = current.geometry[1];

        QImage image(path);

        if ( !(QFile(copyDest + filename).exists()) ) {

            if (!current.isLarge || current.decision == 3) {

                if ( (extension == "jpg") || (extension == "jpeg") )
                    file.copy(copyDest + filename);                 // copy file if it is not large or if user decided to try it upload anyway
                else
                    image.save(copyDest + filename, "jpg", 95);

                saveThumbnail(filename, image, width, height);

            } else                                                  // it is large and the user decision is "resize"
                resizeAndSaveLargeScreenshot(current);            

            emit sendLabelsText( QStringList() << "label_infoScreenshots", QString::number(++copiedScreenshotsNum) );
        }

        QDateTime lm = QFileInfo(file).lastModified();
        qint64 epoch = lm.toMSecsSinceEpoch();
        QString creation = QString::number(epoch/1000);

        // vdf data preparation
        if ( lines.mid( opening, closing - opening )
             .contains("\t\t\t\"filename\"\t\t\"" + selectedGameID + "/screenshots/" + filename + "\"") ) {
            emit deleteCopiedWidgetItem(path);
            continue;
        }

        lines.insert(closing++, "\t\t\"" + QString::number(++lastEntryValue) + "\"");
        lines.insert(closing++, "\t\t{");
        lines.insert(closing++, "\t\t\t\"type\"\t\t\"1\"");
        lines.insert(closing++, "\t\t\t\"filename\"\t\t\"" + selectedGameID + "/screenshots/" + filename + "\"");
        lines.insert(closing++, "\t\t\t\"thumbnail\"\t\t\"" + selectedGameID + "/screenshots/thumbnails/" + filename + "\"");
        lines.insert(closing++, "\t\t\t\"vrfilename\"\t\t\"\"");
        lines.insert(closing++, "\t\t\t\"imported\"\t\t\"0\"");
        lines.insert(closing++, "\t\t\t\"width\"\t\t\"" + QString::number(width) + "\"");
        lines.insert(closing++, "\t\t\t\"height\"\t\t\"" + QString::number(height) + "\"");
        lines.insert(closing++, "\t\t\t\"gameid\"\t\t\"" + selectedGameID + "\"");
        lines.insert(closing++, "\t\t\t\"creation\"\t\t\"" + creation + "\"");
        lines.insert(closing++, "\t\t\t\"caption\"\t\t\"\"");
        lines.insert(closing++, "\t\t\t\"Permissions\"\t\t\"\"");
        lines.insert(closing++, "\t\t\t\"hscreenshot\"\t\t\"\"");
        lines.insert(closing++, "\t\t}");

        addedLines++;

        emit sendProgressBarValue(screenshotPathsPool.indexOf(path) + 1);
        emit deleteCopiedWidgetItem(path);

        if ( !copiedGames.contains(selectedGameID) ) {
            copiedGames << selectedGameID;
            copiedDirsToNum++;
        }

        QCoreApplication::processEvents();
    }

    emit sendLabelsVisible(QStringList() << "label_progress" << "progressBar_status", false);
    emit sendWidgetsDisabled(QStringList() << "pushButton_addScreenshots" << "comboBox_gameID", false);
    emit sendLabelsCleared(QStringList() << "label_status");

    if (addedLines > 0) {
        emit sendLabelsText(QStringList() << "label_infoDirectories", QString::number(copiedDirsToNum));
        emit sendDirStatusLabelsVisible(true);
        emit sendStatusLabelText("screenshots are ready for preparation", "gray");
        emit sendWidgetsDisabled(QStringList() << "pushButton_prepare", false);
    }

    screenshotPathsPool.clear();
    preparedScreenshotList.clear();
}


void Controller::getButtonList(QList<QPushButton*> buttonList)
{
    emit adjustButtons(buttonList, os);
}


void Controller::removeEntryFromScreenshotPathsPool(QString entry)
{
    screenshotPathsPool.removeOne(entry);
}


void Controller::setSelectedUserID(QString text)
{
    selectedUserID = text;
}


QString Controller::convertSlashes(QString str)
{
    QString converted;

    if ( isUnixLikeOS )
        converted = str;
    else
        converted = str.replace("/", "\\");

    return converted;
}


void Controller::returnLinesState()
{
    emit sendLinesState(addedLines);
}


void Controller::clearScreenshotPathsPool()
{
    screenshotPathsPool.clear();
}


void Controller::clearState()
{
    addedLines = 0;
    lines.clear();
    copiedGames.clear();
}


void Controller::clearCopyingStatusLabels()
{
    copiedScreenshotsNum = 0;
    copiedDirsToNum = 0;
}


void Controller::returnLastSelectedScreenshotDir()
{
    emit sendLastSelectedScreenshotDir(lastSelectedScreenshotDir);
}


void Controller::returnScreenshotPathPoolLength()
{
    emit sendProgressBarLength(screenshotPathsPool.length());
}


void Controller::returnSteamDir()
{
    emit sendSteamDir(steamDir);
}


void Controller::setSelectedIDs(QString userID, QString gameID)
{
    selectedUserID = userID;
    selectedGameID = gameID;
}

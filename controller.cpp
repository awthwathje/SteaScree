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


Controller::Controller(QObject *parent) : QObject(parent)
{    
}


void Controller::bootStrap()
{
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

    readSettings(); // read settings from the file, if any

    if ( !screenshotPathsPool.isEmpty() ) {
        populateScreenshotQueue(screenshotPathsPool);
        emit disableWidgets(QStringList() << "pushButtonClearQueue" << "pushButtonCopyScreenshots", false);
    };

    if ( !steamDir.isNull() ) {
        userDataDir = steamDir + "/userdata";
        setUserDataPaths(steamDir);
    };
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
            emit sendLabelsText(QStringList() << "labelSteamDirValue", convertSlashes(steamDir));
        else
            emit sendLabelsText(QStringList() << "labelSteamDirValue", "Not found, please locate manually");
        lastSelectedScreenshotDir = settings->value("Screenshots", QDir::currentPath()).toString();
        lastSelectedUserID = settings->value("UserID").toString();
        lastSelectedGameID = settings->value("GameID").toString();
    settings->endGroup();

    settings->beginGroup("Screenshots");
        screenshotPathsPool = settings->value("Queue").toStringList();
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


void Controller::setUserDataPaths(QString dir) // function to validate and set data paths and IDs
{
    userDataDir = dir + "/userdata";
    QStringList userIDsCombined;

    vdfPaths.clear(); // there may be multiple Steam installations in the system and thus multiple VDFs
    userID.clear();
    someID.clear();
    gameIDs.clear();

    emit clearWidgets( QStringList() << "comboBoxUserID" << "comboBoxGameID");

    QStringList widgetList;
    widgetList << "labelUserID" << "comboBoxUserID" << "labelGameID" << "comboBoxGameID" << "groupBoxScreenshotQueue";

    emit disableWidgets(widgetList, true);

    if ( QDir(dir + "/userdata").exists() ) {

        QDirIterator i(userDataDir, QStringList() << vdfFilename, QDir::Files, QDirIterator::Subdirectories);
        while ( i.hasNext() ) {
            vdfPaths << i.next();
        };

        if ( !vdfPaths.isEmpty() ) {

            emit disableWidgets(widgetList << "groupBoxScreenshotQueue" << "treeWidgetScreenshotList", false);
            emit sendLabelsText(QStringList() << "labelSteamDirValue", convertSlashes(dir));
            emit setLabelStatusErrorVisible(false);

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
            emit sendToComboBox("comboBoxUserID", items);

            emit sendToComboBox("comboBoxGameID", QStringList() << "loading...");

            QNetworkAccessManager *nam = new QNetworkAccessManager(this);

            QObject::connect(nam, &QNetworkAccessManager::finished,
                             this, &Controller::getGameNames);

            nam->get(QNetworkRequest(QUrl("http://api.steampowered.com/ISteamApps/GetAppList/v2")));

        } else
            emit setLabelsOnMissingStuff(false, vdfFilename);

    } else
        emit setLabelsOnMissingStuff(true, vdfFilename);
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
                QString appID = QString::number(static_cast<int>(obj.value("appid").toDouble()));
                QString name = obj.value("name").toString();
                games[appID] = name;
            };
        };

        emit getComboBoxUserIDCurrentText();

        QStringList lines = readVDF();
        int shortcutNamesHeaderPos = lines.indexOf("\t\"shortcutnames\""); // if there are any non-Steam games, get names for them too, from the VDF
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

    emit clearWidgets( QStringList() << "comboBoxGameID" );

    if ( !gameIDs.isEmpty() )
        emit sendToComboBox("comboBoxGameID", gameIDs);

    if ( !lastSelectedGameID.isEmpty() )
        emit setIndexOfComboBoxGameID(lastSelectedGameID);

    emit disableWidgets( QStringList() << "pushButtonAddScreenshots", false);
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
        };

        emit resizeScreenshotListColumns();
    };
}


void Controller::addScreenshotsToPool(QStringList screenshotsSelected)
{
    QListIterator<QString> i(screenshotsSelected);
    while ( i.hasNext() ) {

        QString current = i.next();

        if ( screenshotPathsPool.contains(current) )
            screenshotsSelected.removeOne(current); // copies are removed from the list

    };

    if ( !screenshotsSelected.isEmpty() ) {

        lastSelectedScreenshotDir = screenshotsSelected.last().section('/', 0, -2);
        populateScreenshotQueue(screenshotsSelected);
        screenshotPathsPool << screenshotsSelected;
        emit disableWidgets( QStringList() << "pushButtonClearQueue" << "pushButtonCopyScreenshots", false );
    };
}


QStringList Controller::readVDF() // read text from the VDF and return it in the form of list of strings for easy manipulating
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


void Controller::writeVDF() // write to VDF from list of strings. previous contents are discarded
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
    };

    vdf.close();
}


void Controller::pushScreenshots(QString userID, QString gameID) // this routine copies screenshots to the respective folders and manipulates a string list copy of the VDF. VDF is not written
{
    selectedUserID = userID.replace("\\", "/");
    selectedGameID = gameID.remove(QRegularExpression(" <.+>$")); // it's possible to enter game ID by hand or left what was auto-generated (with <...>)

    if ( lines.isEmpty() )
        lines = readVDF();

    QString path = userDataDir + "/" + selectedUserID + "/remote/" + selectedGameID + "/screenshots/thumbnails";
    if ( !QDir().exists(path) )
        QDir().mkpath(path);

    QRegularExpression re("^\t\"" + selectedGameID + "\"$"); // calculate a location for inserting new screenshot metadata
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

            if ( !unorderedHeaders ) { // proceed only if all headers in the VDF are ordered, e.g. game ID 1000 is before 2000
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
                emit sendLabelsText(QStringList() << "labelInfoDirectories", QString::number(++copiedDirsToNum));

            // routine to detect timestamp overlapping
            QMap<QString, int> repeatingTimestamps;
            QList<QStringList> screenshotPool;
            {
                QListIterator<QString> i(screenshotPathsPool);
                while ( i.hasNext() ) {

                    QString path = i.next();
                    QString timestamp = QFileInfo(QFile(path)).lastModified().toString("yyyyMMddhhmmss");
                    QString filename;
                    int inc = 1;

                    for ( int j = screenshotPathsPool.indexOf(path); j < screenshotPathsPool.length(); ++j ) {

                        QString comparedTimestamp = QFileInfo(QFile(screenshotPathsPool[j])).lastModified().toString("yyyyMMddhhmmss");

                        if ( timestamp == comparedTimestamp ) {

                            if ( !repeatingTimestamps.contains(timestamp) )
                                repeatingTimestamps[timestamp] = 0;
                            else
                                ++repeatingTimestamps[timestamp]; // if timestamp is non-unique for this set, increment the int in the end of a filename

                            inc = repeatingTimestamps[timestamp] + 1;

                            break;
                        };
                    };

                    filename = timestamp + "_" + QString::number(inc) + ".jpg";
                    screenshotPool << ( QStringList() << path << filename );
                };
            }

            QListIterator<QStringList> i(screenshotPool); // when insertion location is determined, proceed to the insertion and file conversion/copying
            while ( i.hasNext() ) {

                QStringList current = i.next();
                QString path = current[0];
                QString filename = current[1];

                // files
                QImage screenshot(path);
                QFile file(path);
                QString extension = path.section('.', -1).toLower();

                if ( !(QFile(copyDest + filename).exists()) ) {

                    if ( (extension == "jpg") | (extension == "jpeg") )
                        file.copy(copyDest + filename);
                    else
                        screenshot.save(copyDest + filename, "jpg", 95);

                    emit sendLabelsText(QStringList() << "labelInfoScreenshots", QString::number(++copiedScreenshotsNum));

                };

                int width = QImage(screenshot).size().width();
                int heigth = QImage(screenshot).size().height();

                int tnWidth = 200;
                int tnHeigth = (tnWidth * heigth) / width;

                screenshot.scaled(QSize(tnWidth, tnHeigth), Qt::IgnoreAspectRatio, Qt::SmoothTransformation).save(copyDest + "/thumbnails/" +
                                                                                        filename, "jpg", 95);
                QDateTime lm = QFileInfo(file).lastModified();
                qint64 epoch = lm.toMSecsSinceEpoch();
                QString creation = QString::number(epoch/1000);

                // vdf
                if ( lines.mid(opening, closing - opening )
                     .contains("\t\t\t\"filename\"\t\t\"" + selectedGameID + "/screenshots/" + filename + "\"") )
                    continue;

                lines.insert(closing++, "\t\t\"" + QString::number(++lastEntryValue) + "\"");
                lines.insert(closing++, "\t\t{");
                lines.insert(closing++, "\t\t\t\"type\"\t\t\"1\"");
                lines.insert(closing++, "\t\t\t\"filename\"\t\t\"" + selectedGameID + "/screenshots/" + filename + "\"");
                lines.insert(closing++, "\t\t\t\"thumbnail\"\t\t\"" + selectedGameID + "/screenshots/thumbnails/" + filename + "\"");
                lines.insert(closing++, "\t\t\t\"vrfilename\"\t\t\"\"");
                lines.insert(closing++, "\t\t\t\"imported\"\t\t\"0\"");
                lines.insert(closing++, "\t\t\t\"width\"\t\t\"" + QString::number(width) + "\"");
                lines.insert(closing++, "\t\t\t\"heigth\"\t\t\"" + QString::number(heigth) + "\"");
                lines.insert(closing++, "\t\t\t\"gameid\"\t\t\"" + selectedGameID + "\"");
                lines.insert(closing++, "\t\t\t\"creation\"\t\t\"" + creation + "\"");
                lines.insert(closing++, "\t\t\t\"caption\"\t\t\"\"");
                lines.insert(closing++, "\t\t\t\"Permissions\"\t\t\"\"");
                lines.insert(closing++, "\t\t\t\"hscreenshot\"\t\t\"\"");
                lines.insert(closing++, "\t\t}");

                addedLines++;

                emit setProgressBarValue(screenshotPathsPool.indexOf(path));
                emit deleteCopiedWidgetItem(path);

                if ( !copiedGames.contains(selectedGameID) )
                    copiedGames << selectedGameID;

                QCoreApplication::processEvents();
            };
        };
    };
}


void Controller::returnOS()
{
    emit sendOS(os);
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


void Controller::returnVDFStatus()
{
    if ( vdfPaths.isEmpty() )
        emit sendVDFStatus(QDir(steamDir + "/userdata").exists(), vdfFilename);
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
    if ( screenshotPathsPool.length() >= 10 )
        emit sendScreenshotPathPoolLength(screenshotPathsPool.length());
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


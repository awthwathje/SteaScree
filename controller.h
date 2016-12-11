#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <QObject>
#include <QSettings>
#include <QTreeWidgetItem>
#include <QNetworkReply>

class Controller : public QObject
{
    Q_OBJECT

public:
    explicit Controller(QObject *parent = 0);
    void bootStrap();


private:
    void readSettings();
    QString convertSlashes(QString str);
    QStringList readVDF();
    void populateScreenshotQueue(QStringList screenshotPathsList);

    bool isUnixLikeOS;
    const QString vdfFilename = "screenshots.vdf";
    QString selectedUserID;
    QString selectedGameID;
    QString userDataDir;
    QString defaultSteamDir;
    QSettings *settings;
    QString steamDir;
    QStringList vdfPaths;
    QString userID;
    QString someID;
    QStringList gameIDs;
    QHash<QString, QString> games;
    QStringList screenshotPathsPool;
    QStringList lines;
    QString lastSelectedScreenshotDir;
    QString lastSelectedUserID;
    QString lastSelectedGameID;
    QStringList copiedGames;
    int lastEntryValue;
    int copiedScreenshotsNum = 0;
    int copiedDirsToNum = 0;
    int addedLines = 0;

    #if defined(Q_OS_WIN32)
    const QString os = "Windows";
    #elif defined(Q_OS_LINUX)
    const QString os = "Linux";
    #elif defined(Q_OS_OSX)
    const QString os = "macOS";
    #endif


signals:
    void sendOS(QString);
    void addWidgetItemToScreenshotList(QTreeWidgetItem *item);
    void resizeScreenshotListColumns();
    void sendScreenshotPathPoolLength(int length);
    void sendSteamDir(QString steamDir);
    void sendLinesState(bool isVDFEmpty);
    void sendVDFStatus(bool userDataExists, QString vdfFilename);
    void moveWindow(QSize geometry, QPoint moveToPoint);
    void setLabelStatusErrorVisible(bool visible);
    void disableWidgets(QStringList list, bool disable);
    void clearWidgets(QStringList list);
    void setLabelsOnMissingStuff(bool userDataMissing, QString vdfFilename);
    void getComboBoxUserIDCurrentText();
    void sendLastSelectedScreenshotDir(QString lastSelectedScreenshotDir);
    void setProgressBarValue(int value);
    void deleteCopiedWidgetItem(QString path);
    void sendToComboBox(QString name, QStringList items);
    void setIndexOfComboBoxGameID(QString lastSelectedGameID);
    void sendLabelsText(QStringList list, QString text);


public slots:
    void returnOS();
    void writeSettings(QSize size, QPoint pos, QString userID, QString gameID);
    void removeEntryFromScreenshotPathsPool(QString entry);
    void returnLastSelectedScreenshotDir();
    void clearScreenshotPathsPool();
    void clearState();
    void returnScreenshotPathPoolLength();
    void pushScreenshots(QString userID, QString gameID);
    void setUserDataPaths(QString dir);
    void returnSteamDir();
    void writeVDF();
    void returnLinesState();
    void clearCopyingStatusLabels();
    void returnVDFStatus();
    void setSelectedUserID(QString text);
    void addScreenshotsToPool(QStringList screenshotsSelected);
    void setSelectedIDs(QString userID, QString gameID);


private slots:
    void getGameNames(QNetworkReply *reply);

};

#endif // CONTROLLER_H

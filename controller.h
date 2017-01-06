#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "largefiledialog.h"
#include "mainwindow.h"
#include "screenshot.h"

#include <QObject>
#include <QSettings>
#include <QTreeWidgetItem>
#include <QNetworkReply>
#include <QPushButton>


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
    QString copyDest;
    qint32 opening;
    qint32 closing;
    qint32 lastEntryValue;
    quint32 copiedScreenshotsNum = 0;
    quint32 copiedDirsToNum = 0;
    quint32 addedLines = 0;
    LargeFileDialog *largeFileDialog;
    QList<Screenshot> preparedScreenshotList;
    const quint32 steamMaxSideSize = 16000;
    const quint32 steamMaxResolution = 26210175;

    #if defined(Q_OS_WIN32)
    const QString os = "Windows";
    #elif defined(Q_OS_LINUX)
    const QString os = "Linux";
    #elif defined(Q_OS_OSX)
    const QString os = "macOS";
    #endif

    void pushScreenshots(QList<Screenshot> screenshotList);
    void resizeAndSaveLargeScreenshot(Screenshot screenshot);
    void getUserDecisionAboutLargeScreenshots(QList<Screenshot> screenshotList, MainWindow *mainWindow);
    void saveThumbnail(QString filename, QImage image, quint32 width, quint32 height);


signals:
    void adjustButtons(QList<QPushButton*> buttonList, QString os);
    void addWidgetItemToScreenshotList(QTreeWidgetItem *item);
    void resizeScreenshotListColumns();
    void sendProgressBarLength(quint32 length);
    void sendSteamDir(QString steamDir);
    void sendLinesState(quint32 addedLines);
    void sendVDFStatus(bool userDataExists, QString vdfFilename);
    void moveWindow(QSize geometry, QPoint moveToPoint);
    void setLabelStatusErrorVisible(bool visible);
    void sendWidgetsDisabled(QStringList list, bool disable);
    void sendLabelsCleared(QStringList list);
    void sendLabelsVisible(QStringList list, bool visible);
    void sendComboBoxesCleared(QStringList list);
    void sendLabelsOnMissingStuff(bool userDataMissing, QString vdfFilename);
    void getComboBoxUserIDCurrentText();
    void sendLastSelectedScreenshotDir(QString lastSelectedScreenshotDir);
    void sendProgressBarValue(quint32 value);
    void deleteCopiedWidgetItem(QString path);
    void sendToComboBox(QString name, QStringList items);
    void sendIndexOfComboBoxGameID(QString lastSelectedGameID);
    void sendLabelsText(QStringList list, QString text);
    void sendScreenshotList(QList<Screenshot> screenshotList, QPoint center, QStringList steamLimits);
    void sendStatusLabelText(QString text, QString color);
    void setupStatusArea(quint32 progressBarMaximum);
    void sendDirStatusLabelsVisible(bool visible);


public slots:
    void getButtonList(QList<QPushButton *> buttonList);
    void writeSettings(QSize size, QPoint pos, QString userID, QString gameID);
    void removeEntryFromScreenshotPathsPool(QString entry);
    void returnLastSelectedScreenshotDir();
    void clearScreenshotPathsPool();
    void clearState();
    void returnScreenshotPathPoolLength();
    void prepareScreenshots(QString userID, QString gameID, MainWindow *mainWindow);
    void setUserDataPaths(QString dir);
    void returnSteamDir();
    void writeVDF();
    void returnLinesState();
    void clearCopyingStatusLabels();
    void returnVDFStatus();
    void setSelectedUserID(QString text);
    void addScreenshotsToPool(QStringList screenshotsSelected);
    void setSelectedIDs(QString userID, QString gameID);
    void prepareScreenshotListWithDecisions(QList<Screenshot> screenshotList);


private slots:
    void getGameNames(QNetworkReply *reply);
};

#endif // CONTROLLER_H

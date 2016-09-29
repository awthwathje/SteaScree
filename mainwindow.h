#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QNetworkReply>
#include <QSettings>


namespace Ui {
class MainWindow;
}


class MainWindow : public QMainWindow
{
    Q_OBJECT


public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();


protected:
    void showEvent(QShowEvent *event) Q_DECL_OVERRIDE;
    void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;


private slots:
    void warnOnMissingVDF();
    void getGameNames(QNetworkReply *reply);
    void on_pushButtonLocateSteamDir_clicked();
    void on_pushButtonAddScreenshots_clicked();
    void on_pushButtonClearQueue_clicked();
    void on_pushButtonCopyScreenshots_clicked();
    void on_pushButtonPrepare_clicked();


signals:
    void vdfIsMissing();


private:
    Ui::MainWindow *ui;
    void setUserDataPaths(QString dir);
    void writeSettings();
    void readSettings();
    void populateScreenshotQueue(QStringList screenshotPathsList);
    void disableAllOnMissingSteamDir();
    void pushScreenshots();
    void toggleLabelInfo(bool isVisible);
    QString convertSlashes(QString str);
    QStringList readVDF();
    void writeVDF(QStringList lines);

    #if defined(Q_OS_WIN32)
    const QString os = "Windows";
    #elif defined(Q_OS_LINUX)
    const QString os = "Linux";
    #elif defined(Q_OS_OSX)
    const QString os = "macOS";
    #endif

    bool isUnixLikeOS;
    bool isFirstStart;
    const QString vdfFilename = "screenshots.vdf";
    QString selectedUserID;
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
    QString selectedGameID;
    QString lastSelectedScreenshotDir;
    QString lastSelectedUserID;
    QString lastSelectedGameID;
    QStringList copiedGames;
    bool nothingAddedToVDF = true;
    int copiedScreenshotsNum = 0;
    int copiedDirsToNum = 0;
};

#endif // MAINWINDOW_H

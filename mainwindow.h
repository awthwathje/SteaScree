#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTreeWidgetItem>


namespace Ui {
class MainWindow;
}


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void bootStrap();


protected:
    void showEvent(QShowEvent *event) Q_DECL_OVERRIDE;
    void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;


private:
    Ui::MainWindow *ui;
    void toggleLabelInfo(bool isVisible);


signals:
    void getOS();
    void pushButtonAddScreenshots_clicked();
    void pushButtonPrepare_clicked();
    void clearScreenshotPathsPool();
    void clearState();
    void getScreenshotPathsPoolLength();
    void pushScreenshots(QString selectedUserID, QString selectedGameID);
    void getSteamDir();
    void setUserDataPaths(QString steamDir);
    void clearCopyingStatusLabels();
    void writeVDF();
    void getVDFStatus();
    void sendSettings(QSize size, QPoint pos, QString userID, QString gameID);
    void sendComboBoxUserIDCurrentText(QString text);
    void sendScreenshotsSelected(QStringList screenshotsSelected);


public slots:
    void setButtonPadding(QString os);
    void addWidgetItemToScreenshotList(QTreeWidgetItem *item);
    void resizeScreenshotListColumns();
    void setWidgetsDisabled(QStringList list, bool disable);
    void setVisibleProgressBar(int length);
    void locateSteamDir(QString steamDir);
    void prepareScreenshots(int addedLines);
    void warnOnMissingVDF(bool userDataExists, QString vdfFilename);
    void moveWindow(QSize geometry, QPoint moveToPoint);
    void setLabelStatusErrorVisible(bool visible);
    void setComboBoxesCleared(QStringList list);
    void insertIntoComboBox(QString name, QStringList items);
    void setLabelsOnMissingStuff(bool userDataMissing, QString vdfFilename);
    void returnComboBoxUserIDCurrentText();
    void returnScreenshotsSelected(QString lastSelectedScreenshotDir);
    void setProgressBarValue(int value);
    void deleteCopiedWidgetItem(QString path);
    void setIndexOfComboBoxGameID(QString lastSelectedGameID);
    void setLabelsText(QStringList list, QString text);


private slots:
    void on_pushButtonAddScreenshots_clicked();
    void on_pushButtonClearQueue_clicked();
    void on_pushButtonCopyScreenshots_clicked();
    void on_pushButtonPrepare_clicked();
    void on_pushButtonLocateSteamDir_clicked();
    void checkVDF();


};

#endif // MAINWINDOW_H

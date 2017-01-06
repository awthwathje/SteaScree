#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTreeWidgetItem>
#include <QMessageBox>
#include <QPushButton>
#include <QMovie>


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
    QMovie *gifLoader = new QMovie("://res/misc/loader.gif");
    void makeWideMessageBox(QMessageBox *msgBox, quint32 width);
    void disableAllControls();


signals:
    void sendButtonList(QList<QPushButton*> buttonList);
    void pushButton_addScreenshots_clicked();
    void pushButton_prepare_clicked();
    void clearScreenshotPathsPool();
    void clearState();
    void getScreenshotPathsPoolLength();
    void sendSelectedIDs(QString selectedUserID, QString selectedGameID, MainWindow *mainWindow);
    void getSteamDir();
    void sendUserDataPaths(QString steamDir);
    void clearCopyingStatusLabels();
    void writeVDF();
    void getVDFStatus();
    void sendSettings(QSize size, QPoint pos, QString userID, QString gameID);
    void sendComboBoxUserIDCurrentText(QString text);
    void sendScreenshotsSelected(QStringList screenshotsSelected);


public slots:
    void addWidgetItemToScreenshotList(QTreeWidgetItem *item);
    void resizeScreenshotListColumns();
    void setWidgetsDisabled(QStringList list, bool disable);
    void setProgressBarLength(quint32 length);
    void locateSteamDir(QString steamDir);
    void prepareScreenshots(quint32 addedLines);
    void warnOnMissingVDF(bool userDataExists, QString vdfFilename);
    void moveWindow(QSize geometry, QPoint moveToPoint);
    void setComboBoxesCleared(QStringList list);
    void setLabelsCleared(QStringList list);
    void insertIntoComboBox(QString name, QStringList items);
    void setLabelsOnMissingStuff(bool userDataMissing, QString vdfFilename);
    void returnComboBoxUserIDCurrentText();
    void returnScreenshotsSelected(QString lastSelectedScreenshotDir);
    void setProgressBarValue(quint32 value);
    void deleteCopiedWidgetItem(QString path);
    void setIndexOfComboBoxGameID(QString lastSelectedGameID);
    void setLabelsText(QStringList list, QString text);
    void setLabelsVisible(QStringList list, bool visible);
    void setStatusLabelText(QString text, QString color);
    void setDirStatusLabelsVisible(bool visible);


private slots:
    void on_pushButton_addScreenshots_clicked();
    void on_pushButton_clearQueue_clicked();
    void on_pushButton_copyScreenshots_clicked();
    void on_pushButton_prepare_clicked();
    void on_pushButton_locateSteamDir_clicked();
    void checkVDF();
};

#endif // MAINWINDOW_H

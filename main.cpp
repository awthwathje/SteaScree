#include "mainwindow.h"
#include "controller.h"

#include <QApplication>

// TODO: maximum resolution check
// TODO: app update check
// TODO: grag'n'drop screenshots

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QCoreApplication::setOrganizationName("Foyl");
    QCoreApplication::setOrganizationDomain("foyl.io");
    QCoreApplication::setApplicationName("SteaScree");

    MainWindow w;
    Controller c;

    QObject::connect(&w, &MainWindow::getOS,
                     &c, &Controller::returnOS);

    QObject::connect(&c, &Controller::sendOS,
                     &w, &MainWindow::setButtonPadding);

    QObject::connect(&c, &Controller::addWidgetItemToScreenshotList,
                     &w, &MainWindow::addWidgetItemToScreenshotList);

    QObject::connect(&c, &Controller::resizeScreenshotListColumns,
                     &w, &MainWindow::resizeScreenshotListColumns);

    QObject::connect(&w, &MainWindow::getSteamDir,
                     &c, &Controller::returnSteamDir);

    QObject::connect(&c, &Controller::sendSteamDir,
                     &w, &MainWindow::locateSteamDir);

    QObject::connect(&w, &MainWindow::setUserDataPaths,
                     &c, &Controller::setUserDataPaths);

    QObject::connect(&w, &MainWindow::pushButtonAddScreenshots_clicked,
                     &c, &Controller::returnLastSelectedScreenshotDir);

    QObject::connect(&c, &Controller::sendLastSelectedScreenshotDir,
                     &w, &MainWindow::returnScreenshotsSelected);

    QObject::connect(&w, &MainWindow::sendScreenshotsSelected,
                     &c, &Controller::addScreenshotsToPool);

    QObject::connect(&w, &MainWindow::pushButtonPrepare_clicked,
                     &c, &Controller::returnLinesState);

    QObject::connect(&c, &Controller::sendLinesState,
                     &w, &MainWindow::prepareScreenshots);

    QObject::connect(&c, &Controller::disableWidgets,
                     &w, &MainWindow::setWidgetsDisabled);

    QObject::connect(&w, &MainWindow::clearScreenshotPathsPool,
                     &c, &Controller::clearScreenshotPathsPool);

    QObject::connect(&w, &MainWindow::clearState,
                     &c, &Controller::clearState);

    QObject::connect(&w, &MainWindow::clearCopyingStatusLabels,
                     &c, &Controller::clearCopyingStatusLabels);

    QObject::connect(&w, &MainWindow::getScreenshotPathsPoolLength,
                     &c, &Controller::returnScreenshotPathPoolLength);

    QObject::connect(&c, &Controller::sendScreenshotPathPoolLength,
                     &w, &MainWindow::setVisibleProgressBar);

    QObject::connect(&w, &MainWindow::writeVDF,
                     &c, &Controller::writeVDF);

    QObject::connect(&w, &MainWindow::pushScreenshots,
                     &c, &Controller::pushScreenshots);

    QObject::connect(&w, &MainWindow::getVDFStatus,
                     &c, &Controller::returnVDFStatus);

    QObject::connect(&c, &Controller::sendVDFStatus,
                     &w, &MainWindow::warnOnMissingVDF);

    QObject::connect(&c, &Controller::moveWindow,
                     &w, &MainWindow::moveWindow);

    QObject::connect(&c, &Controller::setLabelStatusErrorVisible,
                     &w, &MainWindow::setLabelStatusErrorVisible);

    QObject::connect(&w, &MainWindow::sendSettings,
                     &c, &Controller::writeSettings);

    QObject::connect(&c, &Controller::clearWidgets,
                     &w, &MainWindow::setComboBoxesCleared);

    QObject::connect(&c, &Controller::sendToComboBox,
                     &w, &MainWindow::insertIntoComboBox);

    QObject::connect(&c, &Controller::setIndexOfComboBoxGameID,
                     &w, &MainWindow::setIndexOfComboBoxGameID);

    QObject::connect(&c, &Controller::setLabelsOnMissingStuff,
                     &w, &MainWindow::setLabelsOnMissingStuff);

    QObject::connect(&c, &Controller::getComboBoxUserIDCurrentText,
                     &w, &MainWindow::returnComboBoxUserIDCurrentText,
                     Qt::DirectConnection);

    QObject::connect(&w, &MainWindow::sendComboBoxUserIDCurrentText,
                     &c, &Controller::setSelectedUserID);

    QObject::connect(&c, &Controller::sendLabelsText,
                     &w, &MainWindow::setLabelsText);

    QObject::connect(&c, &Controller::setProgressBarValue,
                     &w, &MainWindow::setProgressBarValue);

    QObject::connect(&c, &Controller::deleteCopiedWidgetItem,
                     &w, &MainWindow::deleteCopiedWidgetItem);

    w.show();
    w.bootStrap();
    c.bootStrap();

    return a.exec();
}

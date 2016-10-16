#include "mainwindow.h"
#include "model.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QCoreApplication::setOrganizationName("Foyl");
    QCoreApplication::setOrganizationDomain("foyl.io");
    QCoreApplication::setApplicationName("SteaScree");

    MainWindow w;
    Model m;

    QObject::connect(&w, &MainWindow::getOS,
                     &m, &Model::returnOS);

    QObject::connect(&m, &Model::sendOS,
                     &w, &MainWindow::setButtonPadding);

    QObject::connect(&m, &Model::addWidgetItemToScreenshotList,
                     &w, &MainWindow::addWidgetItemToScreenshotList);

    QObject::connect(&m, &Model::resizeScreenshotListColumns,
                     &w, &MainWindow::resizeScreenshotListColumns);

    QObject::connect(&w, &MainWindow::getSteamDir,
                     &m, &Model::returnSteamDir);

    QObject::connect(&m, &Model::sendSteamDir,
                     &w, &MainWindow::locateSteamDir);

    QObject::connect(&w, &MainWindow::setUserDataPaths,
                     &m, &Model::setUserDataPaths);

    QObject::connect(&w, &MainWindow::pushButtonAddScreenshots_clicked,
                     &m, &Model::returnLastSelectedScreenshotDir);

    QObject::connect(&m, &Model::sendLastSelectedScreenshotDir,
                     &w, &MainWindow::returnScreenshotsSelected);

    QObject::connect(&w, &MainWindow::sendScreenshotsSelected,
                     &m, &Model::addScreenshotsToPool);

    QObject::connect(&w, &MainWindow::pushButtonPrepare_clicked,
                     &m, &Model::returnLinesState);

    QObject::connect(&m, &Model::sendLinesState,
                     &w, &MainWindow::prepareScreenshots);

    QObject::connect(&m, &Model::disableWidgets,
                     &w, &MainWindow::setWidgetsDisabled);

    QObject::connect(&w, &MainWindow::clearScreenshotPathsPool,
                     &m, &Model::clearScreenshotPathsPool);

    QObject::connect(&w, &MainWindow::clearCopyingStatusLabels,
                     &m, &Model::clearCopyingStatusLabels);

    QObject::connect(&w, &MainWindow::getScreenshotPathsPoolLength,
                     &m, &Model::returnScreenshotPathPoolLength);

    QObject::connect(&m, &Model::sendScreenshotPathPoolLength,
                     &w, &MainWindow::setVisibleProgressBar);

    QObject::connect(&w, &MainWindow::writeVDF,
                     &m, &Model::writeVDF);

    QObject::connect(&w, &MainWindow::pushScreenshots,
                     &m, &Model::pushScreenshots);

    QObject::connect(&w, &MainWindow::getVDFStatus,
                     &m, &Model::returnVDFStatus);

    QObject::connect(&m, &Model::sendVDFStatus,
                     &w, &MainWindow::warnOnMissingVDF);

    QObject::connect(&m, &Model::moveWindow,
                     &w, &MainWindow::moveWindow);

    QObject::connect(&m, &Model::setLabelStatusErrorVisible,
                     &w, &MainWindow::setLabelStatusErrorVisible);

    QObject::connect(&w, &MainWindow::sendSettings,
                     &m, &Model::writeSettings);

    QObject::connect(&m, &Model::clearWidgets,
                     &w, &MainWindow::setComboBoxesCleared);

    QObject::connect(&m, &Model::sendToComboBox,
                     &w, &MainWindow::insertIntoComboBox);

    QObject::connect(&m, &Model::setIndexOfComboBoxGameID,
                     &w, &MainWindow::setIndexOfComboBoxGameID);

    QObject::connect(&m, &Model::setLabelsOnMissingStuff,
                     &w, &MainWindow::setLabelsOnMissingStuff);

    QObject::connect(&m, &Model::getComboBoxUserIDCurrentText,
                     &w, &MainWindow::returnComboBoxUserIDCurrentText,
                     Qt::DirectConnection);

    QObject::connect(&w, &MainWindow::sendComboBoxUserIDCurrentText,
                     &m, &Model::setSelectedUserID);

    QObject::connect(&m, &Model::sendLabelsText,
                     &w, &MainWindow::setLabelsText);

    QObject::connect(&m, &Model::setProgressBarValue,
                     &w, &MainWindow::setProgressBarValue);

    QObject::connect(&m, &Model::deleteCopiedWidgetItem,
                     &w, &MainWindow::deleteCopiedWidgetItem);

    w.show();
    w.bootStrap();
    m.bootStrap();

    return a.exec();
}

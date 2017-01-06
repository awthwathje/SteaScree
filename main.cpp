#include "mainwindow.h"
#include "controller.h"
#include "interfaceadjuster.h"
#include "screenshot.h"

#include <QApplication>


Q_DECLARE_METATYPE(Screenshot)


// TODO: maximum resolution check
// TODO: app update check
// TODO: grag'n'drop screenshots
// TODO: "Screenshot queue" widget slightly bounces when the window gets resized
// TODO: make explanatory tooltips in main window
// TODO: more "in progress" states
// TODO: UI in separate thread
// TODO: multi-threading


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QCoreApplication::setOrganizationName("Foyl");
    QCoreApplication::setOrganizationDomain("foyl.io");
    QCoreApplication::setApplicationName("SteaScree");

    MainWindow w;
    Controller c;
    InterfaceAdjuster i;

    QObject::connect(&w, &MainWindow::sendButtonList,
                     &c, &Controller::getButtonList);

    QObject::connect(&c, &Controller::adjustButtons,
                     &i, &InterfaceAdjuster::setButtonsPadding);

    QObject::connect(&c, &Controller::addWidgetItemToScreenshotList,
                     &w, &MainWindow::addWidgetItemToScreenshotList);

    QObject::connect(&c, &Controller::resizeScreenshotListColumns,
                     &w, &MainWindow::resizeScreenshotListColumns);

    QObject::connect(&w, &MainWindow::getSteamDir,
                     &c, &Controller::returnSteamDir);

    QObject::connect(&c, &Controller::sendSteamDir,
                     &w, &MainWindow::locateSteamDir);

    QObject::connect(&w, &MainWindow::sendUserDataPaths,
                     &c, &Controller::setUserDataPaths);

    QObject::connect(&w, &MainWindow::pushButton_addScreenshots_clicked,
                     &c, &Controller::returnLastSelectedScreenshotDir);

    QObject::connect(&c, &Controller::sendLastSelectedScreenshotDir,
                     &w, &MainWindow::returnScreenshotsSelected);

    QObject::connect(&w, &MainWindow::sendScreenshotsSelected,
                     &c, &Controller::addScreenshotsToPool);

    QObject::connect(&w, &MainWindow::pushButton_prepare_clicked,
                     &c, &Controller::returnLinesState);

    QObject::connect(&c, &Controller::sendLinesState,
                     &w, &MainWindow::prepareScreenshots);

    QObject::connect(&c, &Controller::sendWidgetsDisabled,
                     &w, &MainWindow::setWidgetsDisabled);

    QObject::connect(&c, &Controller::sendLabelsVisible,
                     &w, &MainWindow::setLabelsVisible);

    QObject::connect(&c, &Controller::sendStatusLabelText,
                     &w, &MainWindow::setStatusLabelText);

    QObject::connect(&c, &Controller::sendDirStatusLabelsVisible,
                     &w, &MainWindow::setDirStatusLabelsVisible);

    QObject::connect(&w, &MainWindow::clearScreenshotPathsPool,
                     &c, &Controller::clearScreenshotPathsPool);

    QObject::connect(&w, &MainWindow::clearState,
                     &c, &Controller::clearState);

    QObject::connect(&w, &MainWindow::clearCopyingStatusLabels,
                     &c, &Controller::clearCopyingStatusLabels);

    QObject::connect(&w, &MainWindow::getScreenshotPathsPoolLength,
                     &c, &Controller::returnScreenshotPathPoolLength);

    QObject::connect(&c, &Controller::sendProgressBarLength,
                     &w, &MainWindow::setProgressBarLength);

    QObject::connect(&w, &MainWindow::writeVDF,
                     &c, &Controller::writeVDF);

    QObject::connect(&w, &MainWindow::sendSelectedIDs,
                     &c, &Controller::prepareScreenshots);

    QObject::connect(&w, &MainWindow::getVDFStatus,
                     &c, &Controller::returnVDFStatus);

    QObject::connect(&c, &Controller::sendVDFStatus,
                     &w, &MainWindow::warnOnMissingVDF);

    QObject::connect(&c, &Controller::moveWindow,
                     &w, &MainWindow::moveWindow);

//    QObject::connect(&c, &Controller::setLabelStatusErrorVisible,
//                     &w, &MainWindow::setLabelStatusErrorVisible);

    QObject::connect(&w, &MainWindow::sendSettings,
                     &c, &Controller::writeSettings);

    QObject::connect(&c, &Controller::sendComboBoxesCleared,
                     &w, &MainWindow::setComboBoxesCleared);

    QObject::connect(&c, &Controller::sendLabelsCleared,
                     &w, &MainWindow::setLabelsCleared);

    QObject::connect(&c, &Controller::sendToComboBox,
                     &w, &MainWindow::insertIntoComboBox);

    QObject::connect(&c, &Controller::sendIndexOfComboBoxGameID,
                     &w, &MainWindow::setIndexOfComboBoxGameID);

    QObject::connect(&c, &Controller::sendLabelsOnMissingStuff,
                     &w, &MainWindow::setLabelsOnMissingStuff);

    QObject::connect(&c, &Controller::getComboBoxUserIDCurrentText,
                     &w, &MainWindow::returnComboBoxUserIDCurrentText,
                     Qt::DirectConnection);

    QObject::connect(&w, &MainWindow::sendComboBoxUserIDCurrentText,
                     &c, &Controller::setSelectedUserID);

    QObject::connect(&c, &Controller::sendLabelsText,
                     &w, &MainWindow::setLabelsText);

    QObject::connect(&c, &Controller::sendProgressBarValue,
                     &w, &MainWindow::setProgressBarValue);

    QObject::connect(&c, &Controller::deleteCopiedWidgetItem,
                     &w, &MainWindow::deleteCopiedWidgetItem);

    w.show();
    w.bootStrap();
    c.bootStrap();

    return a.exec();
}

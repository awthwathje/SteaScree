#include "controller.h"
#include "interfaceadjuster.h"

#include <QApplication>

Q_DECLARE_METATYPE(Screenshot)


// TODO: design inconsitencies across platforms
// TODO: UI in separate thread
// TODO: multi-threading


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QCoreApplication::setOrganizationName("Foyl");
    QCoreApplication::setOrganizationDomain("foyl.io");
    QCoreApplication::setApplicationName("SteaScree");
    QCoreApplication::setApplicationVersion(QString(APP_VERSION));

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

    QObject::connect(&c, &Controller::moveWindow,
                     &w, &MainWindow::moveWindow);

    QObject::connect(&w, &MainWindow::sendSettings,
                     &c, &Controller::writeSettings);

    QObject::connect(&c, &Controller::sendComboBoxesCleared,
                     &w, &MainWindow::setComboBoxesCleared);

    QObject::connect(&c, &Controller::sendLabelsCleared,
                     &w, &MainWindow::setLabelsCleared);

    QObject::connect(&c, &Controller::sendToComboBox,
                     &w, &MainWindow::insertIntoComboBox);

    QObject::connect(&c, &Controller::sendIndexOfComboBox,
                     &w, &MainWindow::setIndexOfComboBox);

    QObject::connect(&c, &Controller::sendLabelsOnMissingStuff,
                     &w, &MainWindow::setLabelsOnMissingStuff);

    QObject::connect(&c, &Controller::sendLabelsText,
                     &w, &MainWindow::setLabelsText);

    QObject::connect(&c, &Controller::sendProgressBarValue,
                     &w, &MainWindow::setProgressBarValue);

    QObject::connect(&c, &Controller::deleteCopiedWidgetItem,
                     &w, &MainWindow::deleteCopiedWidgetItem);

    QObject::connect(&c, &Controller::sendUpdateInfo,
                     &w, &MainWindow::offerUpdate);

    QObject::connect(&w, &MainWindow::sendNeverOfferUpdate,
                     &c, &Controller::writeSettingNeverOfferUpdate);

    QObject::connect(&w, &MainWindow::sendNewlySelectedUserID,
                     &c, &Controller::fillGameIDs);

    QObject::connect(&w, &MainWindow::sendTreeWidgetPointer,
                     &c, &Controller::receiveTreeWidgetPointer);

    w.bootStrap();
    c.bootStrap();
    w.show();

    return a.exec();
}

#include "controller.h"
#include "interfaceadjuster.h"

#include <QApplication>
#include <QtGlobal>
#include <QtDebug>
#include <QTextStream>
#include <QTextCodec>
#include <QLocale>
#include <QTime>

Q_DECLARE_METATYPE(Screenshot)

static QTextCodec *logCodec = NULL;
static FILE *logStream = NULL;
QString logFilePath = "debug.log";


// TODO: design inconsitencies across platforms
// TODO: UI in separate thread
// TODO: multi-threading


void customMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QByteArray localMsg = msg.toLocal8Bit();
    QString fileName(context.file);
    QByteArray file = logCodec->fromUnicode(fileName);
    QTime time = QTime::currentTime();
    QString formatedTime = time.toString("hh:mm:ss.zzz");
    fprintf(logStream, "%s ", qPrintable(formatedTime));

    switch (type) {
    case QtDebugMsg:
        fprintf(logStream, "Debug: %s (`%s:%u, %s`)\n", localMsg.constData(), file.constData(), context.line, context.function);
        break;
    case QtWarningMsg:
        fprintf(logStream, "Warning: %s (`%s:%u, %s`)\n", localMsg.constData(), file.constData(), context.line, context.function);
        break;
    case QtInfoMsg:
        fprintf(logStream, "Info: %s (`%s:%u, %s`)\n", localMsg.constData(), file.constData(), context.line, context.function);
        break;
    case QtCriticalMsg:
        fprintf(logStream, "Critical: %s (`%s:%u, %s`)\n", localMsg.constData(), file.constData(), context.line, context.function);
        break;
    case QtFatalMsg:
        fprintf(logStream, "Fatal: %s (`%s:%u, %s`)\n", localMsg.constData(), file.constData(), context.line, context.function);
        abort();
        break;
    }
    fflush(logStream);
}

int main(int argc, char *argv[])
{
    QByteArray envVar = qgetenv("QTDIR");       //  check if the app is ran via QT Creator

    if (envVar.isEmpty())
        logStream = _wfopen(logFilePath.toStdWString().c_str(), L"a");  // log to file, append mode
    else
        logStream = stderr;                     // print all errors to console

    logCodec = QTextCodec::codecForName("UTF-8");
    qInstallMessageHandler(customMessageOutput); // custom message handler for debugging

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

    QObject::connect(&c, &Controller::sendJpegQualityValue,
                     &w, &MainWindow::setJpegQualityValue);

    w.bootStrap();
    c.bootStrap();
    w.show();

    return a.exec();
}

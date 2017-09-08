QT          +=  core gui network widgets

TARGET      =   SteaScree

TEMPLATE    =   app

SOURCES     +=  main.cpp\
                mainwindow.cpp \
                controller.cpp \
                largefiledialog.cpp \
                interfaceadjuster.cpp \
                qtreewidgetdraganddrop.cpp

HEADERS     +=  mainwindow.h \
                controller.h \
                largefiledialog.h \
                interfaceadjuster.h \
                screenshot.h \
                qtreewidgetdraganddrop.h

FORMS       +=  mainwindow.ui \
                largefiledialog.ui

RESOURCES   +=  \
                images.qrc

VERSION     =   1.5.4

DEFINES     +=  APP_VERSION=\\\"$$VERSION\\\"

macx:ICON   =   res/icons/SteaScree.icns

win32:RC_ICONS                  = res/icons/SteaScree.ico
win32:QMAKE_TARGET_COMPANY      = Foyl
win32:QMAKE_TARGET_PRODUCT      = SteaScree
win32:QMAKE_TARGET_DESCRIPTION  = SteaScree: Steam Cloud Screenshot Uploader
win32:QMAKE_TARGET_COPYRIGHT    = GNU GPL v3

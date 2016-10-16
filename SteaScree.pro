QT          +=  core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET      =   SteaScree

TEMPLATE    =   app

SOURCES     +=  main.cpp\
                mainwindow.cpp \
                model.cpp

HEADERS     +=  mainwindow.h \
                model.h

FORMS       +=  mainwindow.ui

VERSION     =   1.0.4.0

macx:ICON   =   res/icons/SteaScree.icns

win32:RC_ICONS                  = res/icons/SteaScree.ico
win32:QMAKE_TARGET_COMPANY      = Foyl
win32:QMAKE_TARGET_PRODUCT      = SteaScree
win32:QMAKE_TARGET_DESCRIPTION  = SteaScree: Steam Cloud Screenshot Uploader
win32:QMAKE_TARGET_COPYRIGHT    = GNU GPL v3

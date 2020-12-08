#-------------------------------------------------
#
# Project created by QtCreator 2020-10-17T19:05:56
#
#-------------------------------------------------

QT       += core gui
QT       += multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

//数据库
include(./sqlapi/sqlapi.pri)
INCLUDEPATH += $$PWD/sqlapi
//ui界面
include(./uiapi/uiapi.pri)
INCLUDEPATH += $$PWD/uiapi
//网络
include(./netapi/netapi.pri)
INCLUDEPATH += $$PWD/netapi

TARGET = MusicPlayer
TEMPLATE = app


SOURCES += main.cpp\
        musicwidget.cpp

HEADERS  += musicwidget.h

FORMS    += musicwidget.ui

RESOURCES += \
    resource.qrc

DISTFILES += \
    qss/default.css

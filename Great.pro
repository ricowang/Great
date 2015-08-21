#-------------------------------------------------
#
# Project created by QtCreator 2015-08-08T20:47:24
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Great
TEMPLATE = app

SOURCES += main.cpp\
        greatwindow.cpp \
    viewmanager.cpp \
    contentmanager.cpp \
    downloadmanager.cpp \
    loader.cpp \
    decoder.cpp \
    canvas.cpp

HEADERS  += greatwindow.h \
    viewmanager.h \
    contentmanager.h \
    downloadmanager.h \
    loader.h \
    decoder.h \
    canvas.h

FORMS    += greatwindow.ui

CONFIG += c++11

QMAKE_CXXFLAGS += -std=c++0x

#LIBS += /usr/lib/x86_64-linux-gnu/libjpeg.so
LIBS += /usr/lib/x86_64-linux-gnu/libjpeg.so


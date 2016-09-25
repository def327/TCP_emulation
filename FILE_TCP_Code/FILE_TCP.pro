#-------------------------------------------------
#
# Project created by QtCreator 2016-08-20T23:17:42
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = FILE_TCP
TEMPLATE = app


SOURCES += main.cpp \
    server.cpp \
    client.cpp

HEADERS  += \
    server.h \
    client.h

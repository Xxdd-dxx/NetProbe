QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = NetworkAssistant
TEMPLATE = app
DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    tcpserverwidget.cpp \
    tcpclientwidget.cpp \
    udpserverwidget.cpp \
    udpclientwidget.cpp \
    datatoolwidget.cpp

HEADERS += \
    mainwindow.h \
    tcpserverwidget.h \
    tcpclientwidget.h \
    udpserverwidget.h \
    udpclientwidget.h \
    datatoolwidget.h
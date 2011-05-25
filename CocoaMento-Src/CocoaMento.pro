#-------------------------------------------------
#
# Project created by QtCreator 2011-03-29T16:22:30
#
#-------------------------------------------------

QT       += core gui network

CONFIG += qthread

TARGET = CocoaMento
TEMPLATE = app
LIBS += -framework Cocoa \
        -framework Security \
        -framework Growl

SOURCES += main.cpp\
    statusicon.mm \
    menuitem.mm \
    cocoamenuitemvirtual.mm \
    qtport.cpp \
    cocoaappinitlizer.mm \
    menuviewitem.mm\
    menu.mm \
    configview.cpp \
    mento.cpp \
    accountform.cpp \
    mentothread.cpp \
    guite.cpp

HEADERS  += \
    statusicon.h \
    menuitem.h \
    cocoamenuitemvirtual.h \
    qtport.h \
    cocoaappinitlizer.h \
    menuviewitem.h \
    menu.h \
    configview.h \
    mento.h \
    accountform.h \
    mentothread.h \
    guite.h \

FORMS    += \
    configview.ui \
    accountform.ui \
    guite.ui

RESOURCES +=

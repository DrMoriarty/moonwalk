#-------------------------------------------------
#
# Project created by QtCreator 2011-08-07T21:59:46
#
#-------------------------------------------------

QT       += core gui xml widgets

TARGET = MoonWalk
TEMPLATE = app
ICON = editor.icns
CONFIG += debug

SOURCES += main.cpp\
    animationwindow.cpp \
    drawarea.cpp \
    timeline.cpp \
    anim.cpp \
    levelwindow.cpp \
    mygraphicsview.cpp

HEADERS  += animationwindow.h \
    drawarea.h \
    timeline.h \
    anim.h \
    levelwindow.h \
    main.h \
    mygraphicsview.h

FORMS    += animationwindow.ui \
    levelwindow.ui

RESOURCES += \
    resources.qrc

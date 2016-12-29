#-------------------------------------------------
#
# Project created by QtCreator 2016-12-10T09:31:40
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = python-mod
CONFIG   += console c++11
CONFIG   -= app_bundle

TEMPLATE = app

LIBS += -lpython3.4m

HEADERS += \
    vec_base.h \
    vec_module.h \ 
    py_utils.h

SOURCES += \
    vec_base.cpp \
    vec3.cpp \
    vec_module.cpp \
    main.cpp \
    py_utils.cpp


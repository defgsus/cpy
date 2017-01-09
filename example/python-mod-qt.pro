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
    test_module.h 

SOURCES += \
    test.cpp \
    test_module.cpp \
    main.cpp

python_modfiles = \
    $$PWD/test.cpp

pymod.target = test_module.h
pymod.commands = /home/defgsus/prog/python/dev/lolpig/lolpig.py -i $$python_modfiles -o $$PWD/test_module -m mod -n PYMOD
pymod.depends = $$python_modfiles

QMAKE_EXTRA_TARGETS += pymod

PRE_TARGETDEPS += test_module.h

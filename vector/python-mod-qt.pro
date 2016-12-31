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


python_modfiles = $$PWD/vec_base.cpp $$PWD/vec3.cpp
pymod.target = vec_module.h
#pymod.commands = /home/defgsus/prog/python/dev/lolpig/lolpig.py -i $$python_modfiles -o $$PWD/vec_module -m vec -n MOP
pymod.commands = lolpig.py -i $$python_modfiles -o $$PWD/vec_module -m vec -n MOP
pymod.depends = $$python_modfiles

QMAKE_EXTRA_TARGETS += pymod

PRE_TARGETDEPS += vec_module.h

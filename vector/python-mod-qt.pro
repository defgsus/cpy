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
    vec_module.h \ 
    pyimpl/vec_base.h \
    pyimpl/mat_base.h \
    py_utils.h \
    pyimpl/vector_math.h

SOURCES += \
    main.cpp \
    py_utils.cpp \
    pyimpl/mat_base.cpp \
    pyimpl/vec3.cpp \
    pyimpl/vec_base.cpp \
    vec_module.cpp \
    pyimpl/mat3.cpp


python_modfiles = \
    $$PWD/pyimpl/vec_base.h \
    $$PWD/pyimpl/vec_base.cpp \
    $$PWD/pyimpl/vec3.cpp \
    $$PWD/pyimpl/mat_base.h \
    $$PWD/pyimpl/mat_base.cpp \
    $$PWD/pyimpl/mat3.cpp \

pymod.target = vec_module.h
pymod.commands = /home/defgsus/prog/python/dev/lolpig/lolpig.py -i $$python_modfiles -o $$PWD/vec_module -m vec -n MOP
#pymod.commands = lolpig.py -i $$python_modfiles -o $$PWD/vec_module -m vec -n MOP
pymod.depends = $$python_modfiles

QMAKE_EXTRA_TARGETS += pymod

PRE_TARGETDEPS += vec_module.h

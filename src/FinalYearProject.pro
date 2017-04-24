#-------------------------------------------------
#
# Project created by QtCreator 2017-02-10T15:33:51
#
#-------------------------------------------------

QT       += core gui opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = FinalYearProject
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += main.cpp\
        mainwindow.cpp \
    fft_test.cpp \
    audio.cpp \
    mainwidget.cpp \
    keyboardwidget.cpp \
    waveformwidget.cpp \
    spectrumwidget.cpp \
    scrollbar.cpp \
    util.cpp

HEADERS  += mainwindow.h \
    fft_test.h \
    audio.h \
    mainwidget.h \
    keyboardwidget.h \
    waveformwidget.h \
    spectrumwidget.h \
    scrollbar.h \
    util.h

LIBS += -lopenal -lpthread\
        -lmpg123\
        -lvorbis -lvorbisfile -logg

QMAKE_CXXFLAGS += -std=gnu++11 -d

FORMS += \
    mainwindow.ui

RESOURCES += \
    resources.qrc

DISTFILES += \
    shaders/keyboardkey.vert \
    shaders/keyboardkey.frag \
    shaders/waveform.frag \
    shaders/waveform.vert \
    shaders/chunk.frag \
    shaders/chunk.vert \
    shaders/marker.frag \
    shaders/marker.vert \
    shaders/spectrum.frag \
    shaders/spectrum.vert

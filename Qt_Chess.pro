QT       += core gui multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    src/main.cpp \
    src/qt_chess.cpp \
    src/chesspiece.cpp \
    src/chessboard.cpp \
    src/chessengine.cpp \
    src/soundsettingsdialog.cpp \
    src/pieceiconsettingsdialog.cpp \
    src/boardcolorsettingsdialog.cpp

HEADERS += \
    src/qt_chess.h \
    src/chesspiece.h \
    src/chessboard.h \
    src/chessengine.h \
    src/soundsettingsdialog.h \
    src/pieceiconsettingsdialog.h \
    src/boardcolorsettingsdialog.h

FORMS += \
    src/qt_chess.ui

RESOURCES += \
    resources.qrc

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

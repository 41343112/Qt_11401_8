QT       += core gui multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    qt_chess.cpp \
    chesspiece.cpp \
    chessboard.cpp \
    soundsettingsdialog.cpp \
    pieceiconsettingsdialog.cpp \
    boardcolorsettingsdialog.cpp

HEADERS += \
    qt_chess.h \
    chesspiece.h \
    chessboard.h \
    soundsettingsdialog.h \
    pieceiconsettingsdialog.h \
    boardcolorsettingsdialog.h

FORMS += \
    qt_chess.ui

RESOURCES += \
    resources.qrc

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

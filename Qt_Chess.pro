QT       += core gui multimedia multimediawidgets network websockets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

# Note: Qt WebSockets module is required for online mode
# If you get "Unknown module(s) in QT: websockets" error:
#   Ubuntu/Debian: sudo apt-get install libqt5websockets5-dev
#   Fedora/RHEL:   sudo dnf install qt5-qtwebsockets-devel
#   Arch Linux:    sudo pacman -S qt5-websockets
#   macOS:         brew install qt5 (includes websockets)
#   Windows:       Install via Qt Maintenance Tool (select WebSockets component)

CONFIG += c++17

# Enable console output for qDebug() on Windows
win32: CONFIG += console

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
    src/boardcolorsettingsdialog.cpp \
    src/updatechecker.cpp \
    src/networkmanager.cpp \
    src/onlinedialog.cpp

HEADERS += \
    src/qt_chess.h \
    src/chesspiece.h \
    src/chessboard.h \
    src/chessengine.h \
    src/soundsettingsdialog.h \
    src/pieceiconsettingsdialog.h \
    src/boardcolorsettingsdialog.h \
    src/theme.h \
    src/updatechecker.h \
    src/networkmanager.h \
    src/onlinedialog.h

FORMS += \
    src/qt_chess.ui

RESOURCES += \
    resources.qrc

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

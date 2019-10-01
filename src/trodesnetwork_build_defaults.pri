#Remove qt library linking
QT -= core gui

#Build directories
CONFIG(debug, debug|release): DESTDIR = $$_PRO_FILE_PWD_/build_d
else: DESTDIR = $$_PRO_FILE_PWD_/build

CONFIG += c++14 warn_on

QMAKE_CLEAN += $$DESTDIR/*$$TARGET*

UI_DIR = $$DESTDIR/ui
MOC_DIR = $$DESTDIR/moc
OBJECTS_DIR = $$DESTDIR/obj
RCC_DIR = $$DESTDIR/rcc

INCLUDEPATH += $$PWD/utility/
INCLUDEPATH += $$PWD/zmq/include/
INCLUDEPATH += $$PWD/libTrodesNetwork/include/


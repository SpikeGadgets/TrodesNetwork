#Build a library
TEMPLATE = lib
#Named libTrodesNetwork
TARGET = TrodesNetwork
#Remove qt library linking
QT -= core gui
#General build config
#Build directories
### should go into a .pri file
CONFIG(debug, debug|release): DESTDIR = $$_PRO_FILE_PWD_/build_d
else: DESTDIR = $$_PRO_FILE_PWD_/build
CONFIG += c++14 warn_on
VERSION = 0.1.2
QMAKE_CLEAN += $$DESTDIR/*$$TARGET*


UI_DIR = $$DESTDIR/ui
MOC_DIR = $$DESTDIR/moc
OBJECTS_DIR = $$DESTDIR/obj
RCC_DIR = $$DESTDIR/rcc

### should go into a .pri file
INCLUDEPATH += ../utility/
INCLUDEPATH += ../zmq/include/
INCLUDEPATH += include/

SOURCES +=  src/AbstractModuleClient.cpp \
            src/CZHelp.cpp \
            src/highfreqclasses.cpp \
            src/networkDataTypes.cpp \
            src/networkincludes.cpp \
            src/trodesglobaltypes.cpp \
            src/trodesmsg.cpp


HEADERS +=  include/libTrodesNetwork/AbstractModuleClient.h \
            include/libTrodesNetwork/CZHelp.h \
            include/libTrodesNetwork/highfreqclasses.h \
            include/libTrodesNetwork/networkDataTypes.h \
            include/libTrodesNetwork/networkincludes.h \
            include/libTrodesNetwork/trodesglobaltypes.h \
            include/libTrodesNetwork/trodesmsg.h

QMAKE_CXXFLAGS += -Wno-unused-parameter
unix:!macx {
    LIBS += -Wl,--whole-archive ../zmq/lib/static/libzmq.a -Wl,--no-whole-archive
    LIBS += -Wl,--whole-archive ../zmq/lib/static/libczmq.a -Wl,--no-whole-archive
    LIBS += -Wl,--whole-archive ../zmq/lib/static/libmlm.a -Wl,--no-whole-archive
}

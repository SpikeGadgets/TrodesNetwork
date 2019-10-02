#Build a library
TEMPLATE = lib
#Named trodesnetwork
TARGET = trodesnetwork
win32: QMAKE_EXTENSION_SHLIB=pyd
else:  QMAKE_EXTENSION_SHLIB=so
#Remove qt library linking
QT -= core gui
#General build config
#Build directories
### should go into a .pri file
CONFIG(debug, debug|release): DESTDIR = $$_PRO_FILE_PWD_/build_d
else: DESTDIR = $$_PRO_FILE_PWD_/build
CONFIG += no_plugin_name_prefix plugin #no prefix or symlinks
CONFIG += c++14 warn_on
VERSION = 0.1.2
QMAKE_CLEAN += $$DESTDIR/*$$TARGET*


UI_DIR = $$DESTDIR/ui
MOC_DIR = $$DESTDIR/moc
OBJECTS_DIR = $$DESTDIR/obj
RCC_DIR = $$DESTDIR/rcc

DEFINES += BOOST_PYTHON_STATIC_LIB
DEFINES += BOOST_NUMPY_STATIC_LIB


INCLUDEPATH += ../utility/
INCLUDEPATH += ../zmq/include/
INCLUDEPATH += ../libTrodesNetwork/include/
INCLUDEPATH += boost/include/
SOURCES +=  ../libTrodesNetwork/src/AbstractModuleClient.cpp \
            ../libTrodesNetwork/src/CZHelp.cpp \
            ../libTrodesNetwork/src/highfreqclasses.cpp \
            ../libTrodesNetwork/src/networkDataTypes.cpp \
            ../libTrodesNetwork/src/networkincludes.cpp \
            ../libTrodesNetwork/src/trodesglobaltypes.cpp \
            ../libTrodesNetwork/src/trodesmsg.cpp \
            src/trodesnetworkpython.cpp


HEADERS +=  ../libTrodesNetwork/include/libTrodesNetwork/AbstractModuleClient.h \
            ../libTrodesNetwork/include/libTrodesNetwork/CZHelp.h \
            ../libTrodesNetwork/include/libTrodesNetwork/highfreqclasses.h \
            ../libTrodesNetwork/include/libTrodesNetwork/networkDataTypes.h \
            ../libTrodesNetwork/include/libTrodesNetwork/networkincludes.h \
            ../libTrodesNetwork/include/libTrodesNetwork/trodesglobaltypes.h \
            ../libTrodesNetwork/include/libTrodesNetwork/trodesmsg.h

unix:!macx {
    DEFINES += BOOST_NO_AUTO_PTR
    QMAKE_CXXFLAGS += -Wno-unused-parameter

    INCLUDEPATH += /usr/include/python3.5m/

    LIBS += -Wl,--whole-archive ../zmq/lib/static/libzmq.a -Wl,--no-whole-archive
    LIBS += -Wl,--whole-archive ../zmq/lib/static/libczmq.a -Wl,--no-whole-archive
    LIBS += -Wl,--whole-archive ../zmq/lib/static/libmlm.a -Wl,--no-whole-archive
    LIBS += boost/lib/libboost_python3.a
    LIBS += boost/lib/libboost_numpy3.a
}

win32:{
    CONFIG += skip_target_version_ext
    QMAKE_CXXFLAGS_WARN_ON -= -w34100 #disable unreferenced formal parameter

    INCLUDEPATH += C:/Python3.5/include/
    INCLUDEPATH += C:/Boost/include/boost-1_65_1/

    LIBS += ../zmq/msvc64/ZeroMQ/lib/libzmq-v140-mt-4_3_1.lib
    LIBS += ../zmq/msvc64/czmq/lib/czmq.lib
    LIBS += ../zmq/msvc64/malamute/lib/mlm.lib
    LIBS += C:/Python3.5/libs/python35.lib
    LIBS += C:/Boost/lib/libboost_python3-vc140-mt-1_65_1.lib
    LIBS += C:/Boost/lib/libboost_numpy3-vc140-mt-1_65_1.lib
}

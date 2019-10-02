include(../trodesnetwork_build_defaults.pri)

#Build a library
TEMPLATE = lib
#Named trodesnetwork
TARGET = trodesnetwork
win32: QMAKE_EXTENSION_SHLIB=pyd
else:  QMAKE_EXTENSION_SHLIB=so

#General build config
CONFIG += no_plugin_name_prefix plugin #no prefix or symlinks

VERSION = 0.1.2

DEFINES += BOOST_PYTHON_STATIC_LIB
DEFINES += BOOST_NUMPY_STATIC_LIB

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

    INCLUDEPATH += boost/include/
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

    INCLUDEPATH += boost/msvc64/include/boost-1_65_1/

    LIBS += ../zmq/msvc64/ZeroMQ/lib/libzmq-v140-mt-4_3_1.lib
    LIBS += ../zmq/msvc64/czmq/lib/czmq.lib
    LIBS += ../zmq/msvc64/malamute/lib/mlm.lib
    LIBS += boost/msvc64/lib/libboost_python3-vc140-mt-1_65_1.lib
    LIBS += boost/msvc64/lib/libboost_numpy3-vc140-mt-1_65_1.lib

    INCLUDEPATH += C:/Python3.5/include/
    LIBS += C:/Python3.5/libs/python35.lib
}
macx{
    QMAKE_CFLAGS_WARN_ON += -Wno-unused-parameter
    QMAKE_CXXFLAGS_WARN_ON += -Wno-unused-parameter

    INCLUDEPATH += /usr/local/include/
    INCLUDEPATH += /usr/local/Frameworks/Python.framework/Versions/3.7/include/python3.7m
    LIBS +=  ../zmq/macos/libzmq/lib/libzmq.a
    LIBS +=  ../zmq/macos/czmq/lib/libczmq.a
    LIBS +=  ../zmq/macos/malamute/lib/libmlm.a
    LIBS += /usr/local/lib/libboost_python37.a
    LIBS += /usr/local/lib/libboost_numpy37.a
    LIBS += /usr/local/Frameworks/Python.framework/Versions/3.7/lib/libpython3.7m.dylib
}

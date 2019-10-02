include(../trodesnetwork_build_defaults.pri)

#Build a library
TEMPLATE = lib
#Named libTrodesNetwork
TARGET = TrodesNetwork

VERSION = 0.1.2

QMAKE_CLEAN += $$DESTDIR/*$$TARGET*

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

unix:!macx {
    QMAKE_CXXFLAGS += -Wno-unused-parameter

    LIBS += -Wl,--whole-archive ../zmq/lib/static/libzmq.a -Wl,--no-whole-archive
    LIBS += -Wl,--whole-archive ../zmq/lib/static/libczmq.a -Wl,--no-whole-archive
    LIBS += -Wl,--whole-archive ../zmq/lib/static/libmlm.a -Wl,--no-whole-archive
}

win32:{
    CONFIG += skip_target_version_ext
    QMAKE_CXXFLAGS_WARN_ON -= -w34100 #disable unreferenced formal parameter

    LIBS += ../zmq/msvc64/ZeroMQ/lib/libzmq-v140-mt-4_3_1.lib
    LIBS += ../zmq/msvc64/czmq/lib/czmq.lib
    LIBS += ../zmq/msvc64/malamute/lib/mlm.lib
}

macx{
    QMAKE_CFLAGS_WARN_ON += -Wno-unused-parameter
    QMAKE_CXXFLAGS_WARN_ON += -Wno-unused-parameter

    LIBS +=  ../zmq/macos/libzmq/lib/libzmq.a
    LIBS +=  ../zmq/macos/czmq/lib/libczmq.a
    LIBS +=  ../zmq/macos/malamute/lib/libmlm.a
}

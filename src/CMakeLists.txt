#*******************
# Linking libraries
#TO BUILD ZMQ FROM SOURCE: 
# In libzmq/, czmq/, and malamute/ directories
# ./configure --enable-static --with-pic --with-uuid=no
# make -j8
# make install
# Then copy over static libraries into zmq/lib/static and dynamics into zmq/lib
if(WIN32)
    find_package(libzmq REQUIRED)
    IF (LIBZMQ_FOUND)
        include_directories(${LIBZMQ_INCLUDE_DIRS})
        list(APPEND MORE_LIBRARIES ${LIBZMQ_LIBRARIES})
        set(pkg_config_libs_private "${pkg_config_libs_private} -lzmq")
    ELSE (LIBZMQ_FOUND)
        message( FATAL_ERROR "libzmq not found." )
    ENDIF (LIBZMQ_FOUND)

    find_package(czmq REQUIRED)
    IF (CZMQ_FOUND)
        include_directories(${CZMQ_INCLUDE_DIRS})
        list(APPEND MORE_LIBRARIES ${CZMQ_LIBRARIES})
        set(pkg_config_libs_private "${pkg_config_libs_private} -lczmq")
    ELSE (CZMQ_FOUND)
        message( FATAL_ERROR "czmq not found." )
    ENDIF (CZMQ_FOUND)

    find_package(malamute REQUIRED)
    IF (MALAMUTE_FOUND)
        include_directories(${MALAMUTE_INCLUDE_DIRS})
        list(APPEND MORE_LIBRARIES ${MALAMUTE_LIBRARIES})
        set(pkg_config_libs_private "${pkg_config_libs_private} -lmlm")
    ELSE (MALAMUTE_FOUND)
        message( FATAL_ERROR "malamute not found." )
    ENDIF (MALAMUTE_FOUND)


    link_libraries(
        ${LIBZMQ_LIBRARIES}
        ${CZMQ_LIBRARIES}
        ${MALAMUTE_LIBRARIES}
    )
else()
link_libraries(-Wl,--whole-archive ${CMAKE_CURRENT_SOURCE_DIR}/zmq/lib/static/libzmq.a -Wl,--no-whole-archive)
link_libraries(-Wl,--whole-archive ${CMAKE_CURRENT_SOURCE_DIR}/zmq/lib/static/libczmq.a -Wl,--no-whole-archive)
link_libraries(-Wl,--whole-archive ${CMAKE_CURRENT_SOURCE_DIR}/zmq/lib/static/libmlm.a -Wl,--no-whole-archive)
endif()
add_subdirectory(libTrodesNetwork)
# add_subdirectory(pythonPackaging)

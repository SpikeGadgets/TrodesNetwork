#*******************
# Linking libraries
#TO BUILD ZMQ FROM SOURCE: 
# In libzmq/, czmq/, and malamute/ directories
# ./configure --enable-static --with-pic --with-uuid=no
# make -j8
# make install
# Then copy over static libraries into zmq/lib/static and dynamics into zmq/lib
# if(MSVC)
#     link_libraries(${CMAKE_CURRENT_SOURCE_DIR}/zmq/win32/lib_64/libzmq.lib)
#     link_libraries(${CMAKE_CURRENT_SOURCE_DIR}/zmq/win32/lib_64/libczmq.lib)
#     link_libraries(${CMAKE_CURRENT_SOURCE_DIR}/zmq/win32/lib_64/libmlm.lib)
# elseif(WIN32)
if(WIN32)
    set(LIBZMQ_ROOT_DIR "./zmq/msvc64/ZeroMQ/")
    set(CZMQ_ROOT_DIR "./zmq/msvc64/czmq/")
    set(MALAMUTE_ROOT_DIR "./zmq/msvc64/malamute")

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
    add_definitions(-DZMQ_STATIC)
    add_definitions(-DCZMQ_STATIC)
    # target_compile_definitions(${LIBRARY_TARGET_NAME} PRIVATE MLM_STATIC)
    link_libraries(${LIBRARY_TARGET_NAME} wsock32 ws2_32 Iphlpapi)
    # link_libraries(-Wl,--whole-archive ${CMAKE_INSTALL_PREFIX}/lib/libzmq.dll.a -Wl,--no-whole-archive)
    # link_libraries(-Wl,--whole-archive ${CMAKE_INSTALL_PREFIX}/lib/libczmq.dll.a -Wl,--no-whole-archive)
    # link_libraries(-Wl,--whole-archive ${CMAKE_INSTALL_PREFIX}/lib/libmlm.dll.a -Wl,--no-whole-archive)

elseif(APPLE)
    link_libraries( /usr/local/Cellar/zeromq/4.3.1_1/lib/libzmq.a )
    link_libraries( /usr/local/Cellar/czmq/4.2.0/lib/libczmq.a )
    link_libraries( /usr/local/lib/libmlm.a )
else()
    link_libraries(-Wl,--whole-archive ${CMAKE_CURRENT_SOURCE_DIR}/zmq/lib/static/libzmq.a -Wl,--no-whole-archive)
    link_libraries(-Wl,--whole-archive ${CMAKE_CURRENT_SOURCE_DIR}/zmq/lib/static/libczmq.a -Wl,--no-whole-archive)
    link_libraries(-Wl,--whole-archive ${CMAKE_CURRENT_SOURCE_DIR}/zmq/lib/static/libmlm.a -Wl,--no-whole-archive)
endif()
add_subdirectory(libTrodesNetwork)
add_subdirectory(pythonPackaging)
# add_subdirectory(libTrodesNetwork_C)
install(FILES ./README  DESTINATION .)

#*******************
# Linking libraries
#TO BUILD ZMQ FROM SOURCE: 
# In libzmq/, czmq/, and malamute/ directories
# ./configure --enable-static --with-pic --with-uuid=no
# make -j8
# make install
# Then copy over static libraries into zmq/lib/static and dynamics into zmq/lib
link_libraries(-Wl,--whole-archive ${CMAKE_CURRENT_SOURCE_DIR}/zmq/lib/static/libzmq.a -Wl,--no-whole-archive)
link_libraries(-Wl,--whole-archive ${CMAKE_CURRENT_SOURCE_DIR}/zmq/lib/static/libczmq.a -Wl,--no-whole-archive)
link_libraries(-Wl,--whole-archive ${CMAKE_CURRENT_SOURCE_DIR}/zmq/lib/static/libmlm.a -Wl,--no-whole-archive)

add_subdirectory(libTrodesNetwork)
add_subdirectory(pythonPackaging)

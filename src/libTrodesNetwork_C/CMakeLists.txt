# This is the "physical" (i.e. real) name of the library.
# The actual file name of the library built is constructed based on conventions
# of the native platform, such as lib<LIBRARY_TARGET_NAME>.a,
# lib<LIBRARY_TARGET_NAME>.dylib or <LIBRARY_TARGET_NAME>.lib.
set(LIBRARY_TARGET_NAME libTrodesNetwork_C)
set(CMAKE_NO_BUILTIN_CHRPATH True)
# List of CPP (source) library files.
set(${LIBRARY_TARGET_NAME}_SRC
        ../libTrodesNetwork/src/AbstractModuleClient.cpp
        ../libTrodesNetwork/src/CZHelp.cpp
        ../libTrodesNetwork/src/highfreqclasses.cpp
        ../libTrodesNetwork/src/networkDataTypes.cpp
        ../libTrodesNetwork/src/networkincludes.cpp
        ../libTrodesNetwork/src/trodesglobaltypes.cpp
        ../libTrodesNetwork/src/trodesmsg.cpp
        src/trodesnetwork_c.cpp
)

# List of HPP (header) library files.
set(${LIBRARY_TARGET_NAME}_HDR
        src/trodesnetwork_c.h
)

include_directories(../libTrodesNetwork/include)



# Adds a library target called ${LIBRARY_TARGET_NAME} to be built from the
# source and header files listed in the command invocation.
add_library(${LIBRARY_TARGET_NAME} ${${LIBRARY_TARGET_NAME}_SRC} ${${LIBRARY_TARGET_NAME}_HDR})

# Set two minimum target properties for the library.
# See https://cmake.org/cmake/help/latest/command/set_target_properties.html
# Properties are: 1) Library version number 2) list of corresponding public headers
set_target_properties(${LIBRARY_TARGET_NAME} PROPERTIES VERSION       ${${PROJECT_NAME}_VERSION} 
                                                        PUBLIC_HEADER  "${${LIBRARY_TARGET_NAME}_HDR}")

# Specify include directories for both compilation and installation process.
# The $<INSTALL_PREFIX> generator expression is useful to ensure to create 
# relocatable configuration files, see https://cmake.org/cmake/help/latest/manual/cmake-packages.7.html#creating-relocatable-packages
target_include_directories(${LIBRARY_TARGET_NAME} PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
                                                         "$<INSTALL_INTERFACE:$<INSTALL_PREFIX>/${CMAKE_INSTALL_INCLUDEDIR}>")


#*******************
# Linking libraries
target_link_libraries(${LIBRARY_TARGET_NAME} pthread)
# target_link_libraries(${LIBRARY_TARGET_NAME} ${PYTHON_LIBRARIES})


#Because python doesn't want the lib prefix for imported libraries
SET_TARGET_PROPERTIES(${LIBRARY_TARGET_NAME} PROPERTIES PREFIX "")


# If you used find_package() you need to use target_include_directories() and/or
# target_link_libraries(). As explained previously, depending on the imported
# objects, you may need to call either or both:
# - with imported variable:
#   target_include_directories(${LIBRARY_TARGET_NAME} ${FooPackage_INCLUDE_DIRS})
#   target_link_libraries(${LIBRARY_TARGET_NAME} ${FooPackage_LIBRARIES})
# - with imported target:
#   target_link_libraries(${LIBRARY_TARGET_NAME} FooPackage_LIBRARIES::FooPackage_LIBRARIES)
# Specify installation targets, typology and destination folders.
# install(TARGETS ${LIBRARY_TARGET_NAME}
        # EXPORT  ${PROJECT_NAME}
        # LIBRARY       DESTINATION "${CMAKE_INSTALL_LIBDIR}"                            COMPONENT shlib
        # ARCHIVE       DESTINATION "${CMAKE_INSTALL_LIBDIR}"                            COMPONENT lib
        # RUNTIME       DESTINATION "${CMAKE_INSTALL_BINDIR}"                            COMPONENT bin
        # PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${LIBRARY_TARGET_NAME}" COMPONENT dev)
install(TARGETS ${LIBRARY_TARGET_NAME}
        EXPORT  ${PROJECT_NAME}
        LIBRARY       DESTINATION "${CMAKE_INSTALL_LIBDIR}"                            COMPONENT shlib
        ARCHIVE       DESTINATION "${CMAKE_INSTALL_LIBDIR}"                            COMPONENT lib
        RUNTIME       DESTINATION "${CMAKE_INSTALL_BINDIR}"                            COMPONENT bin
        PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${LIBRARY_TARGET_NAME}" COMPONENT dev
        PRIVATE_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"                       COMPONENT dev)
set_property(GLOBAL APPEND PROPERTY ${PROJECT_NAME}_TARGETS ${LIBRARY_TARGET_NAME})

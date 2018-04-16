# THIS FILE IS 100% COPIED FROM THE MALAMUTE REPOSITORY
#
################################################################################
#  THIS FILE IS 100% GENERATED BY ZPROJECT; DO NOT EDIT EXCEPT EXPERIMENTALLY  #
#  Read the zproject/README.md for information about making permanent changes. #
################################################################################

if (NOT MSVC)
    include(FindPkgConfig)
    pkg_check_modules(PC_MALAMUTE "libmlm")
    if (NOT PC_MALAMUTE_FOUND)
        pkg_check_modules(PC_MALAMUTE "libmlm")
    endif (NOT PC_MALAMUTE_FOUND)
    if (PC_MALAMUTE_FOUND)
        # some libraries install the headers is a subdirectory of the include dir
        # returned by pkg-config, so use a wildcard match to improve chances of finding
        # headers and SOs.
        set(PC_MALAMUTE_INCLUDE_HINTS ${PC_MALAMUTE_INCLUDE_DIRS} ${PC_MALAMUTE_INCLUDE_DIRS}/*)
        set(PC_MALAMUTE_LIBRARY_HINTS ${PC_MALAMUTE_LIBRARY_DIRS} ${PC_MALAMUTE_LIBRARY_DIRS}/*)
    endif(PC_MALAMUTE_FOUND)
endif (NOT MSVC)

find_path (
    MALAMUTE_INCLUDE_DIRS
    NAMES malamute.h
    HINTS ${PC_MALAMUTE_INCLUDE_HINTS}
)

find_library (
    MALAMUTE_LIBRARIES
    NAMES mlm
    HINTS ${PC_MALAMUTE_LIBRARY_HINTS}
)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(
    MALAMUTE
    REQUIRED_VARS MALAMUTE_LIBRARIES MALAMUTE_INCLUDE_DIRS
)
mark_as_advanced(
    MALAMUTE_FOUND
    MALAMUTE_LIBRARIES MALAMUTE_INCLUDE_DIRS
)

################################################################################
#  THIS FILE IS 100% GENERATED BY ZPROJECT; DO NOT EDIT EXCEPT EXPERIMENTALLY  #
#  Read the zproject/README.md for information about making permanent changes. #
################################################################################

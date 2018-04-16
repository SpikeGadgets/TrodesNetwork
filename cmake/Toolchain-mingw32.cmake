# set OS to be for windows
SET(CMAKE_SYSTEM_NAME Windows)
SET(WIN32)
# set compilers, use 32-bit for now.
# TODO: if 64 bit defined, then use 64bit. default to 32 bit
SET(CMAKE_C_COMPILER i686-w64-mingw32-gcc-win32)
SET(CMAKE_CXX_COMPILER i686-w64-mingw32-g++-win32)
SET(CMAKE_RC_COMPILER i686-w64-mingw32-windres)

# First dir: path to include/libs for mingw
# Second dir: path to where all libraries held compiled using mingw32
SET(CMAKE_FIND_ROOT_PATH /usr/i686-w64-mingw32 /home/kevin/mingw32-install)

# Tells cmake to look for includes and libraries in the root_path
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
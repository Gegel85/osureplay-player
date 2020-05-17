#
# A CMake module to find libav
#
# Once done this module will define:
#  libav_FOUND - system has libav
#  libav_INCLUDE_DIRS - the libav include directory
#  libav_LIBRARIES - Link these to use libav
#

IF (NOT libav_INCLUDE_DIRS OR NOT libav_LIBRARIES)
    FIND_PATH(libav_INCLUDE_DIRS
        NAMES
            libavcodec/avcodec.h
        PATHS
            /usr/include/                 # Default Fedora28 system include path
            /usr/local/include/           # Default Fedora28 local include path
            ${CMAKE_MODULE_PATH}/include/ # Expected to contain the path to this file for Windows10
            ${libav_DIR}/include/         # libav root directory (if provided)
            ${libav_DIR}                  # libav root directory (if provided)
    )

    IF (MSVC)     # Windows
        SET(CMAKE_FIND_LIBRARY_PREFIXES ";lib")
        SET(CMAKE_FIND_LIBRARY_SUFFIXES ".lib;.a")
    ELSE (MSVC)   # Linux
        SET(CMAKE_FIND_LIBRARY_PREFIXES "lib")
        SET(CMAKE_FIND_LIBRARY_SUFFIXES ".so;.a")
    ENDIF(MSVC)

    FIND_LIBRARY(libav_AVDEVICE
        NAMES
	    avdevice
        PATHS
            /usr/lib64/                   # Default Fedora28 library path
            /usr/lib/                     # Some more Linux library path
            /usr/lib/x86_64-linux-gnu/    # Some more Linux library path
            /usr/local/lib/               # Some more Linux library path
            /usr/local/lib64/             # Some more Linux library path
            ${libav_DIR}/libavdevice/     # libav root directory (if provided)
            ${libav_DIR}/lib/             # libav root directory (if provided)
            ${libav_DIR}                  # libav root directory (if provided)
            ${CMAKE_MODULE_PATH}/lib/     # Expected to contain the path to this file for Windows10
    )
    FIND_LIBRARY(libav_AVFORMAT
        NAMES
	    avformat
        PATHS
            /usr/lib64/                   # Default Fedora28 library path
            /usr/lib/                     # Some more Linux library path
            /usr/lib/x86_64-linux-gnu/    # Some more Linux library path
            /usr/local/lib/               # Some more Linux library path
            /usr/local/lib64/             # Some more Linux library path
            ${libav_DIR}/libavformat/     # libav root directory (if provided)
            ${libav_DIR}/lib/             # libav root directory (if provided)
            ${libav_DIR}                  # libav root directory (if provided)
            ${CMAKE_MODULE_PATH}/lib/     # Expected to contain the path to this file for Windows10
    )
    FIND_LIBRARY(libav_AVFILTER
        NAMES
	    avfilter
        PATHS
            /usr/lib64/                   # Default Fedora28 library path
            /usr/lib/                     # Some more Linux library path
            /usr/lib/x86_64-linux-gnu/    # Some more Linux library path
            /usr/local/lib/               # Some more Linux library path
            /usr/local/lib64/             # Some more Linux library path
            ${libav_DIR}/libavfilter/     # libav root directory (if provided)
            ${libav_DIR}/lib/             # libav root directory (if provided)
            ${libav_DIR}                  # libav root directory (if provided)
            ${CMAKE_MODULE_PATH}/lib/     # Expected to contain the path to this file for Windows10
    )
    FIND_LIBRARY(libav_AVCODEC
        NAMES
	    avcodec
        PATHS
            /usr/lib64/                   # Default Fedora28 library path
            /usr/lib/                     # Some more Linux library path
            /usr/lib/x86_64-linux-gnu/    # Some more Linux library path
            /usr/local/lib/               # Some more Linux library path
            /usr/local/lib64/             # Some more Linux library path
            ${libav_DIR}/libavcodec/      # libav root directory (if provided)
            ${libav_DIR}/lib/             # libav root directory (if provided)
            ${libav_DIR}                  # libav root directory (if provided)
            ${CMAKE_MODULE_PATH}/lib/     # Expected to contain the path to this file for Windows10
    )
    FIND_LIBRARY(libav_SWRESAMPLE
        NAMES
	    swresample
        PATHS
            /usr/lib64/                   # Default Fedora28 library path
            /usr/lib/                     # Some more Linux library path
            /usr/lib/x86_64-linux-gnu/    # Some more Linux library path
            /usr/local/lib/               # Some more Linux library path
            /usr/local/lib64/             # Some more Linux library path
            ${libav_DIR}/libswresample/   # libav root directory (if provided)
            ${libav_DIR}/lib/             # libav root directory (if provided)
            ${libav_DIR}                  # libav root directory (if provided)
            ${CMAKE_MODULE_PATH}/lib/     # Expected to contain the path to this file for Windows10
    )
    FIND_LIBRARY(libav_SWSCALE
        NAMES
	    swscale
        PATHS
            /usr/lib64/                   # Default Fedora28 library path
            /usr/lib/                     # Some more Linux library path
            /usr/lib/x86_64-linux-gnu/    # Some more Linux library path
            /usr/local/lib/               # Some more Linux library path
            /usr/local/lib64/             # Some more Linux library path
            ${libav_DIR}/libswscale/      # libav root directory (if provided)
            ${libav_DIR}/lib/             # libav root directory (if provided)
            ${libav_DIR}                  # libav root directory (if provided)
            ${CMAKE_MODULE_PATH}/lib/     # Expected to contain the path to this file for Windows10
    )
    FIND_LIBRARY(libav_AVUTIL
        NAMES
	    avutil
        PATHS
            /usr/lib64/                   # Default Fedora28 library path
            /usr/lib/                     # Some more Linux library path
            /usr/lib/x86_64-linux-gnu/    # Some more Linux library path
            /usr/local/lib/               # Some more Linux library path
            /usr/local/lib64/             # Some more Linux library path
            ${libav_DIR}/libavutil/       # libav root directory (if provided)
            ${libav_DIR}/lib/             # libav root directory (if provided)
            ${libav_DIR}                  # libav root directory (if provided)
            ${CMAKE_MODULE_PATH}/lib/     # Expected to contain the path to this file for Windows10
    )
    set(
	libav_LIBRARIES
	${libav_AVDEVICE}
	${libav_AVFORMAT}
	${libav_AVFILTER}
	${libav_AVCODEC}
	${libav_SWRESAMPLE}
	${libav_SWSCALE}
	${libav_AVUTIL}
    )
ENDIF (NOT libav_INCLUDE_DIRS OR NOT libav_LIBRARIES)

IF (libav_INCLUDE_DIRS AND libav_LIBRARIES)
    SET(libav_FOUND TRUE)
ELSE (libav_INCLUDE_DIRS AND libav_LIBRARIES)
    SET(libav_FOUND FALSE)
ENDIF (libav_INCLUDE_DIRS AND libav_LIBRARIES)

IF (libav_FIND_REQUIRED AND NOT libav_FOUND)
    MESSAGE(FATAL_ERROR
            "  libav not found.\n"
            "      Windows: Fill CMake variable libav_DIR to the provided directory.\n"
            "      Linux: Install libav using your package manager ($> sudo dnf install libav-devel).\n"
            )
ENDIF (libav_FIND_REQUIRED AND NOT libav_FOUND)

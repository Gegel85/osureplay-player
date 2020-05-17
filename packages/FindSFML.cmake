#
# A CMake module to find SFML
#
# Once done this module will define:
#  SFML_FOUND - system has SFML
#  SFML_INCLUDE_DIRS - the SFML include directory
#  SFML_LIBRARIES - Link these to use SFML
#

IF (NOT SFML_INCLUDE_DIRS OR NOT SFML_LIBRARIES)
    FIND_PATH(SFML_INCLUDE_DIRS
        NAMES
            SFML/Graphics.hpp
        PATHS
            /usr/include/                 # Default Fedora28 system include path
            /usr/local/include/           # Default Fedora28 local include path
            ${CMAKE_MODULE_PATH}/include/ # Expected to contain the path to this file for Windows10
            ${SFML_DIR}/include/         # SFML root directory (if provided)
            ${SFML_DIR}                  # SFML root directory (if provided)
    )

    IF (MSVC)     # Windows
        SET(CMAKE_FIND_LIBRARY_PREFIXES ";lib")
        SET(CMAKE_FIND_LIBRARY_SUFFIXES ".lib;.a")
    ELSE (MSVC)   # Linux
        SET(CMAKE_FIND_LIBRARY_PREFIXES "lib")
        SET(CMAKE_FIND_LIBRARY_SUFFIXES ".so;.a;.dll.a")
    ENDIF(MSVC)

    FIND_LIBRARY(SFML_AUDIO
        NAMES
            sfml-audio
        PATHS
            /usr/lib64/                   # Default Fedora28 library path
            /usr/lib/                     # Some more Linux library path
            /usr/lib/x86_64-linux-gnu/    # Some more Linux library path
            /usr/local/lib/               # Some more Linux library path
            /usr/local/lib64/             # Some more Linux library path
            ${SFML_DIR}/lib/             # SFML root directory (if provided)
            ${SFML_DIR}                  # SFML root directory (if provided)
            ${CMAKE_MODULE_PATH}/lib/     # Expected to contain the path to this file for Windows10
    )
    FIND_LIBRARY(SFML_GRAPHICS
        NAMES
            sfml-graphics
        PATHS
            /usr/lib64/                   # Default Fedora28 library path
            /usr/lib/                     # Some more Linux library path
            /usr/lib/x86_64-linux-gnu/    # Some more Linux library path
            /usr/local/lib/               # Some more Linux library path
            /usr/local/lib64/             # Some more Linux library path
            ${SFML_DIR}/lib/             # SFML root directory (if provided)
            ${SFML_DIR}                  # SFML root directory (if provided)
            ${CMAKE_MODULE_PATH}/lib/     # Expected to contain the path to this file for Windows10
    )
    FIND_LIBRARY(SFML_SYSTEM
        NAMES
            sfml-system
        PATHS
            /usr/lib64/                   # Default Fedora28 library path
            /usr/lib/                     # Some more Linux library path
            /usr/lib/x86_64-linux-gnu/    # Some more Linux library path
            /usr/local/lib/               # Some more Linux library path
            /usr/local/lib64/             # Some more Linux library path
            ${SFML_DIR}/lib/             # SFML root directory (if provided)
            ${SFML_DIR}                  # SFML root directory (if provided)
            ${CMAKE_MODULE_PATH}/lib/     # Expected to contain the path to this file for Windows10
    )
    FIND_LIBRARY(SFML_WINDOW
        NAMES
            sfml-window
        PATHS
            /usr/lib64/                   # Default Fedora28 library path
            /usr/lib/                     # Some more Linux library path
            /usr/lib/x86_64-linux-gnu/    # Some more Linux library path
            /usr/local/lib/               # Some more Linux library path
            /usr/local/lib64/             # Some more Linux library path
            ${SFML_DIR}/lib/             # SFML root directory (if provided)
            ${SFML_DIR}                  # SFML root directory (if provided)
            ${CMAKE_MODULE_PATH}/lib/     # Expected to contain the path to this file for Windows10
    )
    set(
	SFML_LIBRARIES
	${SFML_AUDIO}
	${SFML_GRAPHICS}
	${SFML_SYSTEM}
	${SFML_WINDOW}
    )
ENDIF (NOT SFML_INCLUDE_DIRS OR NOT SFML_LIBRARIES)

IF (SFML_INCLUDE_DIRS AND SFML_LIBRARIES)
    SET(SFML_FOUND TRUE)
ELSE (SFML_INCLUDE_DIRS AND SFML_LIBRARIES)
    SET(SFML_FOUND FALSE)
ENDIF (SFML_INCLUDE_DIRS AND SFML_LIBRARIES)

IF (SFML_FIND_REQUIRED AND NOT SFML_FOUND)
    MESSAGE(FATAL_ERROR
            "  SFML not found.\n"
            "      Windows: Fill CMake variable SFML_DIR to the provided directory.\n"
            "      Linux: Install SFML using your package manager ($> sudo dnf install sfml-devel).\n"
            )
ENDIF (SFML_FIND_REQUIRED AND NOT SFML_FOUND)

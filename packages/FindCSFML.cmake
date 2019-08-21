#
# A CMake module to find CSFML
#
# Once done this module will define:
#  CSFML_FOUND - system has CSFML
#  CSFML_INCLUDE_DIRS - the CSFML include directory
#  CSFML_LIBRARIES - Link these to use CSFML
#

IF (NOT CSFML_INCLUDE_DIRS OR NOT CSFML_LIBRARIES)
    FIND_PATH(CSFML_INCLUDE_DIRS
        NAMES
            SFML/Graphics.h
        PATHS
            /usr/include/                 # Default Fedora28 system include path
            /usr/local/include/           # Default Fedora28 local include path
            ${CMAKE_MODULE_PATH}/include/ # Expected to contain the path to this file for Windows10
            ${CSFML_DIR}/include/         # CSFML root directory (if provided)
            ${CSFML_DIR}                  # CSFML root directory (if provided)
    )

    IF (MSVC)     # Windows
        SET(CMAKE_FIND_LIBRARY_PREFIXES ";lib")
        SET(CMAKE_FIND_LIBRARY_SUFFIXES ".lib;.a")
    ELSE (MSVC)   # Linux
        SET(CMAKE_FIND_LIBRARY_PREFIXES "lib")
        SET(CMAKE_FIND_LIBRARY_SUFFIXES ".so;.a")
    ENDIF(MSVC)

    FIND_LIBRARY(CSFML_AUDIO
        NAMES
            csfml-audio
        PATHS
            /usr/lib64/                   # Default Fedora28 library path
            /usr/lib/                     # Some more Linux library path
            /usr/lib/x86_64-linux-gnu/    # Some more Linux library path
            /usr/local/lib/               # Some more Linux library path
            /usr/local/lib64/             # Some more Linux library path
            ${CSFML_DIR}/lib/             # CSFML root directory (if provided)
            ${CSFML_DIR}                  # CSFML root directory (if provided)
            ${CMAKE_MODULE_PATH}/lib/     # Expected to contain the path to this file for Windows10
    )
    FIND_LIBRARY(CSFML_GRAPHICS
        NAMES
            csfml-graphics
        PATHS
            /usr/lib64/                   # Default Fedora28 library path
            /usr/lib/                     # Some more Linux library path
            /usr/lib/x86_64-linux-gnu/    # Some more Linux library path
            /usr/local/lib/               # Some more Linux library path
            /usr/local/lib64/             # Some more Linux library path
            ${CSFML_DIR}/lib/             # CSFML root directory (if provided)
            ${CSFML_DIR}                  # CSFML root directory (if provided)
            ${CMAKE_MODULE_PATH}/lib/     # Expected to contain the path to this file for Windows10
    )
    FIND_LIBRARY(CSFML_SYSTEM
        NAMES
            csfml-system
        PATHS
            /usr/lib64/                   # Default Fedora28 library path
            /usr/lib/                     # Some more Linux library path
            /usr/lib/x86_64-linux-gnu/    # Some more Linux library path
            /usr/local/lib/               # Some more Linux library path
            /usr/local/lib64/             # Some more Linux library path
            ${CSFML_DIR}/lib/             # CSFML root directory (if provided)
            ${CSFML_DIR}                  # CSFML root directory (if provided)
            ${CMAKE_MODULE_PATH}/lib/     # Expected to contain the path to this file for Windows10
    )
    FIND_LIBRARY(CSFML_WINDOW
        NAMES
            csfml-window
        PATHS
            /usr/lib64/                   # Default Fedora28 library path
            /usr/lib/                     # Some more Linux library path
            /usr/lib/x86_64-linux-gnu/    # Some more Linux library path
            /usr/local/lib/               # Some more Linux library path
            /usr/local/lib64/             # Some more Linux library path
            ${CSFML_DIR}/lib/             # CSFML root directory (if provided)
            ${CSFML_DIR}                  # CSFML root directory (if provided)
            ${CMAKE_MODULE_PATH}/lib/     # Expected to contain the path to this file for Windows10
    )
    set(
	CSFML_LIBRARIES
	${CSFML_AUDIO}
	${CSFML_GRAPHICS}
	${CSFML_SYSTEM}
	${CSFML_WINDOW}
    )
ENDIF (NOT CSFML_INCLUDE_DIRS OR NOT CSFML_LIBRARIES)

IF (CSFML_INCLUDE_DIRS AND CSFML_LIBRARIES)
    SET(CSFML_FOUND TRUE)
ELSE (CSFML_INCLUDE_DIRS AND CSFML_LIBRARIES)
    SET(CSFML_FOUND FALSE)
ENDIF (CSFML_INCLUDE_DIRS AND CSFML_LIBRARIES)

IF (CSFML_FIND_REQUIRED AND NOT CSFML_FOUND)
    MESSAGE(FATAL_ERROR
            "  CSFML not found.\n"
            "      Windows: Fill CMake variable CSFML_DIR to the provided directory.\n"
            "      Linux: Install CSFML using your package manager ($> sudo dnf install csfml-devel).\n"
            )
ENDIF (CSFML_FIND_REQUIRED AND NOT CSFML_FOUND)

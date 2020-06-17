cmake_minimum_required(VERSION 3.12)
project(osureplay_player)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/packages)

if ("${CMAKE_BUILD_TYPE}" STREQUAL Debug)
	if (CMAKE_COMPILER_IS_GNUCC)
		add_compile_options(-Wall -Wextra -W -g -D_DEBUG)
	endif (CMAKE_COMPILER_IS_GNUCC)
endif ()
if ("${CMAKE_BUILD_TYPE}" STREQUAL Release)
	if (CMAKE_COMPILER_IS_GNUCC)
		add_compile_options(-DNDEBUG -O2)
	endif (CMAKE_COMPILER_IS_GNUCC)
endif ()

find_package(SFML REQUIRED)
find_package(libav REQUIRED)

include_directories(
	include
	lib/osumap-parser/include
	lib/osureplay-parser/include
	lib/osureplay-parser/lzma/src
	lib/osureplay-parser/lzma/src/pavlov
	lib/osureplay-parser/lzma/src/easylzma
	${SFML_INCLUDE_DIRS}
	${libav_INCLUDE_DIRS}
)

add_library(
	osureplay-parser SHARED
	lib/osureplay-parser/include/osu_replay_parser.h
	lib/osureplay-parser/src/osu_replay_parser.c
	lib/osureplay-parser/src/uncompress_lzma.c
	lib/osureplay-parser/lzma/src/common_internal.c
	lib/osureplay-parser/lzma/src/compress.c
	lib/osureplay-parser/lzma/src/decompress.c
	lib/osureplay-parser/lzma/src/lzip_header.c
	lib/osureplay-parser/lzma/src/lzma_header.c
	lib/osureplay-parser/lzma/src/pavlov/7zBuf.c
	lib/osureplay-parser/lzma/src/pavlov/7zBuf2.c
	lib/osureplay-parser/lzma/src/pavlov/7zCrc.c
	lib/osureplay-parser/lzma/src/pavlov/7zFile.c
	lib/osureplay-parser/lzma/src/pavlov/7zStream.c
	lib/osureplay-parser/lzma/src/pavlov/Alloc.c
	lib/osureplay-parser/lzma/src/pavlov/Bcj2.c
	lib/osureplay-parser/lzma/src/pavlov/Bra.c
	lib/osureplay-parser/lzma/src/pavlov/Bra86.c
	lib/osureplay-parser/lzma/src/pavlov/BraIA64.c
	lib/osureplay-parser/lzma/src/pavlov/LzFind.c
	lib/osureplay-parser/lzma/src/pavlov/LzmaDec.c
	lib/osureplay-parser/lzma/src/pavlov/LzmaEnc.c
	lib/osureplay-parser/lzma/src/pavlov/LzmaLib.c
)

add_library(
	osumap-parser SHARED
	lib/osumap-parser/include/osu_map_parser.h
	lib/osumap-parser/src/osu_map_parser.c
)

add_executable(
	osureplay_player
	src/main.cpp
	src/version.hpp
	src/ReplayPlayer.cpp
	src/ReplayPlayer.hpp
	src/Exceptions.hpp
	src/OsuSkin.cpp
	src/OsuSkin.hpp
	src/Sound.cpp
	src/Sound.hpp
	src/libav.hpp
	src/libav.cpp
	src/Utils.cpp
	src/Utils.hpp
	src/HitObjects/HitObject.cpp
	src/HitObjects/HitObject.hpp
	src/Rendering/RenderTarget.cpp
	src/Rendering/RenderTarget.hpp
	src/HitObjects/Slider.cpp
	src/HitObjects/Slider.hpp
	src/HitObjects/HitCircle.cpp
	src/HitObjects/HitCircle.hpp
	src/HitObjects/Spinner.cpp
	src/HitObjects/Spinner.hpp
	src/HitObjects/HitObjectFactory.cpp
	src/HitObjects/HitObjectFactory.hpp
	src/Rendering/SfmlWindowRenderTarget.cpp
	src/Rendering/SfmlWindowRenderTarget.hpp
	src/Rendering/LibAvRenderer.cpp
	src/Rendering/LibAvRenderer.hpp
	src/Sound/SoundManager.cpp
	src/Sound/SoundManager.hpp
	src/Sound/SfmlSoundManager.cpp
	src/Sound/SfmlSoundManager.hpp
	src/Sound/LibAvSoundManager.cpp src/Sound/LibAvSoundManager.hpp)

if (WIN32)
	if (MSVC)
		target_link_libraries(
			osureplay_player
			osumap-parser
			osureplay-parser
			${SFML_LIBRARIES}
			${libav_LIBRARIES}
			bcrypt
			ws2_32
			ole32
			secur32
		)
	else (MSVC)
		target_link_libraries(
			osureplay_player
			osumap-parser
			osureplay-parser
			${SFML_LIBRARIES}
			${libav_LIBRARIES}
			iconv
			bcrypt
			ws2_32
			ole32
			secur32
			z
			lzma
			bz2
		)
	endif (MSVC)
else (WIN32)
	target_link_libraries(
		osureplay_player
		pthread
		osumap-parser
		osureplay-parser
		${SFML_LIBRARIES}
		${libav_LIBRARIES}
		m
		z
	)
endif (WIN32)

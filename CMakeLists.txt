cmake_minimum_required(VERSION 3.12)
project(osureplay_player)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/packages)

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
)

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

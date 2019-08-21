cmake_minimum_required(VERSION 3.12)
project(osureplay_player C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/packages)

find_package(CSFML REQUIRED)
find_package(libav REQUIRED)

include_directories(
	include
	lib/concatf/include
	lib/osumap-parser/include
	lib/osureplay-parser/include
	lib/osureplay-parser/lzma/src
	lib/osureplay-parser/lzma/src/pavlov
	lib/osureplay-parser/lzma/src/easylzma
	${CSFML_INCLUDE_DIRS}
	${libav_INCLUDE_DIRS}
)

add_library(
	concatf
	lib/concatf/include/concatf.h
	lib/concatf/include/flags.h
	lib/concatf/sources/concat.c
	lib/concatf/sources/concatf.c
	lib/concatf/sources/float_to_str.c
	lib/concatf/sources/get_nbrlen.c
	lib/concatf/sources/int_to_str.c
	lib/concatf/sources/my_compute_power_rec_long.c
	lib/concatf/sources/my_putbase.c
	lib/concatf/sources/my_putnbr_base.c
	lib/concatf/sources/my_putstar.c
	lib/concatf/sources/my_putstr_stderr.c
	lib/concatf/sources/my_showstr.c
	lib/concatf/sources/my_strlen_rec.c
	lib/concatf/sources/str.c
	lib/concatf/sources/sub_strings.c
	lib/concatf/sources/uint_to_str.c
	include/replay_config.h)


add_library(
	osureplay-parser
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
	osumap-parser
	lib/osumap-parser/include/osu_map_parser.h
	lib/osumap-parser/src/osu_map_parser.c
)

add_executable(
	osureplay_player
	src/bezier.c
	src/decode_audio_file.c
	src/dict.c
	src/display_hit_object.c
	src/encode_audio.c
	src/encode_video.c
	src/frame_buffer.c
	src/get_slider_points.c
	src/globals.c
	src/load_skin.c
	src/main.c
	src/perfect_circle.c
	src/perfect_line.c
	src/replay_player.c
	src/sound.c
	src/utils.c
)

if (WIN32)
	target_link_libraries(
		${PROJECT_NAME}
		lzma
		concatf
		osumap-parser
		osureplay-parser
		${CSFML_LIBRARIES}
		${libav_LIBRARIES}
		iconv
		bcrypt
		ws2_32
		ole32
		secur32
		z
	)
else (WIN32)
	target_link_libraries(
		${PROJECT_NAME}
		lzma
		pthread
		concatf
		osumap-parser
		osureplay-parser
		${CSFML_LIBRARIES}
		${libav_LIBRARIES}
		m
		z
	)
endif (WIN32)

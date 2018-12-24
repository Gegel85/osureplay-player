cmake_minimum_required(VERSION 3.12)
project(osureplay_player C)

set(CMAKE_C_STANDARD 11)

include_directories(include)
include_directories(lib/concatf/include)
include_directories(lib/osumap-parser/include)
include_directories(lib/osureplay-parser/include)

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
)


add_library(
	osureplayparser
	lib/osureplay-parser/include/osu_replay_parser.h
	lib/osureplay-parser/src/osu_replay_parser.c
)

add_library(
	osumap-parser
	lib/osumap-parser/include/osu_map_parser.h
	lib/osumap-parser/src/osu_map_parser.c
)

add_executable(
	osureplay_player
	src/main.c
)

target_link_libraries(
	${PROJECT_NAME}
	lzma
	osumap-parser
	csfml-graphics
	csfml-window
	csfml-system
)
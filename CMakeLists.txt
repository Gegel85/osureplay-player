cmake_minimum_required(VERSION 3.12)
project(osureplay_player C)

set(CMAKE_C_STANDARD 11)

include_directories(include)
include_directories(lib/osumap-parser/include)
include_directories(lib/osureplay-parser/include)

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
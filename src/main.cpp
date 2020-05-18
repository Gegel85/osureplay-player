//
// Created by Gegel85 on 17/05/2020.
//

#include <iostream>
#include <filesystem>
#include "version.hpp"
#include "ReplayPlayer.hpp"

int main(int argc, char **argv)
{
	if (argc != 3) {
		std::cout << "Usage: " << argv[0] << " [map.osu] [replay.osr]" << std::endl;
		return EXIT_FAILURE;
	}

	OsuReplayPlayer::ReplayPlayer state(argv[1], argv[2]);

	std::cout << "OsuReplayPlayer v" << PLAYER_VERSION << std::endl << std::endl;

	std::cout << "Replay file " << argv[1] << ": " << std::endl;
	state.displayReplayInfos();

	std::cout << "Map file: " << argv[2] << std::endl;
	state.displayMapInfos();

	state.getSkin().addFolder("default_skin");
	state.getSkin().addFolder(std::filesystem::path(argv[2]).parent_path().string());
	return EXIT_SUCCESS;
}
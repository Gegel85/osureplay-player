//
// Created by Gegel85 on 17/05/2020.
//

#include <iostream>
#include <filesystem>
#include "version.hpp"
#include "ReplayPlayer.hpp"
#include "Rendering/SfmlWindowRenderTarget.hpp"

int main(int argc, char **argv)
{
	if (argc != 3) {
		std::cout << "Usage: " << argv[0] << " [map.osu] [replay.osr]" << std::endl;
		return EXIT_FAILURE;
	}

	OsuReplayPlayer::SFMLWindowRenderTarget target({640, 480}, "");
	OsuReplayPlayer::ReplayPlayer state(target, argv[1], argv[2]);

	target.setFramerateLimit(60);
	target.setTitle(std::string(state.getReplay().playerName) + " playing " + state.getBeatmap().metaData.artistUnicode + " (" + state.getBeatmap().metaData.difficulty + ") mapped by " + state.getBeatmap().metaData.creator);
	std::cout << "OsuReplayPlayer v" << PLAYER_VERSION << std::endl << std::endl;

	std::cout << "Replay file " << argv[1] << ": " << std::endl;
#ifdef _DEBUG
	state.displayReplayInfos();
#endif

	std::cout << "Map file: " << argv[2] << std::endl;
#ifdef _DEBUG
	state.displayMapInfos();
#endif

	state.getSkin().addFolder("default_skin");
	state.getSkin().addFolder(std::filesystem::path(argv[2]).parent_path().string());
	state.run();
	return EXIT_SUCCESS;
}
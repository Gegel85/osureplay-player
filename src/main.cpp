//
// Created by Gegel85 on 17/05/2020.
//

#include <iostream>
#include <filesystem>
#include <memory>
#include "version.hpp"
#include "ReplayPlayer.hpp"
#include "Rendering/SfmlWindowRenderTarget.hpp"
#include "Rendering/LibAvRenderer.hpp"
#include "Sound/SfmlSoundManager.hpp"
#include "Sound/LibAvSoundManager.hpp"

int main(int argc, char **argv)
{
	if (argc < 3 || argc > 4) {
		std::cout << "Usage: " << argv[0] << " [map.osu] [replay.osr]" << std::endl;
		return EXIT_FAILURE;
	}

	std::unique_ptr<OsuReplayPlayer::RenderTarget> target;
	std::unique_ptr<OsuReplayPlayer::SoundManager> manager;
	OsuReplayPlayer::SFMLWindowRenderTarget *sfmlTarget = nullptr;

	if (argc == 3) {
		sfmlTarget = new OsuReplayPlayer::SFMLWindowRenderTarget(sf::Vector2u{640, 480}, "");
		manager = std::make_unique<OsuReplayPlayer::SFMLSoundManager>();
		target.reset(sfmlTarget);
	} else if (argc == 4) {
		target = std::make_unique<OsuReplayPlayer::LibAvRenderer>(argv[3] + std::string(".mp4"), sf::Vector2u{640, 480}, 60, 4000000);
		manager = std::make_unique<OsuReplayPlayer::LibAvSoundManager>(argv[3] + std::string(".mp2"));
	}
	OsuReplayPlayer::ReplayPlayer state(*target, *manager, argv[1], argv[2]);

	if (sfmlTarget) {
		sfmlTarget->setFramerateLimit(60);
		sfmlTarget->setTitle(std::string(state.getReplay().playerName) + " playing " + state.getBeatmap().metaData.artistUnicode + " (" + state.getBeatmap().metaData.difficulty + ") mapped by " + state.getBeatmap().metaData.creator);
	}
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
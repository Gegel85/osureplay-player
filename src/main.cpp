//
// Created by Gegel85 on 17/05/2020.
//

#include <iostream>
#include <filesystem>
#include <memory>
#include "version.hpp"
#include "ReplayPlayer.hpp"
#include "Rendering/SfmlWindowRenderTarget.hpp"
#include "Sound/SfmlSoundManager.hpp"
#include "LibAvRendererSound.hpp"

int main(int argc, char **argv)
{
	if (argc < 3 || argc > 4) {
		std::cout << "Usage: " << argv[0] << " [map.osu] [replay.osr]" << std::endl;
		return EXIT_FAILURE;
	}

	std::cout << "OsuReplayPlayer v" << PLAYER_VERSION << std::endl << std::endl;
	std::cout << "Initializing..." << std::endl;

	std::unique_ptr<OsuReplayPlayer::RenderTarget> target;
	std::unique_ptr<OsuReplayPlayer::SoundManager> manager;
	std::unique_ptr<OsuReplayPlayer::LibAvRendererSound> libAv;
	std::map<std::string, std::string> opts;
	OsuReplayPlayer::ReplayPlayer *state;

	OsuReplayPlayer::VideoConfig vc{
		.opts = opts,
		.resolution = {640, 480},
		.bitRate = 1500000,
		.frameRate = 60,
	};
	OsuReplayPlayer::AudioConfig ac{
		.opts = opts,
		.bitRate = 64000,
		.sampleRate = 44100,
	};

	if (argc == 3) {
		auto *sfmlTarget = new OsuReplayPlayer::SFMLWindowRenderTarget(sf::Vector2u{640, 480}, "");

		manager = std::make_unique<OsuReplayPlayer::SFMLSoundManager>();
		target.reset(sfmlTarget);
		state = new OsuReplayPlayer::ReplayPlayer(*target, *manager, argv[1], argv[2]);

		const auto &bm = state->getBeatmap();

		sfmlTarget->setFramerateLimit(60);
		sfmlTarget->setTitle(std::string(state->getReplay().playerName) + " playing " + bm.metaData.artistUnicode + " (" + bm.metaData.difficulty + ") mapped by " + bm.metaData.creator);
	} else {
		libAv = std::make_unique<OsuReplayPlayer::LibAvRendererSound>(argv[3], vc, ac);
		state = new OsuReplayPlayer::ReplayPlayer(*libAv, *libAv, argv[1], argv[2]);
	}

#ifdef _DEBUG
	std::cout << "Replay file " << argv[2] << ": " << std::endl;
	state->displayReplayInfos();

	std::cout << "Map file: " << argv[1] << std::endl;
	state->displayMapInfos();
#endif

	state->getSkin().addFolder("default_skin");
	state->getSkin().addFolder(std::filesystem::path(argv[2]).parent_path().string());
	state->run();
	delete state;
	return EXIT_SUCCESS;
}
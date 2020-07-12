//
// Created by Gegel85 on 17/05/2020.
//

#include <iostream>
#include <getopt.h>
#include <filesystem>
#include <memory>
#include "version.hpp"
#include "ReplayPlayer.hpp"
#include "Rendering/SfmlWindowRenderTarget.hpp"
#include "Sound/SfmlSoundManager.hpp"
#include "LibAvRendererSound.hpp"
#include "Exceptions.hpp"

struct Parameters {
	OsuReplayPlayer::VideoConfig vc;
	OsuReplayPlayer::AudioConfig ac;
	OsuReplayPlayer::ReplayConfig rc;
	std::string beatmapPath;
	std::string replayPath;
	std::string outputPath;
	std::string skinPath;
	bool debug;
};

namespace OsuReplayPlayer
{
	unsigned getUInt(const std::string &elem)
	{
		try {
			return std::stoul(elem);
		} catch (...) {
			throw InvalidCommandLineArgumentException(elem + " is not a valid unsigned integer.");
		}
	}

	void showHelp()
	{
		std::cout << "\t--audio-bitrate=BITRATE"   << ", -a BITRATE"      << ":" << std::endl;
		std::cout << "\t\tChanges the audio bitrate of output video." << std::endl;
		std::cout << "\t\tDefault: 64000" << std::endl;
		std::cout << "\t\tIgnored in debug mode." << std::endl;

		std::cout << "\t--video-bitrate=BITRATE"   << ", -v BITRATE"      << ":" << std::endl;
		std::cout << "\t\tChanges the video bitrate of output video." << std::endl;
		std::cout << "\t\tDefault: 1500000" << std::endl;
		std::cout << "\t\tIgnored in debug mode." << std::endl;

		std::cout << "\t--samplerate=SAMPLERATE"                         << ":" << std::endl;
		std::cout << "\t\tChanges the audio samplerate of output video." << std::endl;
		std::cout << "\t\tDefault: 44100" << std::endl;
		std::cout << "\t\tIgnored in debug mode." << std::endl;

		std::cout << "\t--framerate=FRAMERATE"     << ", -f FRAMERATE"    << ":" << std::endl;
		std::cout << "\t\tChanges the framerate." << std::endl;
		std::cout << "\t\tDefault: 60" << std::endl;

		std::cout << "\t--resolution=WIDTHxHEIGHT" << ", -e WIDTHxHEIGHT" << ":" << std::endl;
		std::cout << "\t\tChanges the size of the window." << std::endl;
		std::cout << "\t\tDefault: 640x480" << std::endl;

		std::cout << "\t--debug"                   << ", -d"              << ":" << std::endl;
		std::cout << "\t\tEnables debug mode." << std::endl;

		std::cout << "\t--beatmap=PATH"            << ", -b PATH"         << ":" << std::endl;
		std::cout << "\t\tSets beatmap file." << std::endl;

		std::cout << "\t--replay=PATH"             << ", -r PATH"         << ":" << std::endl;
		std::cout << "\t\tSets replay file." << std::endl;

		std::cout << "\t--output=PATH"             << ", -o PATH"         << ":" << std::endl;
		std::cout << "\t\tSets video output file." << std::endl;
		std::cout << "\t\tIgnored in debug mode." << std::endl;

		std::cout << "\t--skin=PATH"               << ", -s PATH"         << ":" << std::endl;
		std::cout << "\t\tSets skin folder." << std::endl;

		std::cout << "\t--help"                    << ", -h"              << ":" << std::endl;
		std::cout << "\t\tShows this help dialog." << std::endl;
	}

	void parseArguments(int argc, char **argv, Parameters &params)
	{
		int option_index = 0;

		while (true) {
			struct option long_options[] = {
				{"audio-bitrate", required_argument, 0, 'a'},
				{"video-bitrate", required_argument, 0, 'v'},
				{"samplerate",    required_argument, 0,  1 },
				{"bgDim",         required_argument, 0,  2 },
				{"framerate",     required_argument, 0, 'f'},
				{"resolution",    required_argument, 0, 'e'},
				{"debug",         no_argument,       0, 'd'},
				{"beatmap",       required_argument, 0, 'b'},
				{"replay",        required_argument, 0, 'r'},
				{"output",        required_argument, 0, 'o'},
				{"skin",          required_argument, 0, 's'},
				{"help",          no_argument,       0, 'h'},
				{0,               0,                 0, 0}
			};

			char c = getopt_long(argc, argv, "a:v:s:f:r:db:r:ho:\1:\2:", long_options, &option_index);
			char *pos;

			if (c == -1)
				break;

			switch (c) {
			case 'a':
				params.ac.bitRate = getUInt(optarg);
				break;
			case 'v':
				params.vc.bitRate = getUInt(optarg);
				break;
			case 1:
				params.ac.sampleRate = getUInt(optarg);
				break;
			case 2:
				if (getUInt(optarg) > 100)
					throw InvalidCommandLineArgumentException("Bg dim cannot be higher than 100%");
				params.rc.bgDim = getUInt(optarg);
				break;
			case 'f':
				params.vc.frameRate = getUInt(optarg);
				params.rc.frameRate = params.vc.frameRate;
				break;
			case 'e':
				pos = strchr(optarg, 'x');
				if (!pos)
					throw InvalidCommandLineArgumentException("Invalid resolution format. Expected WIDTHxHEIGHT.");
				*pos = '\0';
				params.vc.resolution.x = getUInt(optarg);
				params.vc.resolution.y = getUInt(pos + 1);
				break;
			case 'd':
				params.debug = true;
				break;
			case 'b':
				if (!*optarg)
					throw InvalidCommandLineArgumentException("Beatmap path cannot be empty.");
				if (!params.beatmapPath.empty())
					throw InvalidCommandLineArgumentException("Beatmap path has already been specified.");
				params.beatmapPath = optarg;
				break;
			case 'r':
				if (!*optarg)
					throw InvalidCommandLineArgumentException("Replay path cannot be empty.");
				if (!params.replayPath.empty())
					throw InvalidCommandLineArgumentException("Replay path has already been specified.");
				params.replayPath = optarg;
				break;
			case 'o':
				if (!*optarg)
					throw InvalidCommandLineArgumentException("Video output path cannot be empty.");
				if (!params.outputPath.empty())
					throw InvalidCommandLineArgumentException("Video output path has already been specified.");
				params.outputPath = optarg;
				break;
			case 's':
				if (!*optarg)
					throw InvalidCommandLineArgumentException("Skin path cannot be empty.");
				if (!params.skinPath.empty())
					throw InvalidCommandLineArgumentException("Skin path has already been specified.");
				params.skinPath = optarg;
				break;
			case 'h':
				showHelp();
				exit(EXIT_SUCCESS);
			case '?':
				throw InvalidCommandLineArgumentException(std::string(argv[optind]) + " is not a valid option.");
			}
		}

		if (params.beatmapPath.empty())
			throw InvalidCommandLineArgumentException("Beatmap path not specified.");
		if (params.replayPath.empty())
			throw InvalidCommandLineArgumentException("Replay path not specified.");
		if (params.outputPath.empty() && !params.debug)
			throw InvalidCommandLineArgumentException("Video output path not specified.");
	}
}

#ifndef _DEBUG

#include "Utils.hpp"

int realMain(int argc, char **argv);

int main(int argc, char **argv)
{
	try {
		return realMain(argc, argv);
	} catch (std::exception &e) {
		std::cerr << "An unexpected error occured:" << std::endl;
		std::cerr << OsuReplayPlayer::Utils::getLastExceptionName() << ": " << std::endl;
		std::cerr << "\t" << e.what() << std::endl;
		return EXIT_FAILURE;
	}
}

int realMain(int argc, char **argv)
#else
int main(int argc, char **argv)
#endif
{
	std::cout << "OsuReplayPlayer v" << PLAYER_VERSION << std::endl << std::endl;

	std::unique_ptr<OsuReplayPlayer::RenderTarget> target;
	std::unique_ptr<OsuReplayPlayer::SoundManager> manager;
	std::unique_ptr<OsuReplayPlayer::LibAvRendererSound> libAv;
	std::map<std::string, std::string> opts;
	OsuReplayPlayer::ReplayPlayer *state;

	Parameters params = {
		.vc = {
			.opts = opts,
			.resolution = {640, 480},
			.bitRate = 1500000,
			.frameRate = 60,
		},
		.ac = {
			.opts = opts,
			.bitRate = 64000,
			.sampleRate = 44100,
		},
		.rc = {
			.frameRate = 60,
			.bgDim = 0
		},
		.beatmapPath = "",
		.replayPath = "",
		.outputPath = "",
		.skinPath = "",
		.debug = false
	};

	try {
		OsuReplayPlayer::parseArguments(argc, argv, params);
	} catch (OsuReplayPlayer::InvalidCommandLineArgumentException &e) {
		std::cerr << "Error: " << e.what() << std::endl;
		std::cerr << "Use \" " << argv[0] << " -h \" for more information" << std::endl;
		return EXIT_FAILURE;
	}

	std::vector<std::string> skins{
		"default_skin"
	};

	if (!params.skinPath.empty())
		skins.push_back(params.skinPath);
	skins.push_back(std::filesystem::path(params.beatmapPath).parent_path().string());

	std::cout << "Initializing..." << std::endl;
	if (params.debug) {
		auto *sfmlTarget = new OsuReplayPlayer::SFMLWindowRenderTarget(params.vc.resolution, "");

		manager = std::make_unique<OsuReplayPlayer::SFMLSoundManager>();
		target.reset(sfmlTarget);
		state = new OsuReplayPlayer::ReplayPlayer(*target, *manager, params.beatmapPath, params.replayPath, params.rc, skins);

		const auto &bm = state->getBeatmap();

		sfmlTarget->setFramerateLimit(params.rc.frameRate);
		sfmlTarget->setTitle(std::string(state->getReplay().playerName) + " playing " + bm.metaData.artist + " - " + bm.metaData.title + " (" + bm.metaData.difficulty + ") mapped by " + bm.metaData.creator);
	} else {
		libAv = std::make_unique<OsuReplayPlayer::LibAvRendererSound>(params.outputPath, params.vc, params.ac);
		state = new OsuReplayPlayer::ReplayPlayer(*libAv, *libAv, params.beatmapPath, params.replayPath, params.rc, skins);
	}

#ifdef _DEBUG
	std::cout << "Replay file " << params.replayPath << ": " << std::endl;
	state->displayReplayInfos();

	std::cout << "Map file: " << params.beatmapPath << std::endl;
	state->displayMapInfos();
#endif

	state->run();
	delete state;
	return EXIT_SUCCESS;
}
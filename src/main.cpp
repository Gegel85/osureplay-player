//
// Created by Gegel85 on 17/05/2020.
//

#include <iostream>
#include <osu_map_parser.hpp>
#include <osu_replay_parser.hpp>
#include <iomanip>
#include <numeric>
#include "version.hpp"

void displayMapInfos(const OsuMap &beatmap)
{
	std::cout << "[General infos]" << std::endl;
	std::cout << "Audio file name: " << beatmap.generalInfos.audioFileName << std::endl;
	std::cout << "Audio lead in: " << beatmap.generalInfos.audioLeadIn << std::endl;
	std::cout << "Preview time: " << std::boolalpha << beatmap.generalInfos.previewTime << std::endl;
	std::cout << "Mode: " << beatmap.generalInfos.mode << std::endl;
	std::cout << "Countdown: " << std::boolalpha << beatmap.generalInfos.countdown << std::noboolalpha << std::endl;
	std::cout << "Hit sounds sample set: " << beatmap.generalInfos.hitSoundsSampleSet << std::endl;
	std::cout << "Stack leniency: " << beatmap.generalInfos.stackLeniency << std::endl;
	std::cout << "Letter box in breaks: " << std::boolalpha << beatmap.generalInfos.letterBoxInBreaks << std::endl;
	std::cout << "Widescreen storyboard: " << beatmap.generalInfos.widescreenStoryboard << std::endl;
	std::cout << "Story fire in front: " << beatmap.generalInfos.storyFireInFront << std::endl;
	std::cout << "Special style: " << beatmap.generalInfos.specialStyle << std::endl;
	std::cout << "Epilepsy warning: " << beatmap.generalInfos.epilepsyWarning << std::endl;
	std::cout << "Use skin sprites: " << beatmap.generalInfos.useSkinSprites << std::noboolalpha << std::endl;

	std::cout << "[Editor infos]" << std::endl;
	std::cout << beatmap.editorInfos.bookmarks.length << " bookmarks";
	if (beatmap.editorInfos.bookmarks.length)
		std::cout << ": " << std::accumulate(
			beatmap.editorInfos.bookmarks.content + 1,
			beatmap.editorInfos.bookmarks.content + beatmap.editorInfos.bookmarks.length,
			std::to_string(*beatmap.editorInfos.bookmarks.content),
			[](const std::string &a, unsigned int b) {
				return a + ", " + std::to_string(b);
			}
		);
	std::cout << "." << std::endl;
	std::cout << "Distance spacing: " << beatmap.editorInfos.distanceSpacing << std::endl;
	std::cout << "Beat division: " << beatmap.editorInfos.beatDivision << std::endl;
	std::cout << "Grid size: " << beatmap.editorInfos.gridSize << std::endl;
	std::cout << "Timeline zoom: " << beatmap.editorInfos.timeLineZoom << std::endl;

	std::cout << "[Meta data]" << std::endl;
	std::cout << "Title: " << beatmap.metaData.title << std::endl;
	std::cout << "Unicode title: " << beatmap.metaData.unicodeTitle << std::endl;
	std::cout << "Artist: " << beatmap.metaData.artist << std::endl;
	std::cout << "Artist unicode: " << beatmap.metaData.artistUnicode << std::endl;
	std::cout << "Creator: " << beatmap.metaData.creator << std::endl;
	std::cout << "Difficulty: " << beatmap.metaData.difficulty << std::endl;
	std::cout << "Music origin: " << beatmap.metaData.musicOrigin << std::endl;

	unsigned len = 0;
	char buffer[8];

	while (beatmap.metaData.tags[len])
		len++;

	std::cout << len << " tags";
	if (len)
		std::cout << ": " << std::accumulate(
			beatmap.metaData.tags + 1,
			beatmap.metaData.tags + len,
			std::string(*beatmap.metaData.tags),
			[](const std::string &a, char *b) {
				return a + ", " + b;
			}
		);
	std::cout << "."<< std::endl;
	std::cout << "Beatmap ID: " << beatmap.metaData.beatmapID << std::endl;
	std::cout << "Beatmap set ID: " << beatmap.metaData.beatmapSetID << std::endl;

	std::cout << "[Difficulty infos]" << std::endl;
	std::cout << "Hp drain rate: " << beatmap.difficulty.hpDrainRate << std::endl;
	std::cout << "Circle size: " << beatmap.difficulty.circleSize << std::endl;
	std::cout << "Overall difficulty: " << beatmap.difficulty.overallDifficulty << std::endl;
	std::cout << "Approach rate: " << beatmap.difficulty.approachRate << std::endl;
	std::cout << "Slider multiplayer: " << beatmap.difficulty.sliderMultiplayer << std::endl;
	std::cout << "Slider tick rate: " << beatmap.difficulty.sliderTickRate << std::endl;

	std::cout << "[Color infos]" << std::endl;
	std::cout << beatmap.colors.length << " colors";
	if (beatmap.colors.length) {
		sprintf(buffer, "#%02x%02x%02x", beatmap.colors.content->red, beatmap.colors.content->green, beatmap.colors.content->blue);
		std::cout << ": " << std::accumulate(
			beatmap.colors.content + 1,
			beatmap.colors.content + beatmap.colors.length,
			std::string(buffer),
			[&buffer](const std::string &a, OsuMap_color b) {
				sprintf(buffer, "#%02x%02x%02x", b.red, b.green, b.blue);
				return a + ", " + buffer;
			}
		);
	}
	std::cout << "."<< std::endl;

	std::cout << "[Timing points]" << std::endl;
	std::cout << beatmap.timingPoints.length << " timing points." << std::endl;

	std::cout << "[Storyboard]" << std::endl;
	std::cout << "Background path: " << beatmap.storyBoard.backgroundPath << std::endl;
	std::cout << "Video path: " << beatmap.storyBoard.videoPath << std::endl;
	std::cout << beatmap.storyBoard.storyboardEvents.length << " events." << std::endl;
}

void displayReplayInfos(const OsuReplay &replay)
{
	std::cout << "Player: " << replay.playerName << std::endl;
	std::cout << "Mode: " << std::setfill('0') << std::setw(2) << std::hex << static_cast<int>(replay.mode) << std::dec << std::endl;
	std::cout << "Version: " << replay.version << std::endl;
	std::cout << "MapHash: " << replay.mapHash << std::endl;
	std::cout << "ReplayHash: " << replay.replayHash << std::endl;
	std::cout << "300s: " << replay.score.nbOf300 << std::endl;
	std::cout << "100s: " << replay.score.nbOf100 << std::endl;
	std::cout << "50s: " << replay.score.nbOf50 << std::endl;
	std::cout << "Gekis: " << replay.score.nbOfGekis << std::endl;
	std::cout << "Katus: " << replay.score.nbOfKatus << std::endl;
	std::cout << "Misses: " << replay.score.nbOfMiss << std::endl;
	std::cout << "Total score: " << replay.score.totalScore << std::endl;
	std::cout << "Max combo: " << replay.score.maxCombo << std::endl;
	std::cout << "Is combo broke: " << std::boolalpha << replay.score.noComboBreak << std::noboolalpha << std::endl;
	std::cout << "Mods: " << replay.mods << std::endl;
	std::cout << "Timestamp: " << std::hex << replay.timestamp << std::endl;
	std::cout << "Something: " << replay.something << std::endl;
	std::cout << "Replay length: " << std::dec << replay.replayLength << std::endl << std::endl;
}

int main(int argc, char **argv)
{
	if (argc != 3) {
		std::cout << "Usage: " << argv[0] << " [map.osu] [replay.osr]" << std::endl;
		return EXIT_FAILURE;
	}

	OsuMap beatmap = OsuMap_parseMapFile(argv[1]);

	if (beatmap.error) {
		std::cout << "Beatmap file (" << argv[1] << ") is not valid: " << beatmap.error << std::endl;
		return EXIT_FAILURE;
	}

	OsuReplay replay = OsuReplay_parseReplayFile(argv[2]);

	if (replay.error) {
		std::cout << "Replay file (" << argv[2] << ") is not valid: " << replay.error << std::endl;
		return EXIT_FAILURE;
	}

	std::cout << "OsuReplayPlayer v" << PLAYER_VERSION << std::endl << std::endl;

	std::cout << "Replay file " << argv[1] << ": " << std::endl;
	displayReplayInfos(replay);

	std::cout << "Map file: " << argv[2] << std::endl;
	displayMapInfos(beatmap);
	return EXIT_SUCCESS;
}
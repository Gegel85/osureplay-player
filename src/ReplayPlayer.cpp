//
// Created by Gegel85 on 17/05/2020.
//

#include <iostream>
#include <numeric>
#include <iomanip>
#include <filesystem>
#include "ReplayPlayer.hpp"
#include "Exceptions.hpp"
#include "HitObjects/HitObjectFactory.hpp"

namespace OsuReplayPlayer
{
	ReplayPlayer::ReplayPlayer(RenderTarget &target, SoundManager &sound, const std::string &beatmapPath, const std::string &replayPath, unsigned fps) :
		_fps(fps),
		_target(target),
		_sound(sound)
	{
		std::cout << "Loading replay file " << replayPath << std::endl;
		this->_replay = OsuReplay_parseReplayFile(replayPath.c_str());
		if (this->_replay.error) {
			std::cerr << "Replay file (" << replayPath << ") is not valid: " << this->_replay.error << std::endl;
			throw InvalidReplayException(this->_replay.error);
		}
		this->_controller.setEvents(this->_replay.gameEvents);

		std::cout << "Loading beatmap file " << beatmapPath << std::endl;
		this->_beatmap = OsuMap_parseMapFile(beatmapPath.c_str());
		if (this->_beatmap.error) {
			std::cerr << "Beatmap file (" << beatmapPath << ") is not valid: " << this->_beatmap.error << std::endl;
			throw InvalidBeatmapException(this->_beatmap.error);
		}

		std::cout << "Preparing player" << std::endl;
		this->_buildHitObjects();

		this->_totalFrames = this->_replay.replayLength * fps / ((this->_replay.mods & MODE_DOUBLE_TIME) ? 1500 : 1000) + 1;
		this->_skin.addSound(std::filesystem::path(beatmapPath).parent_path().append(this->_beatmap.generalInfos.audioFileName).string(), "__bgMusic");
	}

	ReplayPlayer::~ReplayPlayer()
	{
		OsuReplay_destroy(&this->_replay);
		OsuMap_destroy(&this->_beatmap);
	}

	void ReplayPlayer::displayMapInfos()
	{
		std::cout << "File version: " << this->_beatmap.fileVersion << std::endl;
		std::cout << "[General infos]" << std::endl;
		std::cout << "Audio file name: " << this->_beatmap.generalInfos.audioFileName << std::endl;
		std::cout << "Audio lead in: " << this->_beatmap.generalInfos.audioLeadIn << std::endl;
		std::cout << "Preview time: " << std::boolalpha << this->_beatmap.generalInfos.previewTime << std::endl;
		std::cout << "Mode: " << this->_beatmap.generalInfos.mode << std::endl;
		std::cout << "Countdown: " << std::boolalpha << this->_beatmap.generalInfos.countdown << std::noboolalpha << std::endl;
		std::cout << "Hit sounds sample set: " << this->_beatmap.generalInfos.hitSoundsSampleSet << std::endl;
		std::cout << "Stack leniency: " << this->_beatmap.generalInfos.stackLeniency << std::endl;
		std::cout << "Letter box in breaks: " << std::boolalpha << this->_beatmap.generalInfos.letterBoxInBreaks << std::endl;
		std::cout << "Widescreen storyboard: " << this->_beatmap.generalInfos.widescreenStoryboard << std::endl;
		std::cout << "Story fire in front: " << this->_beatmap.generalInfos.storyFireInFront << std::endl;
		std::cout << "Special style: " << this->_beatmap.generalInfos.specialStyle << std::endl;
		std::cout << "Epilepsy warning: " << this->_beatmap.generalInfos.epilepsyWarning << std::endl;
		std::cout << "Use skin sprites: " << this->_beatmap.generalInfos.useSkinSprites << std::noboolalpha << std::endl;

		std::cout << "[Editor infos]" << std::endl;
		std::cout << this->_beatmap.editorInfos.bookmarks.length << " bookmarks";
		if (this->_beatmap.editorInfos.bookmarks.length)
			std::cout << ": " << std::accumulate(
				this->_beatmap.editorInfos.bookmarks.content + 1,
				this->_beatmap.editorInfos.bookmarks.content + this->_beatmap.editorInfos.bookmarks.length,
				std::to_string(*this->_beatmap.editorInfos.bookmarks.content),
				[](const std::string &a, unsigned int b) {
					return a + ", " + std::to_string(b);
				}
			);
		std::cout << "." << std::endl;
		std::cout << "Distance spacing: " << this->_beatmap.editorInfos.distanceSpacing << std::endl;
		std::cout << "Beat division: " << this->_beatmap.editorInfos.beatDivision << std::endl;
		std::cout << "Grid size: " << this->_beatmap.editorInfos.gridSize << std::endl;
		std::cout << "Timeline zoom: " << this->_beatmap.editorInfos.timeLineZoom << std::endl;

		std::cout << "[Meta data]" << std::endl;
		std::cout << "Title: " << this->_beatmap.metaData.title << std::endl;
		std::cout << "Unicode title: " << this->_beatmap.metaData.unicodeTitle << std::endl;
		std::cout << "Artist: " << this->_beatmap.metaData.artist << std::endl;
		std::cout << "Artist unicode: " << this->_beatmap.metaData.artistUnicode << std::endl;
		std::cout << "Creator: " << this->_beatmap.metaData.creator << std::endl;
		std::cout << "Difficulty: " << this->_beatmap.metaData.difficulty << std::endl;
		std::cout << "Music origin: " << this->_beatmap.metaData.musicOrigin << std::endl;

		unsigned len = 0;
		char buffer[8];

		while (this->_beatmap.metaData.tags[len])
			len++;

		std::cout << len << " tags";
		if (len)
			std::cout << ": " << std::accumulate(
				this->_beatmap.metaData.tags + 1,
				this->_beatmap.metaData.tags + len,
				std::string(*this->_beatmap.metaData.tags),
				[](const std::string &a, char *b) {
					return a + ", " + b;
				}
			);
		std::cout << "."<< std::endl;
		std::cout << "Beatmap ID: " << this->_beatmap.metaData.beatmapID << std::endl;
		std::cout << "Beatmap set ID: " << this->_beatmap.metaData.beatmapSetID << std::endl;

		std::cout << "[Difficulty infos]" << std::endl;
		std::cout << "Hp drain rate: " << this->_beatmap.difficulty.hpDrainRate << std::endl;
		std::cout << "Circle size: " << this->_beatmap.difficulty.circleSize << std::endl;
		std::cout << "Overall difficulty: " << this->_beatmap.difficulty.overallDifficulty << std::endl;
		std::cout << "Approach rate: " << this->_beatmap.difficulty.approachRate << std::endl;
		std::cout << "Slider multiplayer: " << this->_beatmap.difficulty.sliderMultiplayer << std::endl;
		std::cout << "Slider tick rate: " << this->_beatmap.difficulty.sliderTickRate << std::endl;

		std::cout << "[Color infos]" << std::endl;
		std::cout << this->_beatmap.colors.length << " colors";
		if (this->_beatmap.colors.length) {
			sprintf(buffer, "#%02x%02x%02x", this->_beatmap.colors.content->red, this->_beatmap.colors.content->green, this->_beatmap.colors.content->blue);
			std::cout << ": " << std::accumulate(
				this->_beatmap.colors.content + 1,
				this->_beatmap.colors.content + this->_beatmap.colors.length,
				std::string(buffer),
				[&buffer](const std::string &a, OsuMap_color b) {
					sprintf(buffer, "#%02x%02x%02x", b.red, b.green, b.blue);
					return a + ", " + buffer;
				}
			);
		}
		std::cout << "."<< std::endl;

		std::cout << "[Timing points]" << std::endl;
		std::cout << this->_beatmap.timingPoints.length << " timing points." << std::endl;

		std::cout << "[Storyboard]" << std::endl;
		std::cout << "Background path: " << this->_beatmap.storyBoard.backgroundPath << std::endl;
		std::cout << "Video path: " << (this->_beatmap.storyBoard.videoPath ?: "Not specified") << std::endl;
		std::cout << this->_beatmap.storyBoard.storyboardEvents.length << " events." << std::endl;
	}

	void ReplayPlayer::displayReplayInfos()
	{
		std::cout << "Player: " << this->_replay.playerName << std::endl;
		std::cout << "Mode: " << std::setfill('0') << std::setw(2) << std::hex << static_cast<int>(this->_replay.mode) << std::dec << std::endl;
		std::cout << "Version: " << this->_replay.version << std::endl;
		std::cout << "MapHash: " << this->_replay.mapHash << std::endl;
		std::cout << "ReplayHash: " << this->_replay.replayHash << std::endl;
		std::cout << "300s: " << this->_replay.score.nbOf300 << std::endl;
		std::cout << "100s: " << this->_replay.score.nbOf100 << std::endl;
		std::cout << "50s: " << this->_replay.score.nbOf50 << std::endl;
		std::cout << "Gekis: " << this->_replay.score.nbOfGekis << std::endl;
		std::cout << "Katus: " << this->_replay.score.nbOfKatus << std::endl;
		std::cout << "Misses: " << this->_replay.score.nbOfMiss << std::endl;
		std::cout << "Total score: " << this->_replay.score.totalScore << std::endl;
		std::cout << "Max combo: " << this->_replay.score.maxCombo << std::endl;
		std::cout << "Is combo broke: " << std::boolalpha << this->_replay.score.noComboBreak << std::noboolalpha << std::endl;
		std::cout << "Mods: " << this->_replay.mods << std::endl;
		std::cout << "Timestamp: " << std::hex << this->_replay.timestamp << std::endl;
		std::cout << "Something: " << this->_replay.something << std::endl;
		std::cout << "Replay length: " << std::dec << this->_replay.replayLength << std::endl << std::endl;
	}

	OsuSkin &ReplayPlayer::getSkin()
	{
		return this->_skin;
	}

	void ReplayPlayer::_buildHitObjects()
	{
		MapState state{
			.lastComboNbr = 1,
			.lastColor = 0,
			.gameMode = this->_replay.mode,
			.skin = this->_skin,
			.colors = this->_beatmap.colors,
			.infos = this->_beatmap.difficulty,
		};

		this->_objs.clear();
		for (unsigned i = 0; i < this->_beatmap.hitObjects.length; i++)
			this->_objs.push_back(HitObjectFactory::build(this->_beatmap.hitObjects.content[i], state));
	}

	void ReplayPlayer::run()
	{
		unsigned currentFrame = 0;

		this->_state.timingPt = *this->_beatmap.timingPoints.content;

		while (this->_target.isValid() && this->_state.currentGameHitObject < this->_objs.size()) {
			if (!this->_musicStarted && this->_beatmap.generalInfos.audioLeadIn <= this->_state.elapsedTime) {
				this->_musicStarted = true;
				this->_sound.playSound(this->_skin.getSound("__bgMusic"), (this->_replay.mods & MODE_DOUBLE_TIME) || (this->_replay.mods & MODE_NIGHTCORE) ? 1.5 : 1.);
			}

			this->_target.clear(sf::Color::Black);
			for (int i = this->_getLastObjToDisplay() - 1; static_cast<unsigned>(i) >= this->_state.currentGameHitObject && i >= 0; i--)
				this->_objs[i]->draw(this->_target, this->_state);

			this->_drawCursor();

			std::cout << "Rendering frame " << currentFrame++ << "/" << this->_totalFrames << std::endl;
			this->_target.renderFrame();
			this->_sound.tick(currentFrame, this->_fps);

			this->_controller.update(this->_state.elapsedTime);
			this->_state.elapsedTime = ((this->_replay.mods & MODE_DOUBLE_TIME) || (this->_replay.mods & MODE_NIGHTCORE) ? 1500.f : 1000.f) * currentFrame / this->_fps;
			this->_updateState();
		}
	}

	unsigned ReplayPlayer::_getLastObjToDisplay()
	{
		unsigned end = this->_state.currentGameHitObject;

		while (true) {
			if (end >= this->_objs.size())
				break;

			auto time = this->_objs[end]->getTimeToAppear();

			if (time >= 800 && time - 800 > this->_state.elapsedTime)
				break;
			end++;
		}
		return end;
	}

	const OsuMap &ReplayPlayer::getBeatmap() const
	{
		return this->_beatmap;
	}

	const OsuReplay &ReplayPlayer::getReplay() const
	{
		return this->_replay;
	}

	void ReplayPlayer::_updateState()
	{
		while (this->_state.currentGameHitObject < this->_objs.size() && this->_objs[this->_state.currentGameHitObject]->hasExpired(this->_state))
			this->_state.currentGameHitObject++;

		while (
			this->_state.currentTimingPt < this->_beatmap.timingPoints.length - 1 &&
			this->_beatmap.timingPoints.content[this->_state.currentTimingPt + 1].timeToHappen < this->_state.elapsedTime
		) {
			this->_state.currentTimingPt++;
			this->_state.timingPt = this->_beatmap.timingPoints.content[this->_state.currentTimingPt];
		}
	}

	void ReplayPlayer::_drawCursor()
	{
		auto pos = this->_controller.getPosition();

		if (pos.x > 0 && pos.y > 0)
			this->_target.drawImage(
				{static_cast<int>(pos.x), static_cast<int>(pos.y)},
				this->_skin.getImage("cursor"),
				{-1, -1},
				{255, 255, 255, 255},
				true,
				0
			);
	}
}
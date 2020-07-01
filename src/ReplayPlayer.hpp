//
// Created by Gegel85 on 17/05/2020.
//

#ifndef OSUREPLAY_PLAYER_REPLAYPLAYER_HPP
#define OSUREPLAY_PLAYER_REPLAYPLAYER_HPP


#include <string>
#include <osu_map_parser.hpp>
#include <osu_replay_parser.hpp>
#include "OsuSkin.hpp"
#include "HitObjects/HitObject.hpp"
#include "Sound/SoundManager.hpp"
#include "Controller/ReplayController.hpp"
#include "OsuParticle.hpp"

namespace OsuReplayPlayer
{
	struct ReplayState {
		bool clicked = false;
		bool M1clicked = false;
		bool M2clicked = false;
		bool K1clicked = false;
		bool K2clicked = false;
		double elapsedTime = 0;
		sf::Vector2f cursorPos;
		unsigned currentGameHitObject = 0;
		unsigned currentTimingPt = 0;
		unsigned combo = 0;
		bool perfectCombo = true;
		OsuMap_timingPointEvent timingPt;
	};

	class ReplayPlayer {
	private:
		sf::Vector2i _bgPos;
		sf::Vector2<int> _bgSize;
		ReplayController _controller;
		bool _musicStarted = false;
		unsigned _fps;
		OsuSkin _skin;
		OsuMap _beatmap;
		OsuReplay _replay;
		ReplayState _state;
		RenderTarget &_target;
		SoundManager &_sound;
		unsigned _totalFrames;
		std::vector<OsuParticle> _particles;
		std::vector<std::unique_ptr<HitObject>> _objs;

		void _buildHitObjects();
		unsigned _getLastObjToDisplay();
		void _updateState();
		void _drawCursor();
		void _drawBackground();
		void _updateParticles();
		void _updateCursorState();
		void _onExpire(HitObject &obj);
		void _drawScoreResult(HitObject &obj);

	public:
		ReplayPlayer(RenderTarget &target, SoundManager &sound, const std::string &beatmapPath, const std::string &replayPath, unsigned fps = 60);
		~ReplayPlayer();
		void displayMapInfos();
		void displayReplayInfos();
		OsuSkin &getSkin();
		const OsuMap &getBeatmap() const;
		const OsuReplay &getReplay() const;
		void run();
	};
}


#endif //OSUREPLAY_PLAYER_REPLAYPLAYER_HPP

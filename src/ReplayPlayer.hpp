//
// Created by Gegel85 on 17/05/2020.
//

#ifndef OSUREPLAY_PLAYER_REPLAYPLAYER_HPP
#define OSUREPLAY_PLAYER_REPLAYPLAYER_HPP


#include <string>
#include <osu_map_parser.hpp>
#include <osu_replay_parser.hpp>

namespace OsuReplayPlayer
{
	class ReplayPlayer {
	private:
		OsuMap _beatmap;
		OsuReplay _replay;

	public:
		ReplayPlayer(const std::string &beatmapPath, const std::string &replayPath);
		~ReplayPlayer();
		void displayMapInfos();
		void displayReplayInfos();
	};
}


#endif //OSUREPLAY_PLAYER_REPLAYPLAYER_HPP

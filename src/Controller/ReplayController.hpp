//
// Created by Gegel85 on 19/06/2020.
//

#ifndef OSUREPLAY_PLAYER_REPLAYCONTROLLER_HPP
#define OSUREPLAY_PLAYER_REPLAYCONTROLLER_HPP


#include <osu_replay_parser.hpp>
#include "Controller.hpp"

namespace OsuReplayPlayer
{
	class ReplayController : public Controller {
	private:
		std::vector<OsuGameEvent> _events;
		unsigned _currentEvent = 0;

	public:
		ReplayController() = default;
		ReplayController(const OsuGameEventArray &events);
		void setEvents(const OsuGameEventArray &events);
		sf::Vector2f getPosition() const override;
		bool isSmokePressed() const override;
		bool isK1Pressed() const override;
		bool isK2Pressed() const override;
		bool isM1Pressed() const override;
		bool isM2Pressed() const override;
		void update(float timeElapsed, const std::function<void (float time)> &onClick, const std::function<void (float time)> &onMove) override;
	};
}


#endif //OSUREPLAY_PLAYER_REPLAYCONTROLLER_HPP

//
// Created by Gegel85 on 19/06/2020.
//

#include "ReplayController.hpp"

namespace OsuReplayPlayer
{
	ReplayController::ReplayController(const OsuGameEventArray &events) :
		_events(events.content, events.content + events.length)
	{
	}

	sf::Vector2f ReplayController::getPosition() const
	{
		const auto &pos = this->_events[this->_currentEvent].cursorPos;

		return {
			pos.x,
			pos.y
		};
	}

	bool ReplayController::isSmokePressed() const
	{
		return this->_events[this->_currentEvent].keysPressed & INPUT_SMOKE;
	}

	bool ReplayController::isK1Pressed() const
	{
		return this->_events[this->_currentEvent].keysPressed & INPUT_K1;
	}

	bool ReplayController::isK2Pressed() const
	{
		return this->_events[this->_currentEvent].keysPressed & INPUT_K2;
	}

	bool ReplayController::isM1Pressed() const
	{
		return this->_events[this->_currentEvent].keysPressed & INPUT_M1;
	}

	bool ReplayController::isM2Pressed() const
	{
		return this->_events[this->_currentEvent].keysPressed & INPUT_M2;
	}

	void ReplayController::update(float timeElapsed)
	{
		while (
			this->_currentEvent < this->_events.size() - 1 &&
			this->_events[this->_currentEvent + 1].timeToHappen <= timeElapsed
		)
			this->_currentEvent++;
	}

	void ReplayController::setEvents(const OsuGameEventArray &events)
	{
		this->_events.clear();
		this->_events.assign(events.content, events.content + events.length);
	}
}
//
// Created by Gegel85 on 19/06/2020.
//

#ifndef OSUREPLAY_PLAYER_CONTROLLER_HPP
#define OSUREPLAY_PLAYER_CONTROLLER_HPP


#include <SFML/Graphics.hpp>

namespace OsuReplayPlayer
{
	class Controller {
	private:
		sf::Vector2f oldPos;

	public:
		bool isPressed() const
		{
			return this->isK1Pressed() || this->isK2Pressed() || this->isM1Pressed() || this->isM2Pressed();
		}

		virtual sf::Vector2f getPosition() const = 0;
		virtual bool isSmokePressed() const = 0;
		virtual bool isK1Pressed() const = 0;
		virtual bool isK2Pressed() const = 0;
		virtual bool isM1Pressed() const = 0;
		virtual bool isM2Pressed() const = 0;
		virtual void update(float timeElapsed, const std::function<void (float time)> &onClick, const std::function<void (float time)> &onMove) = 0;
	};
}


#endif //OSUREPLAY_PLAYER_CONTROLLER_HPP

//
// Created by Gegel85 on 18/05/2020.
//

#include <cmath>
#include <iostream>
#include "HitCircle.hpp"
#include "../ReplayPlayer.hpp"

namespace OsuReplayPlayer::HitObjects
{
	HitCircle::HitCircle(const OsuMap_hitObject &obj, MapState &state, bool endsCombo) :
		HitObject(obj, state, endsCombo)
	{
	}

	void HitCircle::draw(RenderTarget &target, const ReplayState &state)
	{
		unsigned char alpha = this->calcAlpha(state.elapsedTime);
		double radius = this->getRadius();

		target.drawImage(
			{
				this->getPosition().x,
				this->getPosition().y
			},
			this->_skin.getImage("hitcircle"),
			{
				static_cast<int>(radius * 2),
				static_cast<int>(radius * 2)
			},
			{
				this->_color.red,
				this->_color.green,
				this->_color.blue,
				alpha
			},
			true,
			0
		);
		target.drawImage(
			{
				this->getPosition().x,
				this->getPosition().y
			},
			this->_skin.getImage("hitcircleoverlay"),
			{
				static_cast<int>(radius * 2),
				static_cast<int>(radius * 2)
			},
			{255, 255, 255, alpha},
			true,
			0
		);
		this->_displayApproachCircle(target, radius, alpha, state.elapsedTime);
		this->_displayComboNumber(target, alpha);
	}

	void HitCircle::update(const ReplayState &state)
	{
		auto dist =
			std::pow(state.cursorPos.x - this->getPosition().x, 2) +
			std::pow(state.cursorPos.y - this->getPosition().y, 2);

		if (std::pow(this->getRadius(), 2) < dist)
			return;
		if (!state.clicked)
			return;

		double diff = std::abs(this->getTimeToAppear() - state.elapsedTime);

		this->_clicked = true;
		if (diff < 50 + 30 * (5 - this->_difficulty.overallDifficulty) / 5)
			this->_gainedScore = 300;
		else if (diff < 100 + 40 * (5 - this->_difficulty.overallDifficulty) / 5)
			this->_gainedScore = 100;
		else if (diff < 150 + 50 * (5 - this->_difficulty.overallDifficulty) / 5)
			this->_gainedScore = 50;
	}
}
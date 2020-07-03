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
				static_cast<float>(radius * 2),
				static_cast<float>(radius * 2)
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
				static_cast<float>(radius * 2),
				static_cast<float>(radius * 2)
			},
			{255, 255, 255, alpha},
			true,
			0
		);
		this->_displayApproachCircle(target, radius, alpha, state.elapsedTime);
		this->_displayComboNumber(target, alpha);
	}

	void HitCircle::update(const ReplayState &)
	{
	}

	void HitCircle::click(const ReplayState &state)
	{
		auto dist =
			std::pow(state.cursorPos.x - this->getPosition().x, 2) +
			std::pow(state.cursorPos.y - this->getPosition().y, 2);
		auto radius = std::pow(this->getRadius(), 2);

		if (radius < dist)
			return;

		double diff = std::abs(this->getTimeToAppear() - state.elapsedTime);

		this->_clicked = true;
		if (diff < 160 - 12 * this->_difficulty.overallDifficulty) {
			this->_gainedScore = 300;
			this->_brokeCombo = false;
		} else if (diff < 280 - 16 * this->_difficulty.overallDifficulty) {
			this->_gainedScore = 100;
			this->_brokeCombo = false;
		} else if (diff < 400 - 20 * this->_difficulty.overallDifficulty) {
			this->_gainedScore = 50;
			this->_brokeCombo = false;
		}
	}
}
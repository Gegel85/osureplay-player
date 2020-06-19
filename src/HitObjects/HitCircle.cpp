//
// Created by Gegel85 on 18/05/2020.
//

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
		double radius = 54.4 - 4.48 * this->_difficulty.circleSize;

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

	}
}
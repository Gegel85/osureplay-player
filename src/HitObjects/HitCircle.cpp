//
// Created by Gegel85 on 18/05/2020.
//

#include "HitCircle.hpp"
#include "../ReplayPlayer.hpp"

namespace OsuReplayPlayer::HitObjects
{
	HitCircle::HitCircle(const OsuMap_hitObject &obj, MapState &state) :
		HitObject(obj, state)
	{
	}

	void HitCircle::draw(RenderTarget &target, const ReplayState &state)
	{
		unsigned char alpha = this->calcAlpha(state.elapsedTime);
		double radius = 54.4 - 4.48 * this->_difficulty.circleSize;

		target.drawImage(
			{
				static_cast<int>(this->getPosition().x + radius),
				static_cast<int>(this->getPosition().y + radius)
			},
			this->_skin.getImage("hitcircle"),
			{
				static_cast<int>(radius * 2),
				static_cast<int>(radius * 2)
			},
			{
				static_cast<sf::Uint8>(this->_color.red * 0.5),
				static_cast<sf::Uint8>(this->_color.green * 0.5),
				static_cast<sf::Uint8>(this->_color.blue * 0.5),
				alpha
			},
			true,
			0
		);
		target.drawImage(
			{
				static_cast<int>(this->getPosition().x + radius),
				static_cast<int>(this->getPosition().y + radius)
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
		/*displayApproachCircle(
			frameBuffer,
			(sfColor){color.red, color.green, color.blue, alpha},
			object,
			circleSize,
			totalTicks,
			images
		);
		displayNumber(
			frameBuffer,
			combo,
			(sfVector2i) {
				object->position.x + padding.x,
				object->position.y + padding.y
			},
			images,
			alpha,
			15,
			"default"
		);*/
	}
}
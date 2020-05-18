//
// Created by Gegel85 on 18/05/2020.
//

#include "Slider.hpp"

namespace OsuReplayPlayer::HitObjects
{
	Slider::Slider(const OsuSkin &skin, const OsuMap_hitObject &obj, OsuGameMode gameMode, unsigned &lastComboNbr) :
		HitObject(skin, obj, gameMode, lastComboNbr)
	{

	}

	void Slider::draw(RenderTarget &target)
	{

	}
}
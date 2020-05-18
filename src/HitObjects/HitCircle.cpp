//
// Created by Gegel85 on 18/05/2020.
//

#include "HitCircle.hpp"

namespace OsuReplayPlayer::HitObjects
{
	HitCircle::HitCircle(const OsuSkin &skin, const OsuMap_hitObject &obj, OsuGameMode gameMode, unsigned &lastComboNbr) :
		HitObject(skin, obj, gameMode, lastComboNbr)
	{
	}

	void HitCircle::draw(RenderTarget &target)
	{

	}
}
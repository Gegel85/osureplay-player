//
// Created by Gegel85 on 18/05/2020.
//

#include "HitCircle.hpp"

namespace OsuReplayPlayer::HitObjects
{
	HitCircle::HitCircle(const OsuSkin &skin, const OsuMap_hitObject &obj, OsuGameMode gameMode, MapState &state) :
		HitObject(skin, obj, gameMode, state)
	{
	}

	void HitCircle::draw(RenderTarget &target)
	{

	}
}
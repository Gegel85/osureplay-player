//
// Created by Gegel85 on 18/05/2020.
//

#include "Spinner.hpp"

namespace OsuReplayPlayer::HitObjects
{

	Spinner::Spinner(const OsuSkin &skin, const OsuMap_hitObject &obj, OsuGameMode gameMode, unsigned &lastComboNbr) :
		HitObject(skin, obj, gameMode, lastComboNbr)
	{

	}

	void Spinner::draw(RenderTarget &target)
	{

	}
}
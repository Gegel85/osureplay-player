//
// Created by Gegel85 on 18/05/2020.
//

#ifndef OSUREPLAY_PLAYER_SPINNER_HPP
#define OSUREPLAY_PLAYER_SPINNER_HPP


#include "HitObject.hpp"

namespace OsuReplayPlayer::HitObjects
{
	class Spinner : public HitObject {
	public:
		Spinner(const OsuSkin &skin, const OsuMap_hitObject &obj, OsuGameMode gameMode, unsigned &lastComboNbr);

		void draw(RenderTarget &target) override;
	};
}


#endif //OSUREPLAY_PLAYER_SPINNER_HPP

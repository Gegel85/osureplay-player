//
// Created by Gegel85 on 18/05/2020.
//

#ifndef OSUREPLAY_PLAYER_SLIDER_HPP
#define OSUREPLAY_PLAYER_SLIDER_HPP


#include "HitObject.hpp"

namespace OsuReplayPlayer::HitObjects
{
	class Slider : public HitObject {
	public:
		Slider(const OsuSkin &skin, const OsuMap_hitObject &obj, OsuGameMode gameMode, unsigned &lastComboNbr);

		void draw(RenderTarget &target) override;
	};
}


#endif //OSUREPLAY_PLAYER_SLIDER_HPP

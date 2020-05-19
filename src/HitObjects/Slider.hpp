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
		Slider(const OsuMap_hitObject &obj, MapState &state);

		void draw(RenderTarget &target, const ReplayState &state) override;
	};
}


#endif //OSUREPLAY_PLAYER_SLIDER_HPP

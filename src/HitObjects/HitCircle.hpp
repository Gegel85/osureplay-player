//
// Created by Gegel85 on 18/05/2020.
//

#ifndef OSUREPLAY_PLAYER_HITCIRCLE_HPP
#define OSUREPLAY_PLAYER_HITCIRCLE_HPP


#include "HitObject.hpp"

namespace OsuReplayPlayer::HitObjects
{
	class HitCircle : public HitObject {
	public:
		HitCircle(const OsuMap_hitObject &obj, MapState &state, bool endsCombo);
		void draw(RenderTarget &target, const ReplayState &state) override;
		void click(const ReplayState &state) override;
		void update(const ReplayState &state) override;
	};
}


#endif //OSUREPLAY_PLAYER_HITCIRCLE_HPP

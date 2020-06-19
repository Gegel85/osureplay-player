//
// Created by Gegel85 on 18/05/2020.
//

#ifndef OSUREPLAY_PLAYER_SPINNER_HPP
#define OSUREPLAY_PLAYER_SPINNER_HPP


#include "HitObject.hpp"

namespace OsuReplayPlayer::HitObjects
{
	class Spinner : public HitObject {
	private:
		unsigned _end;

	public:
		Spinner(const OsuMap_hitObject &obj, MapState &state, bool endsCombo);

		void draw(RenderTarget &target, const ReplayState &state) override;
		bool hasExpired(ReplayState &state) override;
		void update(const ReplayState &state) override;
	};
}


#endif //OSUREPLAY_PLAYER_SPINNER_HPP

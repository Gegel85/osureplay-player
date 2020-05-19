//
// Created by Gegel85 on 18/05/2020.
//

#ifndef OSUREPLAY_PLAYER_HITOBJECTFACTORY_HPP
#define OSUREPLAY_PLAYER_HITOBJECTFACTORY_HPP


#include "HitObject.hpp"

namespace OsuReplayPlayer
{
	class HitObjectFactory {
	private:
		static const std::vector<std::function<HitObject *(const OsuMap_hitObject &obj, MapState &state)>> _builders;

	public:
		static std::unique_ptr<HitObject> build(const OsuMap_hitObject &obj, MapState &state);
	};
}


#endif //OSUREPLAY_PLAYER_HITOBJECTFACTORY_HPP

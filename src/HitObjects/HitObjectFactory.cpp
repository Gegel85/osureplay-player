//
// Created by Gegel85 on 18/05/2020.
//

#include "HitObjectFactory.hpp"
#include "HitCircle.hpp"
#include "Slider.hpp"
#include "Spinner.hpp"

namespace OsuReplayPlayer
{
	const std::vector<std::function<HitObject *(const OsuSkin &skin, const OsuMap_hitObject &obj, OsuGameMode gameMode, MapState &state)>> HitObjectFactory::_builders{
		[](const OsuSkin &skin, const OsuMap_hitObject &obj, OsuGameMode gameMode, MapState &state){ //HIT_OBJECT_CIRCLE
			return new HitObjects::HitCircle(skin, obj, gameMode, state);
		},
		[](const OsuSkin &skin, const OsuMap_hitObject &obj, OsuGameMode gameMode, MapState &state){ //HIT_OBJECT_SLIDER
			return new HitObjects::Slider(skin, obj, gameMode, state);
		},
		[](const OsuSkin &skin, const OsuMap_hitObject &obj, OsuGameMode gameMode, MapState &state){ //HIT_OBJECT_SPINNER
			return new HitObjects::Spinner(skin, obj, gameMode, state);
		},
		[](const OsuSkin &, const OsuMap_hitObject &, OsuGameMode, MapState &){ //HIT_OBJECT_MANIA_LONG_NOTE
			return nullptr;
		}
	};

	std::unique_ptr<HitObject> HitObjectFactory::build(const OsuSkin &skin, const OsuMap_hitObject &obj, OsuGameMode gameMode, MapState &state)
	{
		return std::unique_ptr<HitObject>(
			HitObjectFactory::_builders[getObjectTypeFromMapValue(obj.type)](skin, obj, gameMode, state)
		);
	}
}
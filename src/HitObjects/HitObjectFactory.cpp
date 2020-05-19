//
// Created by Gegel85 on 18/05/2020.
//

#include "HitObjectFactory.hpp"
#include "HitCircle.hpp"
#include "Slider.hpp"
#include "Spinner.hpp"

namespace OsuReplayPlayer
{
	const std::vector<std::function<HitObject *(const OsuMap_hitObject &obj, MapState &state)>> HitObjectFactory::_builders{
		[](const OsuMap_hitObject &obj, MapState &state){ //HIT_OBJECT_CIRCLE
			return new HitObjects::HitCircle(obj, state);
		},
		[](const OsuMap_hitObject &obj, MapState &state){ //HIT_OBJECT_SLIDER
			return new HitObjects::Slider(obj, state);
		},
		[](const OsuMap_hitObject &obj, MapState &state){ //HIT_OBJECT_SPINNER
			return new HitObjects::Spinner(obj, state);
		},
		[](const OsuMap_hitObject &, MapState &){ //HIT_OBJECT_MANIA_LONG_NOTE
			return nullptr;
		}
	};

	std::unique_ptr<HitObject> HitObjectFactory::build(const OsuMap_hitObject &obj, MapState &state)
	{
		return std::unique_ptr<HitObject>(
			HitObjectFactory::_builders[getObjectTypeFromMapValue(obj.type)](obj, state)
		);
	}
}
//
// Created by Gegel85 on 18/05/2020.
//

#include "HitObjectFactory.hpp"
#include "HitCircle.hpp"
#include "Slider.hpp"
#include "Spinner.hpp"

namespace OsuReplayPlayer
{
	const std::vector<std::function<HitObject *(const OsuMap_hitObject &obj, MapState &state, bool endsCombo)>> HitObjectFactory::_builders{
		[](const OsuMap_hitObject &obj, MapState &state, bool endsCombo){ //HIT_OBJECT_CIRCLE
			return new HitObjects::HitCircle(obj, state, endsCombo);
		},
		[](const OsuMap_hitObject &obj, MapState &state, bool endsCombo){ //HIT_OBJECT_SLIDER
			return new HitObjects::Slider(obj, state, endsCombo);
		},
		[](const OsuMap_hitObject &obj, MapState &state, bool endsCombo){ //HIT_OBJECT_SPINNER
			return new HitObjects::Spinner(obj, state, endsCombo);
		},
		[](const OsuMap_hitObject &, MapState &, bool){ //HIT_OBJECT_MANIA_LONG_NOTE
			return nullptr;
		}
	};

	std::unique_ptr<HitObject> HitObjectFactory::build(const OsuMap_hitObject &obj, MapState &state, bool endsCombo)
	{
		return std::unique_ptr<HitObject>(
			HitObjectFactory::_builders[getObjectTypeFromMapValue(obj.type)](obj, state, endsCombo)
		);
	}
}
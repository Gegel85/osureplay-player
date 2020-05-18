//
// Created by Gegel85 on 18/05/2020.
//

#include "HitObjectFactory.hpp"
#include "HitCircle.hpp"
#include "Slider.hpp"
#include "Spinner.hpp"

namespace OsuReplayPlayer
{
	const std::vector<std::function<HitObject *(const OsuSkin &skin, const OsuMap_hitObject &obj, OsuGameMode gameMode, unsigned &lastComboNbr)>> HitObjectFactory::_builders{
		[](const OsuSkin &skin, const OsuMap_hitObject &obj, OsuGameMode gameMode, unsigned &lastComboNbr){ //HIT_OBJECT_CIRCLE
			return new HitObjects::HitCircle(skin, obj, gameMode, lastComboNbr);
		},
		[](const OsuSkin &skin, const OsuMap_hitObject &obj, OsuGameMode gameMode, unsigned &lastComboNbr){ //HIT_OBJECT_SLIDER
			return new HitObjects::Slider(skin, obj, gameMode, lastComboNbr);
		},
		[](const OsuSkin &skin, const OsuMap_hitObject &obj, OsuGameMode gameMode, unsigned &lastComboNbr){ //HIT_OBJECT_SPINNER
			return new HitObjects::Spinner(skin, obj, gameMode, lastComboNbr);
		},
		[](const OsuSkin &, const OsuMap_hitObject &, OsuGameMode, unsigned &){ //HIT_OBJECT_MANIA_LONG_NOTE
			return nullptr;
		}
	};

	std::unique_ptr<HitObject> HitObjectFactory::build(const OsuSkin &skin, const OsuMap_hitObject &obj, OsuGameMode gameMode, unsigned &lastComboNbr)
	{
		return std::unique_ptr<HitObject>(
			HitObjectFactory::_builders[getObjectTypeFromMapValue(obj.type)](skin, obj, gameMode, lastComboNbr)
		);
	}
}
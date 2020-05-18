//
// Created by Gegel85 on 18/05/2020.
//

#ifndef OSUREPLAY_PLAYER_HITOBJECT_HPP
#define OSUREPLAY_PLAYER_HITOBJECT_HPP


#include <osu_map_parser.hpp>
#include <string>
#include "../OsuSkin.hpp"
#include "../Rendering/RenderTarget.hpp"

namespace OsuReplayPlayer
{
	enum HitObjectType {
		HIT_OBJECT_CIRCLE,
		HIT_OBJECT_SLIDER,
		HIT_OBJECT_SPINNER,
		HIT_OBJECT_MANIA_LONG_NOTE
	};

	HitObjectType getObjectTypeFromMapValue(unsigned char);

	class HitObject {
	private:
		bool _newCombo;
		HitObjectType _type;
		unsigned char _hitSound;
		unsigned char _colorSkip;
		unsigned long _timeToAppear;
		OsuIntegerVector _position;
		OsuMap_hitObjectAddition _extra;

	protected:
		OsuGameMode _gameMode;
		const OsuSkin &_skin;
		unsigned _comboNbr;

	public:
		HitObject(const OsuSkin &skin, const OsuMap_hitObject &obj, OsuGameMode gameMode, unsigned &lastComboNbr);

		bool isNewCombo() const;
		HitObjectType getType() const;
		unsigned char getHitSound() const;
		unsigned char getColorSkip() const;
		unsigned long getTimeToAppear() const;
		OsuIntegerVector getPosition() const;
		const OsuMap_hitObjectAddition &getExtra() const;

		virtual void draw(RenderTarget &target) = 0;
	};
}


#endif //OSUREPLAY_PLAYER_HITOBJECT_HPP

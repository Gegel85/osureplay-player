//
// Created by Gegel85 on 18/05/2020.
//

#include "HitObject.hpp"

namespace OsuReplayPlayer
{
	HitObjectType getObjectTypeFromMapValue(unsigned char val)
	{
		if (val & HITOBJ_SPINNER)
			return HIT_OBJECT_SPINNER;
		if (val & HITOBJ_SLIDER)
			return HIT_OBJECT_SLIDER;
		if (val & HITOBJ_MANIA_LONG_NOTE)
			return HIT_OBJECT_MANIA_LONG_NOTE;
		return HIT_OBJECT_CIRCLE;
	}

	HitObject::HitObject(const OsuMap_hitObject &obj, MapState &state) :
		_newCombo(obj.type & HITOBJ_NEW_COMBO),
		_type(getObjectTypeFromMapValue(obj.type)),
		_hitSound(obj.hitSound),
		_colorSkip((obj.type & HITOBJ_COLOR_SKIP_NBR) >> 4),
		_timeToAppear(obj.timeToAppear),
		_position(obj.position),
		_extra(obj.extra),
		_comboNbr(this->_newCombo ? (state.lastComboNbr = 1) : (++state.lastComboNbr)),
		_mods(0),
		_gameMode(state.gameMode),
		_color(this->_newCombo ? (state.colors.content[state.lastColor = (state.lastColor + 1 + this->_colorSkip) % state.colors.length]) : state.colors.content[state.lastColor]),
		_skin(state.skin),
		_difficulty(state.infos)
	{

	}

	bool HitObject::isNewCombo() const
	{
		return this->_newCombo;
	}

	HitObjectType HitObject::getType() const
	{
		return this->_type;
	}

	unsigned char HitObject::getHitSound() const
	{
		return this->_hitSound;
	}

	unsigned char HitObject::getColorSkip() const
	{
		return this->_colorSkip;
	}

	unsigned long HitObject::getTimeToAppear() const
	{
		return this->_timeToAppear;
	}

	OsuIntegerVector HitObject::getPosition() const
	{
		return this->_position;
	}

	const OsuMap_hitObjectAddition &HitObject::getExtra() const
	{
		return this->_extra;
	}

	unsigned char	HitObject::calcAlpha(unsigned long totalTicks)
	{
		if (static_cast<long>(this->_timeToAppear - totalTicks) <= 400)
			return BASE_OBJ_ALPHA;
		return static_cast<long>(this->_timeToAppear - totalTicks - 400) * -BASE_OBJ_ALPHA / 400 + BASE_OBJ_ALPHA;
	}
}
//
// Created by Gegel85 on 18/05/2020.
//

#ifndef OSUREPLAY_PLAYER_HITOBJECT_HPP
#define OSUREPLAY_PLAYER_HITOBJECT_HPP


#include <osu_map_parser.hpp>
#include <string>
#include "../OsuSkin.hpp"
#include "../Rendering/RenderTarget.hpp"

#define BASE_OBJ_ALPHA 255

namespace OsuReplayPlayer
{
	enum HitObjectType {
		HIT_OBJECT_CIRCLE,
		HIT_OBJECT_SLIDER,
		HIT_OBJECT_SPINNER,
		HIT_OBJECT_MANIA_LONG_NOTE
	};

	struct MapState {
		unsigned lastComboNbr;
		unsigned lastColor;
		OsuGameMode gameMode;
		const OsuSkin &skin;
		const OsuMap_colorArray &colors;
		const OsuMap_difficultyInfos &infos;
	};

	struct ReplayState;

	HitObjectType getObjectTypeFromMapValue(unsigned char);

	class HitObject {
	private:
		bool _newCombo;
		bool _endCombo;
		HitObjectType _type;
		unsigned char _hitSound;
		unsigned char _colorSkip;
		unsigned long _timeToAppear;
		OsuMap_hitObjectAddition _extra;

	protected:
		unsigned _gainedScore = 0;
		OsuIntegerVector _position;
		bool _clicked = false;
		bool _brokeCombo = true;
		unsigned _comboNbr;
		unsigned char _mods;
		OsuGameMode _gameMode;
		OsuMap_color _color;
		const OsuSkin &_skin;
		OsuMap_difficultyInfos _difficulty;

		void _displayApproachCircle(RenderTarget &target, float baseRadius, unsigned char alpha, unsigned long totalTicks);
		void _displayComboNumber(RenderTarget &target, unsigned char alpha);

	public:
		HitObject(const OsuMap_hitObject &obj, MapState &state, bool endsCombo);

		bool isEndCombo() const;
		bool isNewCombo() const;
		HitObjectType getType() const;
		unsigned char getHitSound() const;
		unsigned char getColorSkip() const;
		unsigned long getTimeToAppear() const;
		OsuIntegerVector getPosition() const;
		const OsuMap_hitObjectAddition &getExtra() const;
		unsigned char calcAlpha(unsigned long totalTicks);
		unsigned getScore() const;
		bool brokeCombo() const;
		double getRadius() const;
		virtual bool hasExpired(ReplayState &state) const;
		virtual OsuIntegerVector getScoreParticlePosition() const;

		virtual void draw(RenderTarget &target, const ReplayState &state) = 0;
		virtual void update(const ReplayState &state) = 0;
	};
}


#endif //OSUREPLAY_PLAYER_HITOBJECT_HPP

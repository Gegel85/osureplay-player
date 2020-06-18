//
// Created by Gegel85 on 18/05/2020.
//

#include "../ReplayPlayer.hpp"
#include "Spinner.hpp"

namespace OsuReplayPlayer::HitObjects
{

	Spinner::Spinner(const OsuMap_hitObject &obj, MapState &state) :
		HitObject(obj, state),
		_end(*reinterpret_cast<unsigned *>(obj.additionalInfos))
	{

	}

	void Spinner::draw(RenderTarget &target, const ReplayState &state)
	{
		size_t	duration = this->_end - this->getTimeToAppear();
		size_t	remaining = this->_end - state.elapsedTime;
		float	radius = this->getTimeToAppear() >= state.elapsedTime ? 400 : 4 * (100 - (duration - remaining) * 100.f / duration);

		target.drawImage(
			{320 ,240},
			this->_skin.getImage("spinner-approachcircle"),
			{
				static_cast<int>(radius),
				static_cast<int>(radius)
			},
			{255, 255, 255, calcAlpha(state.elapsedTime)},
			true,
			0
		);
	}

	bool Spinner::hasExpired(ReplayState &state)
	{
		return this->_end <= state.elapsedTime;
	}
}
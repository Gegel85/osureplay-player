//
// Created by Gegel85 on 18/05/2020.
//

#include <cmath>
#include "Slider.hpp"
#include "../ReplayPlayer.hpp"

namespace OsuReplayPlayer::HitObjects
{
	Slider::Slider(const OsuMap_hitObject &obj, MapState &state) :
		HitObject(obj, state)
	{
		auto *infos = reinterpret_cast<OsuMap_hitObjectSliderInfos *>(obj.additionalInfos);

		this->_pixelLength = infos->pixelLength;
		this->_nbOfRepeats = infos->nbOfRepeats;
		//TODO: Implement slider shapes
		this->_points.reserve(infos->curvePoints.length + 1);
		this->_points[0] = this->getPosition();
		for (unsigned i = 0; i < infos->curvePoints.length; i++)
			this->_points[i + 1] = infos->curvePoints.content[i];
		this->_edgeHitSounds.reserve(this->_nbOfRepeats + 1);
		this->_edgeAdditions.reserve(this->_nbOfRepeats + 1);
		for (unsigned i = 0; i < this->_nbOfRepeats + 1; i++) {
			this->_edgeHitSounds[i] = infos->edgeHitsounds[i];
			this->_edgeAdditions[i] = infos->edgeAdditions[i];
		}
		this->_end = infos->curvePoints.content[infos->curvePoints.length - 1];
	}

	void Slider::draw(RenderTarget &target, const ReplayState &state)
	{
		unsigned char alpha = this->calcAlpha(state.elapsedTime);
		double radius = 54.4 - 4.48 * this->_difficulty.circleSize;

		for (auto &point : this->_drawPoints)
			target.drawPixel(point, {this->_color.red, this->_color.green, this->_color.blue, alpha});

		target.drawImage(
			{
				this->_end.x,
				this->_end.y
			},
			this->_skin.getImage("sliderendcircle"),
			{
				static_cast<int>(radius * 2),
				static_cast<int>(radius * 2)
			},
			{
				this->_color.red,
				this->_color.green,
				this->_color.blue,
				alpha
			},
			true,
			0
		);

		target.drawImage(
			{
				this->getPosition().x,
				this->getPosition().y
			},
			this->_skin.hasImage("sliderstartcircle") ? this->_skin.getImage("sliderstartcircle") : this->_skin.getImage("hitcircle"),
			{
				static_cast<int>(radius * 2),
				static_cast<int>(radius * 2)
			},
			{
				this->_color.red,
				this->_color.green,
				this->_color.blue,
				alpha
			},
			true,
			0
		);

		target.drawImage(
			{
				this->getPosition().x,
				this->getPosition().y
			},
			this->_skin.hasImage("sliderstartcircleoverlay") ? this->_skin.getImage("sliderstartcircleoverlay") : this->_skin.getImage("hitcircleoverlay"),
			{
				static_cast<int>(radius * 2),
				static_cast<int>(radius * 2)
			},
			{255, 255, 255, alpha},
			true,
			0
		);

		if (this->getTimeToAppear() < state.elapsedTime) {
			double den = this->_pixelLength / (100 * this->_difficulty.sliderMultiplayer) * state.timingPt.millisecondsPerBeat;
			double ptId = this->_points.size() * std::fmod( state.elapsedTime - this->getTimeToAppear(), den) / den;
			sf::Vector2i currentPoint =  {
				this->_points[(int)ptId].x,
				this->_points[(int)ptId].y
			};

			target.drawImage(
				currentPoint,
				this->_skin.getImage("sliderb0"),
				{
					static_cast<int>(radius * 2),
					static_cast<int>(radius * 2)
				},
				{255, 255, 255, 255},
				true,
				0
			);

			target.drawImage(
				currentPoint,
				this->_skin.getImage("sliderfollowcircle"),
				{
					static_cast<int>(radius * 4),
					static_cast<int>(radius * 4)
				},
				{255, 255, 255, 255},
				true,
				0
			);
		} else
			this->_displayApproachCircle(target, radius, alpha, state.elapsedTime);

		this->_displayComboNumber(target, alpha);
	}
}
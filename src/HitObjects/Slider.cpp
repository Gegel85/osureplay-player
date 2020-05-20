//
// Created by Gegel85 on 18/05/2020.
//

#include <cmath>
#include "Slider.hpp"
#include "../ReplayPlayer.hpp"
#include "../Exceptions.hpp"

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
		this->_points.push_back(this->getPosition());
		for (unsigned i = 0; i < infos->curvePoints.length; i++)
			this->_points.push_back(infos->curvePoints.content[i]);
		this->_edgeHitSounds.reserve(this->_nbOfRepeats + 1);
		this->_edgeAdditions.reserve(this->_nbOfRepeats + 1);
		for (unsigned i = 0; i < this->_nbOfRepeats + 1; i++) {
			this->_edgeHitSounds.push_back(infos->edgeHitsounds[i]);
			this->_edgeAdditions.push_back(infos->edgeAdditions[i]);
		}
		this->_end = infos->curvePoints.content[infos->curvePoints.length - 1];
		this->_type = static_cast<OsuSliderShape>(infos->type);
		this->_makeCurve();
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
			auto len = this->_getTimeLength(state.timingPt);
			double ptId = this->_points.size() * std::fmod( state.elapsedTime - this->getTimeToAppear(), len) / len;

			sf::Vector2i currentPoint =  {
				this->_points[static_cast<int>(ptId)].x,
				this->_points[static_cast<int>(ptId)].y
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

	bool Slider::hasExpired(ReplayState &state)
	{
		return this->_clicked || this->getTimeToAppear() + this->_getTimeLength(state.timingPt) < state.elapsedTime;
	}

	double Slider::_getTimeLength(OsuMap_timingPointEvent timingPt) const
	{
		return this->_pixelLength / (100 * this->_difficulty.sliderMultiplayer) * timingPt.millisecondsPerBeat * this->_nbOfRepeats;
	}

	OsuIntegerVector Slider::_getBezierPoint(const std::vector<OsuIntegerVector> &points, double percent)
	{
		if (points.empty())
			return {0, 0};
		if (points.size() == 1)
			return points.back();

		std::vector<OsuIntegerVector> pts;

		pts.resize(points.size() - 1);
		for (unsigned i = 0; i < pts.size(); i++)
			pts[i] = {
				static_cast<int>(((points[i + 1].x - points[i].x) * percent / 100) + points[i].x),
				static_cast<int>(((points[i + 1].y - points[i].y) * percent / 100) + points[i].y)
			};
		return _getBezierPoint(pts, percent);
	}

	void Slider::_makeBezierCurve()
	{
		std::vector<std::vector<OsuIntegerVector>> pts;

		pts.emplace_back();
		for (auto &point : this->_points) {
			if (
				!pts.back().empty() &&
				point.x == pts.back().back().x &&
				point.y == pts.back().back().y
			)
				pts.emplace_back();
			pts.back().push_back(point);
		}
		this->_points.clear();
		this->_points.reserve(POINTS_PRECISION * pts.size());
		for (auto &pt : pts)
			for (unsigned j = 0; j < (pt.size() > 1 ? POINTS_PRECISION : 1); j++)
				this->_points.emplace_back(Slider::_getBezierPoint(pt, j / (POINTS_PRECISION / 100.)));
		this->_points.shrink_to_fit();
	}

	void Slider::_makeCurve()
	{
		switch (this->_type) {
		case SLIDER_SHAPE_BEZIER:
			this->_makeBezierCurve();
			break;
		case SLIDER_SHAPE_LINE:
			break;
		case SLIDER_SHAPE_PERFECT_CIRCLE:
			break;
		case SLIDER_SHAPE_CATMULL:
			break;
		default:
			throw InvalidSliderException(std::string("Invalid slider type '") + static_cast<char>(this->_type) + "'");
		}
	}
}
//
// Created by Gegel85 on 18/05/2020.
//

#include <cmath>
#include <iostream>
#include "Slider.hpp"
#include "../ReplayPlayer.hpp"
#include "../Exceptions.hpp"
#include "../Utils.hpp"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef M_PI_2
#define M_PI_2 1.57079632679489661923
#endif

template<typename _Tp>
struct __less : public std::binary_function<_Tp, _Tp, bool>
{
	constexpr bool operator()(const _Tp& v1, const _Tp& v2) const
	{
		size_t val1 = (static_cast<size_t>(v1.y) << 32U) | (static_cast<size_t>(v1.x) & 0xFFFFFFFFU);
		size_t val2 = (static_cast<size_t>(v2.y) << 32U) | (static_cast<size_t>(v2.x) & 0xFFFFFFFFU);

		return val1 < val2;
	}
};

namespace OsuReplayPlayer::HitObjects
{
	Slider::Slider(const OsuMap_hitObject &obj, MapState &state, bool endsCombo) :
		HitObject(obj, state, endsCombo)
	{
		auto *infos = reinterpret_cast<OsuMap_hitObjectSliderInfos *>(obj.additionalInfos);

		this->_pixelLength = infos->pixelLength;
		this->_nbOfRepeats = infos->nbOfRepeats;
		this->_points.reserve(infos->curvePoints.length + 1);
		this->_points.push_back(this->getPosition());
		for (unsigned i = 0; i < infos->curvePoints.length; i++)
			this->_points.push_back(infos->curvePoints.content[i]);
		this->_edgeHitSounds.reserve(this->_nbOfRepeats + 1);
		this->_edgeAdditions.reserve(this->_nbOfRepeats + 1);
		for (unsigned i = 0; i < this->_nbOfRepeats + 1; i++) {
			this->_edgeHitSounds.push_back(infos->edgeHitsounds ? infos->edgeHitsounds[i] : 0);
			this->_edgeAdditions.push_back(infos->edgeAdditions ? infos->edgeAdditions[i] : OsuMap_sampleSet{0, 0});
		}
		this->_end = infos->curvePoints.content[infos->curvePoints.length - 1];
		this->_type = static_cast<OsuSliderShape>(infos->type);
		this->_makeCurve();

		if (this->_angles.size() < this->_points.size())
			this->_angles.resize(this->_points.size());

		double radius = this->getRadius();
		double sqRadius = radius * radius;
		std::set<sf::Vector2i, __less<sf::Vector2i>> _temp;
		sf::Vector2i _bottomRight = {INT32_MIN, INT32_MIN};

		this->_topLeft = {INT32_MAX, INT32_MAX};

		for (auto &pt : this->_points)
			for (int x = -radius; x <= radius; x++)
				for (int y = -radius; y <= radius; y++)
					if (pow(x, 2) + pow(y, 2) <= sqRadius) {
						sf::Vector2i point(x + pt.x, y + pt.y);

						_temp.emplace(point);
						this->_topLeft.x = std::min(this->_topLeft.x, point.x);
						this->_topLeft.y = std::min(this->_topLeft.y, point.y);
						_bottomRight.x = std::max(_bottomRight.x, point.x);
						_bottomRight.y = std::max(_bottomRight.y, point.y);
					}
		this->_image.create(_bottomRight.x - this->_topLeft.x + 1, _bottomRight.y - this->_topLeft.y + 1, sf::Color::Transparent);
		for (auto &pt : _temp)
			this->_image.setPixel(pt.x - this->_topLeft.x, pt.y - this->_topLeft.y, {255, 255, 255, 255});
	}

	void Slider::draw(RenderTarget &target, const ReplayState &state)
	{
		unsigned char alpha = this->calcAlpha(state.elapsedTime);
		double radius = this->getRadius();
		auto len = this->_getTimeLength(state.timingPt);
		double ptsBetweenPoints = state.timingPt.millisecondsPerBeat * this->_points.size() / len;

		target.drawImage(
			{
				static_cast<int>(this->_topLeft.x),
				static_cast<int>(this->_topLeft.y),
			},
			this->_image,
			{-1, -1},
			{
				this->_color.red,
				this->_color.green,
				this->_color.blue,
				alpha
			},
			false,
			0
		);

		for (double i = ptsBetweenPoints; i < this->_points.size(); i += ptsBetweenPoints)
			target.drawImage(
				{
					this->_points[i].x,
					this->_points[i].y,
				},
				this->_skin.getImage("sliderscorepoint"),
				{-1, -1},
				{255, 255, 255, alpha},
				true,
				0
			);

		target.drawImage(
			{
				this->_end.x,
				this->_end.y
			},
			this->_skin.hasImage("sliderendcircle") ?
				this->_skin.getImage("sliderendcircle") :
				this->_skin.getImage("hitcircle"),
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
				this->_end.x,
				this->_end.y
			},
			this->_skin.hasImage("sliderendcircleoverlay") ?
				this->_skin.getImage("sliderendcircleoverlay") :
				this->_skin.getImage("hitcircleoverlay"),
			{
				static_cast<int>(radius * 2),
				static_cast<int>(radius * 2)
			},
			{255, 255, 255, alpha},
			true,
			0
		);

		target.drawImage(
			{
				this->getPosition().x,
				this->getPosition().y
			},
			this->_skin.hasImage("sliderstartcircle") ?
				this->_skin.getImage("sliderstartcircle") :
				this->_skin.getImage("hitcircle"),
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
			this->_skin.hasImage("sliderstartcircleoverlay") ?
				this->_skin.getImage("sliderstartcircleoverlay") :
				this->_skin.getImage("hitcircleoverlay"),
			{
				static_cast<int>(radius * 2),
				static_cast<int>(radius * 2)
			},
			{255, 255, 255, alpha},
			true,
			0
		);

		if (this->getTimeToAppear() < state.elapsedTime) {
			double ptId = this->_points.size() * std::fmod(state.elapsedTime - this->getTimeToAppear(), len) / len;

			sf::Vector2i currentPoint =  {
				this->_points[ptId].x,
				this->_points[ptId].y
			};

			if (!this->_skin.isImageSkinned("sliderb0"))
				target.drawImage(
					currentPoint,
					this->_skin.getImage("sliderb-nd"),
					{
						static_cast<int>(radius * 1.975),
						static_cast<int>(radius * 1.975)
					},
					{0, 0, 0,255},
					true,
					this->_angles.at(ptId) * 180 / M_PI
				);

			target.drawImage(
				currentPoint,
				this->_skin.getImage("sliderb" + std::to_string(this->_ballAnimation)),
				{
					static_cast<int>(radius * 1.975),
					static_cast<int>(radius * 1.975)
				},
				{
					this->_color.red,
					this->_color.green,
					this->_color.blue,
					255
				},
				true,
				this->_angles.at(ptId) * 180 / M_PI
			);

			if (!this->_skin.hasImage("sliderb" + std::to_string(++this->_ballAnimation)))
				this->_ballAnimation = 0;

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

	bool Slider::hasExpired(ReplayState &state) const
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
		this->_angles.resize(this->_points.size());
		this->_points.shrink_to_fit();
	}

	void Slider::_getLinePoints()
	{
		OsuIntegerVector diff;
		std::vector<OsuIntegerVector> array;

		if (this->_points.size() != 2)
			throw InvalidSliderException("Invalid linear slider: there is more than a single point");
		diff = {
			this->_points[1].x - this->_points[0].x,
			this->_points[1].y - this->_points[0].y
		};
		array.resize(sqrt(pow(diff.x, 2) + pow(diff.y, 2)));
		for (unsigned i = 0; i < array.size(); i++)
			array[i] = {
				static_cast<int>(diff.x * (static_cast<float>(i) / array.size()) + this->getPosition().x),
				static_cast<int>(diff.y * (static_cast<float>(i) / array.size()) + this->getPosition().y)
			};
		this->_angles.resize(array.size(), atan2(this->_points[1].y - this->_points[0].y, this->_points[1].x - this->_points[0].x));
		this->_points = array;
	}

	double	calcAngle(OsuIntegerVector pt, sf::Vector2<double> center)
	{
		sf::Vector2<double> point1(pt.x, pt.y);
		sf::Vector2<double> point2 = {center.x, center.y};
		double distance = sqrt(pow(point2.x - point1.x, 2) + pow(point2.y - point1.y, 2));
		sf::Vector2<double> vec2 = {(point2.x - point1.x) / distance, (point2.y - point1.y) / distance};

		return (atan2(vec2.y, vec2.x) * 180 / M_PI);
	}

	void Slider::_getCirclePoints()
	{
		std::vector<OsuIntegerVector> newArray;
		sf::Vector2<double> center;
		double radius;
		double angles[3];
		double arcAngle;
		double values[3];
		bool goClockwise;

		if (this->_points.size() != 3)
			throw InvalidSliderException("Invalid linear slider: there is more than 2 points");

		auto pt1 = this->_points[0];
		auto pt2 = this->_points[1];
		auto pt3 = this->_points[2];

		values[0] = ((pt2.x - pt1.x) * (pt3.y - pt2.y) + (pt2.y - pt1.y) * (pt2.x - pt3.x));
		if (!values[0]) {
			double dists[3] = {
				pow(pt1.x - pt2.x, 2) + pow(pt1.y - pt2.y, 2),
				pow(pt2.x - pt3.x, 2) + pow(pt2.y - pt3.y, 2),
				pow(pt1.x - pt3.x, 2) + pow(pt1.y - pt3.y, 2),
			};
			double best = fmax(fmax(dists[0], dists[1]), dists[2]);

			if (best == dists[0]) {
				this->_points.pop_back();
				return this->_getLinePoints();
			}
			if (best == dists[1]) {
				this->_points.erase(this->_points.begin());
				return this->_getLinePoints();
			}
			if (best == dists[2]) {
				this->_points.erase(this->_points.begin() + 1);
				return this->_getLinePoints();
			}
		}

		values[1] = (pow(pt2.x, 2) + pow(pt2.y, 2) - pow(pt3.x, 2) - pow(pt3.y, 2));
		values[2] = (pow(pt2.x, 2) + pow(pt2.y, 2) - pow(pt1.x, 2) - pow(pt1.y, 2));
		//Don't ask please
		center.y = (
			(pt2.x - pt3.x) * values[2] - (pt2.x - pt1.x) * values[1]
		) / (values[0] * 2);

		/*
		** Tbh, I don't know myself
		** That just worked so it's here
		** That's the center of the circle
		*/
		center.x = (
			(pt2.y - pt1.y) * values[1] - (pt2.y - pt3.y) * values[2]
		) / (values[0] * 2);

		//Calc the radius (quick maths)
		radius = sqrt(pow(pt1.x - center.x, 2) + pow(pt1.y - center.y, 2));

		//Calc angles
		angles[0] = fmod(360 - calcAngle(pt1, center), 360);
		angles[1] = fmod(360 - calcAngle(pt2, center), 360);
		angles[2] = fmod(360 - calcAngle(pt3, center), 360);

		//Check whether we go clockwise or not
		goClockwise = fmod(angles[1] - angles[0] + 360, 360) > fmod(angles[2] - angles[0] + 360, 360);

		//Calc the total arc angle
		arcAngle = fmod((goClockwise ? angles[0] - angles[2] : angles[2] - angles[0]) + 360, 360);

		//The size of the arc in pixels
		newArray.resize(M_PI_2 * radius * arcAngle / 360 + 1);
		this->_angles.reserve(newArray.size());

		//Create the arc
		for (size_t i = 0; i < newArray.size(); i++) {
			double val = static_cast<float>(i) / newArray.size();
			double angle = ((goClockwise ? angles[2] : angles[0]) + (arcAngle * (goClockwise ? 1 - val : val))) * M_PI / 180;

			this->_angles.push_back(goClockwise ? -angle - M_PI_2 : M_PI_2 - angle);
			newArray[i] = OsuIntegerVector{
				lround(-cos(angle) * radius + center.x),
				lround(sin(angle) * radius + center.y)
			};
		}
		this->_points = newArray;
	}

	void Slider::_makeCurve()
	{
		switch (this->_type) {
		case SLIDER_SHAPE_BEZIER:
			this->_makeBezierCurve();
			break;
		case SLIDER_SHAPE_LINE:
			this->_getLinePoints();
			break;
		case SLIDER_SHAPE_PERFECT_CIRCLE:
			this->_getCirclePoints();
			break;
		case SLIDER_SHAPE_CATMULL:
			throw NotImplementedException("Catmull slider are deprecated and not yet supported.");
		default:
			throw InvalidSliderException(std::string("Invalid slider type '") + static_cast<char>(this->_type) + "'");
		}
	}

	void Slider::update(const ReplayState &)
	{

	}

	OsuIntegerVector Slider::getScoreParticlePosition() const
	{
		return this->_points.back();
	}
}
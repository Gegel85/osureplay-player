//
// Created by Gegel85 on 18/05/2020.
//

#ifndef OSUREPLAY_PLAYER_SLIDER_HPP
#define OSUREPLAY_PLAYER_SLIDER_HPP


#include <unordered_set>
#include "HitObject.hpp"

#define POINTS_PRECISION 75

namespace OsuReplayPlayer::HitObjects
{
	enum OsuSliderShape {
		SLIDER_SHAPE_BEZIER = 'B',
		SLIDER_SHAPE_LINE = 'L',
		SLIDER_SHAPE_PERFECT_CIRCLE = 'P',
		SLIDER_SHAPE_CATMULL = 'C'
	};

	class Slider : public HitObject {
	private:
		OsuSliderShape _type;
		double _pixelLength;
		unsigned int _nbOfRepeats;
		OsuIntegerVector _end;
		sf::Vector2i _topLeft;
		sf::Image _image;
		std::vector<OsuIntegerVector> _points;
		std::vector<unsigned char> _edgeHitSounds;
		std::vector<OsuMap_sampleSet> _edgeAdditions;

		void _makeCurve();
		void _makeBezierCurve();
		OsuIntegerVector _getBezierPoint(const std::vector<OsuIntegerVector> &points, double percent);
		double _getTimeLength(OsuMap_timingPointEvent timingPt) const;
	public:
		Slider(const OsuMap_hitObject &obj, MapState &state);

		void draw(RenderTarget &target, const ReplayState &state) override;
		bool hasExpired(ReplayState &state) override;
	};
}


#endif //OSUREPLAY_PLAYER_SLIDER_HPP

#include <math.h>
#include <slider_calcs.h>
#include "frame_buffer.h"
#include "osu_map_parser.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef M_PI_2
#define M_PI_2 1.57079632679489661923
#endif

#define pt1	pos
#define pt2	points.content[0]
#define pt3	points.content[1]
#define percent	(double)i / newArray.length

typedef struct {
	double x;
	double y;
}	OsuDoubleVector;

double	calcAngle(OsuIntegerVector pt, OsuDoubleVector center)
{
	OsuDoubleVector	point1 = {pt.x, pt.y};
	OsuDoubleVector	point2 = {center.x, center.y};
	double		distance = sqrt(pow(point2.x - point1.x, 2) + pow(point2.y - point1.y, 2));
	OsuDoubleVector	vec2 = {(point2.x - point1.x) / distance, (point2.y - point1.y) / distance};

	return (atan2(vec2.y, vec2.x) * 180 / M_PI);
}

OsuIntegerVectorArray	getCirclePoints(OsuIntegerVectorArray points, OsuIntegerVector pos)
{
	OsuIntegerVectorArray	newArray = {0, NULL};
	double			radius;
	OsuDoubleVector		center;
	bool			goClockwise;
	double			angles[3];
	double			arcAngle;
	double			values[3];

	if (points.length != 2)
		display_error("Invalid number of points provided for a perfect circle slider. (3 expected but %lu found)\n", (unsigned long)points.length + 1);

	values[0] = ((pt2.x - pt1.x) * (pt3.y - pt2.y) + (pt2.y - pt1.y) * (pt2.x - pt3.x));
	if (!values[0]) {
		double dists[3] = {
			pow(pt1.x - pt2.x, 2) + pow(pt1.y - pt2.y, 2),
			pow(pt2.x - pt3.x, 2) + pow(pt2.y - pt3.y, 2),
			pow(pt1.x - pt3.x, 2) + pow(pt1.y - pt3.y, 2),
		};
		double best = fmax(fmax(dists[0], dists[1]), dists[2]);

		points.length = 1;
		if (best == dists[0])
			return getLinePoints(points, pos);
		if (best == dists[1]) {
			pos = pt3;
			return getLinePoints(points, pos);
		}
		if (best == dists[2]) {
			points.content = &pt3;
			return getLinePoints(points, pos);
		}
	}

	values[1] = (pow(pt2.x, 2) + pow(pt2.y, 2) - pow(pt3.x, 2) - pow(pt3.y, 2));
	values[2] = (pow(pt2.x, 2) + pow(pt2.y, 2) - pow(pt1.x, 2) - pow(pt1.y, 2));
	//Don't ask please
	center.y =
	(
		(pt2.x - pt3.x) * values[2] -
		(pt2.x - pt1.x) * values[1]
	) / (
		values[0] * 2
	);

	/*
	** Tbh, I don't know myself
	** That just worked so it's here
	** That's the center of the circle
	*/
	center.x =
	(
		(pt2.y - pt1.y) * values[1] -
		(pt2.y - pt3.y) * values[2]
	) / (
		values[0] * 2
	);

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
	newArray.length = (size_t)(M_PI_2 * radius * arcAngle / 360) + 1;

	//Alloc memory to hold the points
	newArray.content = malloc(newArray.length * sizeof(*newArray.content));
	if (!newArray.content)
		display_error("memory allocation error (%luB)\n", (unsigned long)(newArray.length * sizeof(*newArray.content)));

	//Create the arc
	for (size_t i = 0; i < newArray.length; i++) {
		double	angle = ((goClockwise ? angles[2] : angles[0]) + (arcAngle * (goClockwise ? 1 - percent : percent))) * M_PI / 180;

		newArray.content[i].x = lround(-cos(angle) * radius + center.x);
		newArray.content[i].y = lround(sin(angle) * radius + center.y);
	}
	return newArray;
}

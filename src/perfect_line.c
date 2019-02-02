//
// Created by Gegel85 on 02/02/2019.
//

#include <osu_map_parser.h>
#include <malloc.h>
#include <math.h>
#include "frame_buffer.h"

OsuIntegerVectorArray	getLinePoints(OsuIntegerVectorArray points, OsuIntegerVector pos)
{
	OsuIntegerVectorArray	array;
	OsuIntegerVector	diff;

	if (points.length != 1)
		display_error("Invalid linear slider: there is more than a single point");
	diff = (OsuIntegerVector){pos.x - points.content[0].x, pos.y - points.content[0].y};
	array.length = sqrt(pow(diff.x, 2) + pow(diff.y, 2));
	array.content = malloc(array.length * sizeof(*array.content));
	if (!array.content)
		display_error("Memory allocation error");
	for (unsigned i = 0; i < array.length; i++)
		array.content[i] = (OsuIntegerVector){
			diff.x * ((float)i / array.length) + points.content[0].x,
			diff.y * ((float)i / array.length) + points.content[0].y
		};
	return array;
}

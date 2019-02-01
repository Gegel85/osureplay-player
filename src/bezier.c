#include <SFML/Graphics.h>
#include <malloc.h>
#include <osu_map_parser.h>
#include "frame_buffer.h"

#define POINTS_PRECISION 100

OsuIntegerVector	bezier(OsuIntegerVectorArray points, double percent)
{
	OsuIntegerVector	result;
	OsuIntegerVectorArray	pts = {points.length - 1, NULL};

	if (points.length <= 0)
		return (OsuIntegerVector){0, 0};
	if (points.length == 1)
		return *points.content;
	pts.content = malloc(pts.length * sizeof(*pts.content));
	if (!pts.content)
		display_error("Memory allocation error\n");
	for (unsigned i = 0; i < pts.length; i++)
		pts.content[i] = (OsuIntegerVector){
			((points.content[i].x - points.content[i + 1].x) * percent / 100) + points.content[i].x,
			((points.content[i].y - points.content[i + 1].y) * percent / 100) + points.content[i].y
		};
	result = bezier(pts, percent);
	free(pts.content);
	return result;
}

OsuIntegerVectorArray	getBezierPoints(OsuIntegerVectorArray points, OsuIntegerVector pos)
{
	OsuIntegerVectorArray	*pts = malloc(sizeof(*pts));
	unsigned		arrLen = 1;
	OsuIntegerVectorArray	result = {0, NULL};
	unsigned		counter = 0;

	if (!pts)
		display_error("Memory allocation error\n");
	pts->length = 1;
	pts->content = malloc(sizeof(*pts->content));
	if (!pts->content)
		display_error("Memory allocation error\n");
	*pts->content = pos;
	for (unsigned i = 0; i < points.length; i++) {
		if (
			pts[arrLen - 1].length >= 1 &&
			points.content[i].x == pts[arrLen - 1].content[pts[arrLen - 1].length - 1].x &&
			points.content[i].y == pts[arrLen - 1].content[pts[arrLen - 1].length - 1].y
		) {
			arrLen++;
			pts = realloc(pts, arrLen * sizeof(*pts));
			if (!pts)
				display_error("Memory allocation error\n");
			pts[arrLen - 1].length = 0;
			pts[arrLen - 1].content = NULL;
		}
		pts[arrLen - 1].content = realloc(pts[arrLen - 1].content, ++pts[arrLen - 1].length * sizeof(*pts[arrLen - 1].content));
		if (!pts->content)
			display_error("Memory allocation error\n");
		pts[arrLen - 1].content[pts[arrLen - 1].length - 1] = points.content[i];
	}
	for (unsigned i = 0; i < arrLen; i++)
		result.length += pts[i].length > 1 ? POINTS_PRECISION : 1;
	result.content = malloc(result.length * sizeof(*result.content));
	for (unsigned i = 0; i < arrLen; i++)
		for (unsigned j = 0; j < (pts[i].length > 1 ? POINTS_PRECISION : 1); j += 1) {
			result.content[counter] = bezier(pts[i], j / 10.);
			counter++;
		}
	return result;
}
//
// Created by Gegel85 on 02/02/2019.
//

#include <stdlib.h>
#include <osu_map_parser.h>
#include <string.h>
#include "slider_calcs.h"
#include "frame_buffer.h"

void	getRealPointsSliders(OsuMap_hitObject *obj)
{
	OsuMap_hitObjectSliderInfos	*infos = obj->additionalInfos;
	OsuIntegerVectorArray		points = infos->curvePoints;

	for (unsigned i = 0; i < points.length - 1; i++) {
		if (points.content[i].x == points.content[i + 1].x && points.content[i].y == points.content[i + 1].y) {
			memmove(&points.content[i], &points.content[i + 1], (points.length - i - 1) * sizeof(*points.content));
			i--;
			points.length--;
		}
	}
	switch (infos->type) {
		case 'B':
			infos->curvePoints = getBezierPoints(points, obj->position);
			break;
		case 'L':
			infos->curvePoints = getLinePoints(points, obj->position);
			break;
		case 'P':
			infos->curvePoints = getCirclePoints(points, obj->position);
			break;
		case 'C':
			display_warning("Warning: Catmull sliders are deprecated and are, therefore, not supported\n");
			return;
		default:
			display_error("Invalid slider type '%c'\n", infos->type);

	}
	free(points.content);
}

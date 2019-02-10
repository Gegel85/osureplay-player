//
// Created by Gegel85 on 02/02/2019.
//

#include <stdlib.h>
#include <osu_map_parser.h>
#include "slider_calcs.h"
#include "frame_buffer.h"

void	getRealPointsSliders(OsuMap_hitObject *obj)
{
	OsuMap_hitObjectSliderInfos	*infos = obj->additionalInfos;
	OsuIntegerVectorArray		points = infos->curvePoints;

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
			display_warning("Warning: Catmull sliders are deprecated and are, therefore, not supported");
			return;
		default:
			display_error("Invalid slider type");

	}
	free(points.content);
}

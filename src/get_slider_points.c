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
			free(points.content);
			break;
		case 'L':
			infos->curvePoints = getLinePoints(points, obj->position);
			free(points.content);
			break;
		case 'C':
			display_error("Catmull slider are deprecated and are, therefore, not supported");
			break;
		case 'P':
			break;
		default:
			display_error("Invalid slider type");

	}
}

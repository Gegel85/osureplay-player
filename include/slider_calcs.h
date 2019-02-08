#ifndef OSUREPLAY_PLAYER_SLIDER_CALCS_H
#define OSUREPLAY_PLAYER_SLIDER_CALCS_H


#include <osu_map_parser.h>

void			getRealPointsSliders(OsuMap_hitObject *obj);
OsuIntegerVectorArray	getBezierPoints(OsuIntegerVectorArray points, OsuIntegerVector pos);
OsuIntegerVectorArray	getCirclePoints(OsuIntegerVectorArray points, OsuIntegerVector pos);
OsuIntegerVectorArray	getLinePoints(OsuIntegerVectorArray points, OsuIntegerVector pos);


#endif //OSUREPLAY_PLAYER_SLIDER_CALCS_H

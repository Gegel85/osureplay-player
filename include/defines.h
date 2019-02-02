#ifndef OSUREPLAY_PLAYER_DEFINES_H
#define OSUREPLAY_PLAYER_DEFINES_H

#define toSfVector2f(x, y)	((sfVector2f){x, y})

#define sliderInfos(elem)	((OsuMap_hitObjectSliderInfos *)elem)

#define sliderLength(beatmap, elem, timeingpt)	sliderInfos(beatmap->hitObjects.content[elem].additionalInfos)->pixelLength / (100 * beatmap->difficulty.sliderMultiplayer) * timeingpt.millisecondsPerBeat

#define BASE_OBJ_ALPHA		255
#define nbOfSound		8
#define bgAlpha			120

#endif //OSUREPLAY_PLAYER_DEFINES_H

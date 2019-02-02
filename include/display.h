//
// Created by Gegel85 on 02/02/2019.
//

#ifndef OSUREPLAY_PLAYER_DISPLAY_H
#define OSUREPLAY_PLAYER_DISPLAY_H


#include <osu_map_parser.h>
#include <SFML/Audio.h>
#include "dict.h"

void	playSound(sfSoundBuffer *buffer);
void	displayHitObjects(unsigned currentComboColor, unsigned currentGameHitObject, unsigned currentTimingPoint, OsuMap *beatmap, unsigned long totalTicks, unsigned beginCombo, Dict *images, Dict *sounds);


#endif //OSUREPLAY_PLAYER_DISPLAY_H

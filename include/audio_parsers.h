//
// Created by Gegel85 on 24/03/2019.
//

#ifndef OSUREPLAY_PLAYER_AUDIO_PARSERS_H
#define OSUREPLAY_PLAYER_AUDIO_PARSERS_H


#include <stdio.h>
#include <stdbool.h>
#include "sound.h"

#define PARSERS_COUNT 1

extern bool (* const audioChecker[])(FILE *stream);
extern Sound *(* const audioParsers[])(FILE *stream);


#endif //OSUREPLAY_PLAYER_AUDIO_PARSERS_H

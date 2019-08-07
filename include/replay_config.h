//
// Created by Gegel85 on 06/08/2019.
//

#ifndef OSUREPLAY_PLAYER_REPLAY_CONFIG_H
#define OSUREPLAY_PLAYER_REPLAY_CONFIG_H

#include <stddef.h>
#include <SFML/Audio.h>
#include <osu_replay_parser.h>
#include <osu_map_parser.h>
#include "dict.h"

typedef struct ReplayConfig {
	char		*filePath;
	unsigned long	bitRate;
	unsigned long	frameRate;
	sfVector2u	resolution;
	OsuReplay	*replay;
	OsuMap		*beatmap;
	Dict		*sounds;
	Dict		*images;
	bool		cleanUp;
	unsigned char	bgAlpha;
} ReplayConfig;

#endif //OSUREPLAY_PLAYER_REPLAY_CONFIG_H

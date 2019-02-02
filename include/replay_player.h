//
// Created by Gegel85 on 02/02/2019.
//

#ifndef OSUREPLAY_PLAYER_REPLAY_PLAYER_H
#define OSUREPLAY_PLAYER_REPLAY_PLAYER_H


#include <stdio.h>

#include <osu_map_parser.h>
#include <osu_replay_parser.h>
#include <libavcodec/avcodec.h>
#include <SFML/Graphics.h>

#include "dict.h"

typedef struct replayPlayerState {
	unsigned long	totalTicks;
	unsigned long	currentTimingPoint;
	unsigned int	currentGameEvent;
	unsigned int	currentLifeEvent;
	unsigned int	currentGameHitObject;
	unsigned int	beginCombo;
	unsigned int	currentComboColor;
	sfVector2f	cursorPos;
	unsigned int	pressed;
	float		life;
	int		*played;
	bool		musicStarted;
	FILE		*stream;
	AVCodecContext	*codecContext;
	AVFrame		*frame;
	AVPacket	*packet;
	int		frameNb;
} replayPlayerState;

void	encode_frame(AVCodecContext *enc_ctx, AVFrame *frame, AVPacket *pkt, FILE *outfile);
void	playReplay(OsuReplay *replay, OsuMap *beatmap, sfVector2u size, Dict *sounds, Dict *images, char *path);


#endif //OSUREPLAY_PLAYER_REPLAY_PLAYER_H

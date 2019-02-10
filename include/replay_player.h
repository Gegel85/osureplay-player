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
#include "frame_buffer.h"
#include "sound.h"

typedef struct replayPlayerState {
	double		totalTicks;
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
	AVCodecContext	*videoCodecContext;
	AVCodecContext	*audioCodecContext;
	AVFrame		*videoFrame;
	AVFrame		*audioFrame;
	AVPacket	*videoPacket;
	AVPacket	*audioPacket;
	int		frameNb;
	unsigned long	totalFrames;
	Dict		*sounds;
	Dict		*images;
	FrameBuffer	frame_buffer;
	PlayingSound	*playingSounds;
} replayPlayerState;

void	encodeVideoFrame(AVCodecContext *enc_ctx, AVFrame *frame,
			     AVPacket *pkt, FILE *outfile);
void	playReplay(OsuReplay *replay, OsuMap *beatmap, sfVector2u size, Dict *sounds, Dict *images, char *path);


#endif //OSUREPLAY_PLAYER_REPLAY_PLAYER_H

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
#include <libavformat/avformat.h>

#include "dict.h"
#include "frame_buffer.h"
#include "sound.h"

typedef struct replayPlayerState {
	/*Game state*/
	double		totalTicks;		//Current time
	unsigned long	currentTimingPoint;	//Currently active timing point
	unsigned int	currentGameEvent;	//Current game event id
	unsigned int	currentLifeEvent;	//Current life event id
	unsigned int	currentGameHitObject;	//Current hit object id
	unsigned int	beginCombo;		//Current combo state
	unsigned int	currentComboColor;	//Current color for objects
	sfVector2f	cursorPos;		//Current cursor position
	unsigned int	pressed;		//Currently pressed buttons
	float		life;			//Current player life

	/*Player state*/
	int		*played;		//Number of time the hit sound has been played for each game object
	bool		musicStarted;		//Whether the music has started or not
	AVStream	*videoAvStream;		//Video stream
	AVStream	*audioAvStream;		//Audio stream
	AVFrame		*videoFrame;		//Audio stream
	AVFormatContext	*formatContext;		//Format context
	Dict		*sounds;		//All the loaded sounds
	Dict		*images;		//All the loaded sprites
	FrameBuffer	frameBuffer;		//The framebuffer
	PlayingSound	*playingSounds;		//All the currently playing sounds

	/*Stats*/
	unsigned int	pressedKey1;		//The number of times the key1 was pressed
	unsigned int	pressedKey2;		//The number of times the key2 was pressed
	int		frameNb;		//Current frame number
	unsigned long	totalFrames;		//Total number of frames
} replayPlayerState;

void	encodeAudioFrame(AVCodecContext *enc_ctx, AVFrame *frame, AVPacket *pkt, FILE *output);
void	encodeVideoFrame(AVFormatContext *fmtContext, AVStream *stream, AVFrame *frame);
void	playReplay(OsuReplay *replay, OsuMap *beatmap, sfVector2u size, Dict *sounds, Dict *images, char *path);


#endif //OSUREPLAY_PLAYER_REPLAY_PLAYER_H

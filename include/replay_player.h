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
#include "replay_config.h"

typedef struct ReplayPlayerState {
	double		totalTicks;		//Current time
	unsigned long	currentTimingPoint;	//Currently active timing point
	unsigned int	currentGameEvent;	//Current game event id
	unsigned int	currentLifeEvent;	//Current life event id
	unsigned int	currentGameHitObject;	//Current hit object id
	unsigned int	beginCombo;		//Current combo state
	unsigned int	currentComboColor;	//Current color for objects
	sfVector2f	cursorPos;		//Current cursor position
	unsigned int	pressed;		//Currently pressed buttons
	unsigned int	oldPressed;		//Buttons pressed last frame
	float		life;			//Current player life
	int		*played;		//Number of time the hit sound has been played for each game object
	bool		musicStarted;		//Whether the music has started or not
	FILE		*videoStream;		//Video file stream
	FILE		*audioStream;		//Audio file stream
	AVCodecContext	*videoCodecContext;	//Video codec context
	AVCodecContext	*audioCodecContext;	//Audio codec context
	AVFrame		*videoFrame;		//Frame used for video
	AVFrame		*audioFrame;		//Frame used for audio
	AVPacket	*videoPacket;		//Packet used for video
	AVPacket	*audioPacket;		//Packet used for audio
	unsigned int	pressedKey1;		//The number of times the key1 was pressed
	unsigned int	pressedKey2;		//The number of times the key2 was pressed
	unsigned int	frameNb;		//Current frame number
	unsigned long	totalFrames;		//Total number of frames
	Dict		*sounds;		//All the loaded sounds
	Dict		*images;		//All the loaded sprites
	FrameBuffer	frameBuffer;		//The framebuffer
	PlayingSound	*playingSounds;		//All the currently playing sounds
	const char	*backgroundPictureIndex;//The index of the background picture
	const char	*musicIndex;		//The index of the music
	OsuMap		*beatmap;		//The beatmap being played
	OsuReplay	*replay;		//The replay being played;
	bool		isEnd;			//Whether the replay session has came to an end
	unsigned char	bgAlpha;		//Current background alpha
	unsigned char	minNbAlpha;		//The minimun alpha the background can get (user set setting)
} ReplayPlayerState;

void	encodeAudioFrame(AVCodecContext *enc_ctx, AVFrame *frame, AVPacket *pkt, FILE *output);
void	encodeVideoFrame(AVCodecContext *enc_ctx, AVFrame *frame, AVPacket *pkt, FILE *outfile);
void	playReplay(const ReplayConfig *configs);


#endif //OSUREPLAY_PLAYER_REPLAY_PLAYER_H

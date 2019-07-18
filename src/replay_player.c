#include <stdlib.h>
#include <string.h>
#include <libavutil/channel_layout.h>
#include <osu_replay_parser.h>
#include <osu_map_parser.h>
#include <SFML/Graphics.h>
#include <concatf.h>
#include "sound.h"
#include "display.h"
#include "skin.h"
#include "dict.h"
#include "globals.h"
#include "replay_player.h"

AVCodecContext *initVideoCodec(sfVector2u size)
{
	const AVCodec	*codec;
	AVCodecContext	*codecContext;

	/* find the video encoder */
	codec = avcodec_find_encoder(AV_CODEC_ID_MPEG1VIDEO);
	if (!codec)
		display_error("MPEG1VIDEO codec not found\n");
	codecContext = avcodec_alloc_context3(codec);

	/* put sample parameters */
	codecContext->bit_rate = 4000000;

	/* resolution must be a multiple of two */
	codecContext->width = size.x;
	codecContext->height = size.y;

	/* frames per second */
	codecContext->time_base = (AVRational){1, 60};
	codecContext->framerate = (AVRational){60, 1};

	codecContext->gop_size = 10; /* emit one intra frame every ten frames */
	codecContext->max_b_frames = 1;
	codecContext->pix_fmt = AV_PIX_FMT_YUV420P;

	/* open it */
	if (avcodec_open2(codecContext, codec, NULL) < 0)
		display_error("Couldn't open video codec\n");

	return codecContext;
}

AVCodecContext *initAudioCodec()
{
	const AVCodec	*codec;
	AVCodecContext	*codecContext;

	/* register all the codecs */
	avcodec_register_all();

	/* find the MP2 encoder */
	codec = avcodec_find_encoder(AV_CODEC_ID_MP3);
	if (!codec)
		display_error("codec not found\n");
	codecContext = avcodec_alloc_context3(codec);

	/* put sample parameters */
	codecContext->bit_rate = 64000;
	codecContext->sample_fmt = AV_SAMPLE_FMT_S16;

	/* select other audio parameters supported by the encoder */
	codecContext->sample_rate = SAMPLE_RATE;
	codecContext->channel_layout = AV_CH_LAYOUT_MONO;
	codecContext->channels = 1;

	/* open it */
	if (avcodec_open2(codecContext, codec, NULL) < 0)
		display_error("could not open codec\n");

	printf("%i\n", codecContext->frame_size);
	return codecContext;
}

AVFrame	*initVideoFrame(AVCodecContext *context)
{
	AVFrame	*frame;

	frame = av_frame_alloc();
	if (!frame)
		display_error("could not allocate video frame\n");
	frame->format = context->pix_fmt;
	frame->width = context->width;
	frame->height = context->height;
	frame->quality = 1;

	if (av_frame_get_buffer(frame, 32) < 0)
		display_error("could not allocate video data buffers\n");
	return frame;
}

AVFrame	*initAudioFrames(AVCodecContext *context)
{
	AVFrame	*frame;

	/* frame containing input raw audio */
	frame = av_frame_alloc();
	if (!frame)
		display_error("could not allocate audio frame\n");
	frame->nb_samples = context->frame_size;
	frame->format = context->sample_fmt;
	frame->channel_layout = context->channel_layout;

	/* allocate the data buffers */
	if (av_frame_get_buffer(frame, 0) < 0)
		display_error("could not allocate audio data buffers: %s\n", strerror(AVUNERROR(av_frame_get_buffer(frame, 0))));
	return frame;
}

void	startResplaySession(replayPlayerState *state, const char *path, OsuMap *beatmap, sfVector2u size)
{
	/* Put base values */
	memset(state, 0, sizeof(*state));
	state->life = 1;
	state->beginCombo = 1;
	state->played = calloc(beatmap->hitObjects.length, sizeof(*state->played));
	if (!state->played)
		display_error("Memory allocation error (%luB)\n", (unsigned long)sizeof(*state->played) * (unsigned long)beatmap->hitObjects.length);

	/* init framebuffer */
	FrameBuffer_init(&state->frame_buffer, size);

	/* Debug mode -> don't create audio/video contexts */
	if (!path)
		return;

	char	audioPath[strlen(path) + 5];
	char	videoPath[strlen(path) + 5];

	/* Get paths */
	sprintf(audioPath, "%s.mp3", path);
	sprintf(videoPath, "%s.mp4", path);

	/* register all codecs */
	avcodec_register_all();

	/* init codecs */
	state->videoCodecContext = initVideoCodec(size);
	state->audioCodecContext = initAudioCodec();

	/* init video frame */
	state->videoFrame = initVideoFrame(state->videoCodecContext);
	state->videoPacket = av_packet_alloc();
	if (!state->videoPacket)
		display_error("Memory allocation error\n");

	/* init audio frames */
	state->audioFrame = initAudioFrames(state->audioCodecContext);
	state->audioPacket = av_packet_alloc();
	if (!state->audioPacket)
		display_error("Memory allocation error\n");

	state->frameNb = 1;

	/* open files */
	state->videoStream = fopen(videoPath, "wb");
	if (!state->videoStream)
		display_error("Cannot open %s: %s\n", path, strerror(errno));
	state->audioStream = fopen(audioPath, "wb");
	if (!state->audioStream)
		display_error("Cannot open %s: %s\n", path, strerror(errno));

	state->playingSounds = malloc(sizeof(*state->playingSounds));
	if (!state->playingSounds)
		display_error("Memory allocation error (%lu)\n", sizeof(*state->playingSounds));
	memset(state->playingSounds, 0, sizeof(*state->playingSounds));
}

void	finishReplaySession(replayPlayerState *state)
{
	uint8_t endcode[] = {0, 0, 1, 0xb7};

	if (!state->videoStream)
		return;

	/* destroy framebuffer */
	FrameBuffer_destroy(&state->frame_buffer);

	/* flush the encoders */
	encodeVideoFrame(state->videoCodecContext, NULL, state->videoPacket, state->videoStream);
	encodeAudioFrame(state->audioCodecContext, NULL, state->audioPacket, state->videoStream);

	/* add sequence end code to have a real MPEG file */
	fwrite(endcode, 1, sizeof(endcode), state->videoStream);
	fclose(state->videoStream);
	fclose(state->audioStream);

	/* free frames */
	av_frame_free(&state->videoFrame);
	av_frame_free(&state->audioFrame);

	/* free packets */
	av_packet_free(&state->videoPacket);
	av_packet_free(&state->audioPacket);

	/* free codec contexts */
	avcodec_free_context(&state->videoCodecContext);
	avcodec_free_context(&state->audioCodecContext);
}

void	playReplay(OsuReplay *replay, OsuMap *beatmap, sfVector2u size, Dict *sounds, Dict *images, char *path)
{
	sfVideoMode		mode = {size.x, size.y, 32};
	sfEvent			event;
	sfClock			*clock = NULL;
	replayPlayerState	state;

	startResplaySession(&state, path, beatmap, size);

	if (!path) {
		window = sfRenderWindow_create(mode, "Osu Replay Player", sfDefaultStyle, NULL);
		if (!window)
			exit(EXIT_FAILURE);
		sfRenderWindow_setFramerateLimit(window, 60);
	}

	state.sounds = sounds;
	state.images = images;
	state.totalFrames = replay->replayLength * 60 / ((replay->mods & MODE_DOUBLE_TIME) || (replay->mods & MODE_NIGHTCORE) ? 1500 : 1000) + 1;
	printf("Replay length: %lums, %lu video frame(s), %lu audio frames\n", replay->replayLength, state.totalFrames, state.totalFrames * 44100);
	padding = (sfVector2u){64, 48};

	beatmap->backgroundPath = beatmap->backgroundPath ? strToLower(getFileName(beatmap->backgroundPath)) : NULL;
	beatmap->generalInfos.audioFileName = beatmap->generalInfos.audioFileName ? strToLower(getFileName(beatmap->generalInfos.audioFileName)) : NULL;
	while (
		state.currentGameEvent < replay->gameEvents.length ||
		replay->gameEvents.content[state.currentGameEvent].timeToHappen > (long)state.totalTicks
	) {
		if (!path) {
			while (sfRenderWindow_pollEvent(window, &event)) {
				if (event.type == sfEvtClosed) {
					sfRenderWindow_close(window);
					return;
				}
			}
			sfRenderWindow_clear(window, (sfColor){0, 0, 0, 255});
			if (clock) {
				if ((replay->mods & MODE_DOUBLE_TIME) || (replay->mods & MODE_NIGHTCORE))
					state.totalTicks = sfTime_asMilliseconds(sfClock_getElapsedTime(clock)) * 1.5;
				else
					state.totalTicks = sfTime_asMilliseconds(sfClock_getElapsedTime(clock));
			}
		} else
			state.totalTicks += ((replay->mods & MODE_DOUBLE_TIME) || (replay->mods & MODE_NIGHTCORE) ? 1500 : 1000) / 60.;

		FrameBuffer_clear(&state.frame_buffer, (sfColor){0, 0, 0, 255});
		FrameBuffer_drawImage(
			&state.frame_buffer,
			(sfVector2i){0, 0},
			Dict_getElement(images, beatmap->backgroundPath),
			(sfVector2i){size.x, size.y},
			(sfColor){255, 255, 255, bgAlpha},
			false,
			0
		);

		if ((unsigned)beatmap->generalInfos.audioLeadIn <= state.totalTicks && !state.musicStarted) {
			if (!state.musicStarted) {
				if (!path && music) {
					sfMusic_play(music);
					if ((replay->mods & MODE_DOUBLE_TIME) || (replay->mods & MODE_NIGHTCORE))
						sfMusic_setPitch(music, 1.5);
				} else
					playSound(&state, beatmap->generalInfos.audioFileName, (replay->mods & MODE_NIGHTCORE) ? 1.5 : 1, (replay->mods & MODE_DOUBLE_TIME) || (replay->mods & MODE_NIGHTCORE) ? 1.5 : 1);
				state.musicStarted = true;
			}
		}

		while (
			state.currentLifeEvent < replay->lifeBar.length &&
			replay->lifeBar.content[state.currentLifeEvent].timeToHappen <= state.totalTicks
		) {
			state.life = replay->lifeBar.content[state.currentLifeEvent].newValue;
			state.currentLifeEvent++;
		}

		while (
			state.currentGameEvent < replay->gameEvents.length &&
			replay->gameEvents.content[state.currentGameEvent].timeToHappen <= (int)state.totalTicks
		) {
			state.cursorPos = *(sfVector2f *)&replay->gameEvents.content[state.currentGameEvent].cursorPos;
			state.cursorPos.x += padding.x;
			state.cursorPos.y += padding.y;
			state.pressed = replay->gameEvents.content[state.currentGameEvent].keysPressed;
			state.currentGameEvent++;
		}

		if (
			!state.played[state.currentGameHitObject] &&
			beatmap->hitObjects.content[state.currentGameHitObject].timeToAppear <= state.totalTicks &&
			beatmap->hitObjects.content[state.currentGameHitObject].type & HITOBJ_SLIDER
		) {
			playSound(&state, "drum-hitnormal", 1, 1);
			state.played[state.currentGameHitObject]++;
		}
		while (
			state.currentGameHitObject < beatmap->hitObjects.length && (
				(
					!(beatmap->hitObjects.content[state.currentGameHitObject].type & HITOBJ_SLIDER) &&
					!(beatmap->hitObjects.content[state.currentGameHitObject].type & HITOBJ_SPINNER) &&
					beatmap->hitObjects.content[state.currentGameHitObject].timeToAppear + 20 <= state.totalTicks
				) || (
					beatmap->hitObjects.content[state.currentGameHitObject].type & HITOBJ_SLIDER &&
					beatmap->hitObjects.content[state.currentGameHitObject].timeToAppear +
					sliderLength(beatmap, state.currentGameHitObject, beatmap->timingPoints.content[state.currentTimingPoint]) <= state.totalTicks
				) || (
					beatmap->hitObjects.content[state.currentGameHitObject].type & HITOBJ_SPINNER &&
					*(unsigned long *)beatmap->hitObjects.content[state.currentGameHitObject].additionalInfos <= state.totalTicks
				)
			)
		) {
			playSound(&state, "drum-hitnormal", 1, 1);
			state.played[state.currentGameHitObject] = true;
			if (beatmap->hitObjects.content[state.currentGameHitObject].type & HITOBJ_NEW_COMBO) {
				state.beginCombo = 0;
				state.currentComboColor = (state.currentComboColor + 1) % beatmap->colors.length;
			}
			state.beginCombo++;
			state.currentGameHitObject++;
		}

		displayHitObjects(&state, beatmap);

		FrameBuffer_drawFilledRectangle(&state.frame_buffer, (sfVector2i){10, 10}, (sfVector2u){300 * state.life, 20}, (sfColor){255, 255, 255, 255});
		FrameBuffer_drawImage(&state.frame_buffer, (sfVector2i){state.cursorPos.x, state.cursorPos.y}, Dict_getElement(images, "cursor"), (sfVector2i){-1, -1}, (sfColor){255, 255, 255, 255}, true, 0);

		if (!path) {
			FrameBuffer_draw(&state.frame_buffer, window);
			sfRenderWindow_display(window);
			if (!clock)
				clock = sfClock_create();
		} else {
			FrameBuffer_encode(&state.frame_buffer, &state);
			encodePlayingSounds(&state);
			state.frameNb++;
		}
	}
	finishReplaySession(&state);
}

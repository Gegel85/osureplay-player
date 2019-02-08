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

	/* find the audio encoder */
	codec = avcodec_find_encoder(AV_CODEC_ID_MP2);
	if (!codec)
		display_error("MP2 codec not found\n");
	codecContext = avcodec_alloc_context3(codec);
	codecContext->bit_rate = 64000;
	codecContext->sample_rate = 44100;
	codecContext->channels = 2;
	codecContext->channel_layout = AV_CH_LAYOUT_STEREO;
	codecContext->sample_fmt = AV_SAMPLE_FMT_S16;

	/* open it */
	if (avcodec_open2(codecContext, codec, NULL) < 0)
		display_error("Couldn't open audio codec\n");

	return codecContext;
}

AVFrame	*initVideoFrame(AVCodecContext *context)
{
	AVFrame	*frame;

	frame = av_frame_alloc();
	frame->format = context->pix_fmt;
	frame->width = context->width;
	frame->height = context->height;
	frame->quality = 1;

	if (av_frame_get_buffer(frame, 32) < 0)
		display_error("Memory allocation error\n");
	return frame;
}

AVFrame	*initAudioFrames(AVCodecContext *context, uint64_t layout)
{
	AVFrame	*frame;

	frame = av_frame_alloc();
	frame->format = context->sample_fmt;
	frame->nb_samples = context->sample_rate / 60 + 1;
	frame->sample_rate = context->sample_rate;
	frame->channel_layout = layout;
	frame->quality = 1;

	if (av_frame_get_buffer(frame, 32) < 0)
		display_error("Memory allocation error\n");
	return frame;
}

void	startResplaySession(replayPlayerState *state, const char *path, OsuMap *beatmap, sfVector2u size)
{
	/* Put base values */
	memset(state, 0, sizeof(*state));
	state->life = 1;
	state->beginCombo = 1;
	state->played = malloc(sizeof(*state->played) * beatmap->hitObjects.length);
	if (!state->played)
		display_error("Memory allocation error (%luB)\n", sizeof(*state->played) * beatmap->hitObjects.length);
	memset(state->played, 0, sizeof(*state->played) * beatmap->hitObjects.length);
	state->stream = NULL;

	/* Debug mode -> don't create audio/video contexts */
	if (!path)
		return;

	/* init framebuffer */
	FrameBuffer_init(&state->frame_buffer, size);

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
	state->audioFrames[0] = initAudioFrames(state->audioCodecContext, AV_CH_STEREO_LEFT);
	state->audioFrames[1] = initAudioFrames(state->audioCodecContext, AV_CH_STEREO_RIGHT);
	state->audioPacket = av_packet_alloc();
	if (!state->audioPacket)
		display_error("Memory allocation error\n");

	/* open file */
	state->stream = fopen(path, "wb");
	if (!state->stream)
		display_error("Connot open %s: %s\n", path, strerror(errno));
}

void	finishReplaySession(replayPlayerState *state)
{
	uint8_t endcode[] = {0, 0, 1, 0xb7};

	if (!state->stream)
		return;

	/* destroy framebuffer */
	FrameBuffer_destroy(&state->frame_buffer);

	/* flush the encoder */
	encodeFrame(state->videoCodecContext, NULL, state->videoPacket, state->stream);

	/* add sequence end code to have a real MPEG file */
	fwrite(endcode, 1, sizeof(endcode), state->stream);
	fclose(state->stream);

	/* free frames */
	av_frame_free(&state->videoFrame);
	av_frame_free(&state->audioFrames[0]);
	av_frame_free(&state->audioFrames[1]);

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

		if ((unsigned)beatmap->generalInfos.audioLeadIn <= state.totalTicks && music && sfMusic_getStatus(music) != sfPlaying) {
			if (!state.musicStarted) {
				if (!path) {
					sfMusic_play(music);
					if ((replay->mods & MODE_DOUBLE_TIME) || (replay->mods & MODE_NIGHTCORE))
						sfMusic_setPitch(music, 1.5);
				}
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
			if (!path)
				playSound(&state, "drum-hitnormal", 1);
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
					(unsigned long)*(long *)beatmap->hitObjects.content[state.currentGameHitObject].additionalInfos <= state.totalTicks
				)
			)
		) {
			if (!path)
				playSound(&state, "drum-hitnormal", 1);
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
		} else
			FrameBuffer_encode(&state.frame_buffer, &state);
	}
	finishReplaySession(&state);
}

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

AVStream *createVideoStream(AVFormatContext *fmtContext, sfVector2u size)
{
	AVCodec		*codec;
	AVCPBProperties	*properties;
	AVStream	*stream = avformat_new_stream(fmtContext, NULL);
	AVCodecContext	*codecContext;

	if (!stream)
		display_error("Couldn't alloc video stream\n");

	codecContext = stream->codec;
	codecContext->codec_id = fmtContext->oformat->video_codec;
	codecContext->codec_type = AVMEDIA_TYPE_VIDEO;

	/* Put sample parameters */
	codecContext->bit_rate = 400000;

	/* Resolution must be a multiple of two */
	codecContext->width = size.x;
	codecContext->height = size.y;

	/* Frames per second */
	codecContext->time_base = (AVRational){1, 60};
	codecContext->framerate = (AVRational){60, 1};

	codecContext->pix_fmt = AV_PIX_FMT_YUV420P;

	/* Emit one intra frame every 12 frames at most */
	codecContext->gop_size = 12;
	if (codecContext->codec_id == AV_CODEC_ID_MPEG1VIDEO)
		codecContext->mb_decision = 2;

	if (fmtContext->oformat->flags & AVFMT_GLOBALHEADER)
		codecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

	/* Find the codec */
	codec = avcodec_find_encoder(codecContext->codec_id);
	if (!codec)
		display_error("Cannot find codec\n");

	/* Open it */
	if (avcodec_open2(codecContext, codec, NULL) < 0)
		display_error("Cannot open codec\n");

	properties = (AVCPBProperties *)av_stream_new_side_data(stream, AV_PKT_DATA_CPB_PROPERTIES, sizeof(*properties));

	properties->avg_bitrate = 900;
	properties->max_bitrate = 1000;
	properties->min_bitrate = 0;
	properties->buffer_size = 2 * 1024 * 1024;
	properties->vbv_delay = UINT64_MAX;
	return stream;
}

AVStream *createAudioStream(AVFormatContext *fmtContext)
{
	AVCodec		*codec;
	AVStream	*stream = avformat_new_stream(fmtContext, NULL);
	AVCodecContext	*codecContext;

	if (!stream)
		display_error("Couldn't alloc audio stream\n");

	codecContext = stream->codec;
	codecContext->codec_id = fmtContext->oformat->audio_codec;
	codecContext->codec_type = AVMEDIA_TYPE_AUDIO;

	/* Put sample parameters */
	codecContext->bit_rate = 64000;
	codecContext->sample_fmt = AV_SAMPLE_FMT_S16;
	codecContext->channels = 1;
	codecContext->channel_layout = AV_CH_LAYOUT_MONO;
	codecContext->sample_rate = SAMPLE_RATE;

	if (fmtContext->oformat->flags & AVFMT_GLOBALHEADER)
		codecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

	/* Find the codec */
	codec = avcodec_find_encoder(codecContext->codec_id);
	if (!codec)
		display_error("Cannot find codec\n");

	/* Open it */
	if (avcodec_open2(codecContext, codec, NULL) < 0)
		display_error("Cannot open codec\n");

	return stream;
}

AVFrame	*createVideoFrame(AVCodecContext *context)
{
	AVFrame	*frame;

	frame = av_frame_alloc();
	if (!frame)
		display_error("Cannot allocate video frame\n");
	frame->format = context->pix_fmt;
	frame->width = context->width;
	frame->height = context->height;
	frame->quality = 1;

	if (av_frame_get_buffer(frame, 32) < 0)
		display_error("Cannot allocate video data buffers\n");
	return frame;
}

void	startReplaySession(replayPlayerState *state, const char *path, OsuMap *beatmap, sfVector2u size)
{
	/* Put base values */
	memset(state, 0, sizeof(*state));
	state->life = 1;
	state->beginCombo = 1;
	state->played = calloc(beatmap->hitObjects.length, sizeof(*state->played));
	if (!state->played)
		display_error("Memory allocation error (%luB)\n", (unsigned long)sizeof(*state->played) * (unsigned long)beatmap->hitObjects.length);

	/* Init framebuffer */
	FrameBuffer_init(&state->frameBuffer, size);

	/* Debug mode -> don't create audio/video contexts */
	if (!path)
		return;

	/* Register all codecs */
	av_register_all();

	//Create format context
	avformat_alloc_output_context2(&state->formatContext, NULL, NULL, path);

	if (!state->formatContext)
		display_error("Cannot open format corresponding to this file or cannot deduce format from file name\n");

	/* Init codecs */
	state->videoAvStream = createVideoStream(state->formatContext, size);
	//state->audioAvStream = createAudioStream(state->formatContext);

	/* Init video frame */
	state->videoFrame = createVideoFrame(state->videoAvStream->codec);

	state->frameNb = 1;

	state->playingSounds = calloc(1, sizeof(*state->playingSounds));
	if (!state->playingSounds)
		display_error("Memory allocation error (%lu)\n", (unsigned long)sizeof(*state->playingSounds));

	if (!(state->formatContext->oformat->flags & AVFMT_NOFILE) && avio_open(&state->formatContext->pb, path, AVIO_FLAG_WRITE) < 0)
		display_error("Cannot open file '%s'\n", path);
	/* Write file header */
	if (avformat_write_header(state->formatContext, NULL) < 0)
		display_error("Cannot write header");

	av_dump_format(state->formatContext, 0, path, 1);
}

void	finishReplaySession(replayPlayerState *state)
{
	if (!state->formatContext)
		return;

	/* Write file trailer */
	av_write_trailer(state->formatContext);

	/* Destroy framebuffer */
	FrameBuffer_destroy(&state->frameBuffer);

	/* Flush the encoders */
	encodeVideoFrame(state->formatContext, state->videoAvStream, NULL);
	//encodeAudioFrame(state->audioCodecContext, NULL, state->audioPacket, state->videoStream);

	/* free frames */
	av_frame_free(&state->videoFrame);

	/* free packets*/
	/*av_packet_free(&state->videoPacket);
	av_packet_free(&state->audioPacket);*/

	/* free codec contexts */
	/*avcodec_free_context(&state->videoCodecContext);
	avcodec_free_context(&state->audioCodecContext);*/

	avcodec_close(state->audioAvStream->codec);
	avcodec_close(state->videoAvStream->codec);

	if (!(state->formatContext->oformat->flags & AVFMT_NOFILE))
		avio_close(state->formatContext->pb);

	av_free(state->formatContext);
}

void	playReplay(OsuReplay *replay, OsuMap *beatmap, sfVector2u size, Dict *sounds, Dict *images, char *path)
{
	sfVideoMode		mode = {size.x, size.y, 32};
	sfEvent			event;
	sfClock			*clock = NULL;
	replayPlayerState	state;

	startReplaySession(&state, path, beatmap, size);

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

		FrameBuffer_clear(&state.frameBuffer, (sfColor){0, 0, 0, 255});
		FrameBuffer_drawImage(
			&state.frameBuffer,
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

		FrameBuffer_drawFilledRectangle(&state.frameBuffer, (sfVector2i){10, 10}, (sfVector2u){300 * state.life, 20}, (sfColor){255, 255, 255, 255});
		FrameBuffer_drawImage(&state.frameBuffer, (sfVector2i){state.cursorPos.x, state.cursorPos.y}, Dict_getElement(images, "cursor"), (sfVector2i){-1, -1}, (sfColor){255, 255, 255, 255}, true, 0);

		if (!path) {
			FrameBuffer_draw(&state.frameBuffer, window);
			sfRenderWindow_display(window);
			if (!clock)
				clock = sfClock_create();
		} else {
			FrameBuffer_encode(&state.frameBuffer, &state);
			encodePlayingSounds(&state);
			state.frameNb++;
		}
	}
	finishReplaySession(&state);
}

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libavutil/channel_layout.h>
#include <osu_replay_parser.h>
#include <osu_map_parser.h>
#include <SFML/Graphics.h>
#include <concatf.h>
#include <utils.h>
#include "sound.h"
#include "display.h"
#include "skin.h"
#include "dict.h"
#include "globals.h"
#include "replay_player.h"

AVCodecContext *initVideoCodec(sfVector2u size, unsigned frameRate, uint64_t bitRate)
{
	const AVCodec	*codec;
	AVCodecContext	*codecContext;

	/* find the video encoder */
	codec = avcodec_find_encoder(AV_CODEC_ID_MPEG1VIDEO);
	if (!codec)
		display_error("MPEG1VIDEO codec not found\n");
	codecContext = avcodec_alloc_context3(codec);

	/* put sample parameters */
	codecContext->bit_rate = bitRate;

	/* resolution must be a multiple of two */
	codecContext->width = size.x;
	codecContext->height = size.y;

	/* frames per second */
	codecContext->time_base = (AVRational){1, frameRate};
	codecContext->framerate = (AVRational){frameRate, 1};

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
	codec = avcodec_find_encoder(AV_CODEC_ID_MP2);
	if (!codec)
		display_error("MP2 codec not found\n");
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

void	startReplaySession(ReplayPlayerState *state, const ReplayConfig *config)
{
	printf("Starting replay session...\n");

	memset(state, 0, sizeof(*state));

	/* Put base values */
	state->life = 1;
	state->beginCombo = 1;
	state->replay = config->replay;
	state->beatmap = config->beatmap;
	state->sounds = config->sounds;
	state->images = config->images;
	state->bgAlpha = config->bgAlpha;
	state->minNbAlpha = config->bgAlpha;
	state->totalFrames = config->replay->replayLength * config->frameRate / ((config->replay->mods & MODE_DOUBLE_TIME) ? 1500 : 1000) + 1;

	if (config->beatmap->backgroundPath)
		state->backgroundPictureIndex = strToLower(getFileName(config->beatmap->backgroundPath));
	if (!state->backgroundPictureIndex || !Dict_getElement(state->images, state->backgroundPictureIndex))
		display_warning("The beatmap is missing an background picture file");

	if (config->beatmap->generalInfos.audioFileName)
		state->musicIndex = strToLower(getFileName(config->beatmap->generalInfos.audioFileName));
	if (!state->musicIndex || !Dict_getElement(state->sounds, state->musicIndex))
		display_warning("The beatmap is missing an music file");

	state->played = calloc(config->beatmap->hitObjects.length, sizeof(*state->played));
	if (!state->played)
		display_error("Memory allocation error (%luB)\n", (unsigned long)sizeof(*state->played) * (unsigned long)config->beatmap->hitObjects.length);

	/* init framebuffer */
	FrameBuffer_init(&state->frameBuffer, config->resolution);
	state->frameBuffer.scale.x = config->resolution.x / 640.;
	state->frameBuffer.scale.y = config->resolution.y / 480.;

	/* Debug mode -> don't create audio/video contexts */
	if (!config->filePath)
		return;

	if (strstr(config->filePath, "'"))
		display_error("Invalid filename provided: Name cannot contain \"'\"");

	if (strstr(config->filePath, "\""))
		display_error("Invalid filename provided: Name cannot contain \"\"\"");

	char	audioPath[strlen(config->filePath) + 5];
	char	videoPath[strlen(config->filePath) + 5];

	/* Get paths */
	sprintf(audioPath, "%s.mp2", config->filePath);
	sprintf(videoPath, "%s.mp4", config->filePath);

	/* register all codecs */
	avcodec_register_all();

	/* init codecs */
	state->videoCodecContext = initVideoCodec(config->resolution, config->frameRate, config->bitRate);
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
	state->playingSounds = calloc(1, sizeof(*state->playingSounds));
	if (!state->playingSounds)
		display_error("Memory allocation error (%lu)\n", (unsigned long)sizeof(*state->playingSounds));

	/* open files */
	state->videoStream = fopen(videoPath, "wb");
	if (!state->videoStream)
		display_error("Cannot open %s: %s\n", videoPath, strerror(errno));
	state->audioStream = fopen(audioPath, "wb");
	if (!state->audioStream)
		display_error("Cannot open %s: %s\n", audioPath, strerror(errno));
}

void	destroyAVLibElements(ReplayPlayerState *state)
{
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

void	finishReplaySession(ReplayPlayerState *state, const ReplayConfig *config)
{
	uint8_t endcode[] = {0, 0, 1, 0xb7};

	if (!state->videoStream)
		return;

	/* destroy framebuffer */
	FrameBuffer_destroy(&state->frameBuffer);

	printf("Finishing replay...\n");

	/* flush audio */
	encodePlayingSounds(state, true);

	/* flush the encoders */
	encodeVideoFrame(state->videoCodecContext, NULL, state->videoPacket, state->videoStream);
	encodeAudioFrame(state->audioCodecContext, NULL, state->audioPacket, state->videoStream);

	/* add sequence end code to have a real MPEG file */
	fwrite(endcode, 1, sizeof(endcode), state->videoStream);

	/* close files */
	fclose(state->videoStream);
	fclose(state->audioStream);

	destroyAVLibElements(state);

	free(state->playingSounds);

	char	cwd[PATH_MAX];

	getcwd(cwd, sizeof(cwd));

	/* Mix created files */
#ifdef _WIN32
	char	commandBuffer[47 + getNbrLen(config->bitRate, 10) + strlen(cwd) * 3 + strlen(config->filePath) * 3];

	sprintf(commandBuffer, "ffmpeg -y -i \"%s\\%s.mp2\" -i \"%s\\%s.mp4\" -b:v %lu \"%s\\%s\" 1>&2", cwd, config->filePath, cwd, config->filePath, config->bitRate, cwd, config->filePath);
#else
	char	commandBuffer[43 + getNbrLen(config->bitRate, 10) + strlen(cwd) + strlen(config->filePath) * 3];

	sprintf(commandBuffer, "cd %s && ffmpeg -y -i '%s.mp2' -i '%s.mp4' -b:v %lu '%s' 1>&2", config->filePath, config->filePath, config->filePath, config->bitRate, config->filePath);
#endif

	printf("Executing command: %s\n", commandBuffer);
	int code = system(commandBuffer);

	if (code)
		display_error("Command \"%s\" failed with error code %i\nYou can find %s.mp4 %s.mp2 which are the generated audio and video.", commandBuffer, code, config->filePath, config->filePath);

	if (config->cleanUp) {
		printf("Cleaning up\n");

		char audioPath[strlen(config->filePath) + 5];
		char videoPath[strlen(config->filePath) + 5];

		/* Get paths */
		sprintf(audioPath, "%s.mp2", config->filePath);
		sprintf(videoPath, "%s.mp4", config->filePath);

		/* Delete temp files */
		remove(audioPath);
		remove(videoPath);
	}
}

void	drawBackground(ReplayPlayerState *state)
{
	sfVector2f	scale = state->frameBuffer.scale;
	sfVector2i	size;
	sfVector2i	pos;
	sfVector2u	imgSize;
	double		ratio;

	if (!Dict_getElement(state->images, state->backgroundPictureIndex))
		return;

	imgSize = sfImage_getSize(Dict_getElement(state->images, state->backgroundPictureIndex));
	if ((double)imgSize.x / state->frameBuffer.size.x > (double)imgSize.y / state->frameBuffer.size.y)
		ratio = (double)imgSize.y / state->frameBuffer.size.y;
	else
		ratio = (double)imgSize.x / state->frameBuffer.size.x;

	size.x = imgSize.x / ratio;
	size.y = imgSize.y / ratio;
	pos.x = (state->frameBuffer.size.x - imgSize.x / ratio) / 2;
	pos.y = (state->frameBuffer.size.y - imgSize.y / ratio) / 2;

	state->frameBuffer.scale = (sfVector2f){1, 1};
	FrameBuffer_drawImage(
		&state->frameBuffer,
		pos,
		Dict_getElement(state->images, state->backgroundPictureIndex),
		size,
		(sfColor){255, 255, 255, state->bgAlpha},
		false,
		0
	);
	state->frameBuffer.scale = scale;
}

void	makeFrame(ReplayPlayerState *state)
{
	FrameBuffer_clear(&state->frameBuffer, (sfColor){0, 0, 0, 255});
	drawBackground(state);

	if ((unsigned)state->beatmap->generalInfos.audioLeadIn <= state->totalTicks && !state->musicStarted) {
		if (!state->videoStream && music) {
			sfMusic_play(music);
			if ((state->replay->mods & MODE_DOUBLE_TIME))
				sfMusic_setPitch(music, 1.5);
		} else
			playSound(state, state->beatmap->generalInfos.audioFileName, (state->replay->mods & MODE_NIGHTCORE) ? 1.5 : 1, (state->replay->mods & MODE_DOUBLE_TIME) ? 1.5 : 1);
		state->musicStarted = true;
	}

	while (
		state->currentLifeEvent < state->replay->lifeBar.length &&
		state->replay->lifeBar.content[state->currentLifeEvent].timeToHappen <= state->totalTicks
	) {
		state->life = state->replay->lifeBar.content[state->currentLifeEvent].newValue;
		state->currentLifeEvent++;
	}

	while (
		state->currentGameEvent < state->replay->gameEvents.length &&
		state->replay->gameEvents.content[state->currentGameEvent].timeToHappen <= (int)state->totalTicks
	) {
		state->cursorPos = *(sfVector2f *)&state->replay->gameEvents.content[state->currentGameEvent].cursorPos;
		state->cursorPos.x += padding.x;
		state->cursorPos.y += padding.y;
		state->pressed = state->replay->gameEvents.content[state->currentGameEvent].keysPressed;
		state->currentGameEvent++;
	}

	if (
		!state->played[state->currentGameHitObject] &&
		state->beatmap->hitObjects.content[state->currentGameHitObject].timeToAppear <= state->totalTicks &&
		state->beatmap->hitObjects.content[state->currentGameHitObject].type & HITOBJ_SLIDER
	) {
		playSound(state, "drum-hitnormal", 1, 1);
		state->played[state->currentGameHitObject]++;
	}
	while (
		state->currentGameHitObject < state->beatmap->hitObjects.length && (
			(
				!(state->beatmap->hitObjects.content[state->currentGameHitObject].type & HITOBJ_SLIDER) &&
				!(state->beatmap->hitObjects.content[state->currentGameHitObject].type & HITOBJ_SPINNER) &&
				state->beatmap->hitObjects.content[state->currentGameHitObject].timeToAppear + 20 <= state->totalTicks
			) || (
				state->beatmap->hitObjects.content[state->currentGameHitObject].type & HITOBJ_SLIDER &&
				state->beatmap->hitObjects.content[state->currentGameHitObject].timeToAppear +
				sliderLength(state->beatmap, state->currentGameHitObject, state->beatmap->timingPoints.content[state->currentTimingPoint]) <= state->totalTicks
			) || (
				state->beatmap->hitObjects.content[state->currentGameHitObject].type & HITOBJ_SPINNER &&
				*(unsigned long *)state->beatmap->hitObjects.content[state->currentGameHitObject].additionalInfos <= state->totalTicks
			)
		)
		) {
		playSound(state, "drum-hitnormal", 1, 1);
		state->played[state->currentGameHitObject] = true;
		if (state->beatmap->hitObjects.content[state->currentGameHitObject].type & HITOBJ_NEW_COMBO) {
			state->beginCombo = 0;
			state->currentComboColor = (state->currentComboColor + 1) % state->beatmap->colors.length;
		}
		state->beginCombo++;
		state->currentGameHitObject++;
	}

	displayHitObjects(state, state->beatmap);

	FrameBuffer_drawFilledRectangle(&state->frameBuffer, (sfVector2i){10, 10}, (sfVector2u){300 * state->life, 20}, (sfColor){255, 255, 255, 255});
	FrameBuffer_drawImage(
		&state->frameBuffer,
		(sfVector2i){state->cursorPos.x, state->cursorPos.y},
		Dict_getElement(state->images, "cursor"),
		(sfVector2i){-1, -1},
		(sfColor){255, 255, 255, 255},
		true,
		0
	);

	state->isEnd = state->currentGameEvent >= state->replay->gameEvents.length && state->replay->gameEvents.content[state->currentGameEvent].timeToHappen <= (long)state->totalTicks;
}

void	playReplay(const ReplayConfig *config)
{
	sfVideoMode		mode = {config->resolution.x, config->resolution.y, 32};
	sfEvent			event;
	sfClock			*clock = NULL;
	ReplayPlayerState	state;

	startReplaySession(&state, config);

	if (!config->filePath) {
		window = sfRenderWindow_create(mode, "Osu Replay Player", sfDefaultStyle, NULL);
		if (!window)
			exit(EXIT_FAILURE);
		sfRenderWindow_setFramerateLimit(window, 60);
	}

	printf("Replay length: %lums, %lu frame%s\n", config->replay->replayLength, state.totalFrames, state.totalFrames ? "" : "s");
	padding = (sfVector2u){64, 48};

	while (!state.isEnd) {
		if (!config->filePath) {
			while (sfRenderWindow_pollEvent(window, &event)) {
				if (event.type == sfEvtClosed) {
					sfRenderWindow_close(window);
					return;
				}
			}
			sfRenderWindow_clear(window, (sfColor){0, 0, 0, 255});
			if (clock) {
				if ((config->replay->mods & MODE_DOUBLE_TIME) || (config->replay->mods & MODE_NIGHTCORE))
					state.totalTicks = sfTime_asMilliseconds(sfClock_getElapsedTime(clock)) * 1.5;
				else
					state.totalTicks = sfTime_asMilliseconds(sfClock_getElapsedTime(clock));
			}
		} else
			state.totalTicks += ((config->replay->mods & MODE_DOUBLE_TIME) || (config->replay->mods & MODE_NIGHTCORE) ? 1500 : 1000) / 60.;

		makeFrame(&state);

		if (!config->filePath) {
			FrameBuffer_draw(&state.frameBuffer, window);
			sfRenderWindow_display(window);
			if (!clock)
				clock = sfClock_create();
		} else {
			FrameBuffer_encode(&state.frameBuffer, &state);
			encodePlayingSounds(&state, false);
			state.frameNb++;
		}
	}
	finishReplaySession(&state, config);
}

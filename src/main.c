#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <SFML/Graphics.h>
#include <SFML/Audio.h>
#include <concatf.h>
#include <osu_replay_parser.h>
#include <osu_map_parser.h>
#include "skin.h"
#include "dict.h"
#include "defines.h"
#include "globals.h"

void	playSound(sfSoundBuffer *buffer)
{
	static sfSound	*sounds[nbOfSound];
	static int	current = 0;
	static bool	first = true;

	if (first) {
		for (int i = 0; i < nbOfSound; i++)
			sounds[i] = sfSound_create();
		first = false;
	}
	if (!buffer)
		return;
	sfSound_setBuffer(sounds[current], buffer);
	sfSound_play(sounds[current++]);
	current %= nbOfSound;
}

sfVector2f	getTextSize(unsigned char *str, unsigned charSize)
{
	sfVector2f	size = {0, charSize};

	for (int i = 0; str[i]; i++) {
		sfGlyph	glyph = sfFont_getGlyph(font, str[i], charSize, false, 0);

		size.x += glyph.advance;
	}
	return size;
}

unsigned char	calcAlpha(OsuMap_hitObject obj, unsigned long totalTicks)
{
	if ((long)(obj.timeToAppear - totalTicks) <= 400)
		return BASE_OBJ_ALPHA;
	return (unsigned char)((long)(obj.timeToAppear - totalTicks - 400) * -BASE_OBJ_ALPHA / 400 + BASE_OBJ_ALPHA);
}

unsigned	getLastObjToDisplay(unsigned currentGameHitObject, unsigned currentTimingPoint, OsuMap *beatmap, unsigned long totalTicks)
{
	unsigned	end = currentGameHitObject;

	while (true) {
		if (end >= beatmap->hitObjects.length)
			break;
		if (beatmap->hitObjects.content[end].timeToAppear - 800 > totalTicks)
			break;
		end++;
	}
	return end;
}

void	displayApproachCircle(sfColor color, OsuMap_hitObject object, OsuMap *beatmap, unsigned long ticks, Dict *images)
{
	if (object.timeToAppear < ticks)
		return;
	FrameBuffer_drawImage(
		&frame_buffer,
		(sfVector2i){
			object.position.x + padding.x,
			object.position.y + padding.y
		},
		Dict_getElement(
			images,
			"approachcircle"
		),
		(sfVector2i){
			(54.4f - 4.48f * (float)beatmap->difficulty.circleSize + (object.timeToAppear - ticks) / 8.f) * 2,
			(54.4f - 4.48f * (float)beatmap->difficulty.circleSize + (object.timeToAppear - ticks) / 8.f) * 2
		},
		color,
		true,
		0
	);
}

void	displayHitObjects(unsigned currentComboColor, unsigned currentGameHitObject, unsigned currentTimingPoint, OsuMap *beatmap, unsigned long totalTicks, unsigned beginCombo, Dict *images, Dict *sounds)
{
	char		buffer[11];
	unsigned	end;
	unsigned	combo;
	unsigned	color;

	end = getLastObjToDisplay(currentGameHitObject, currentTimingPoint, beatmap, totalTicks);
	for (unsigned i = end - 1; i >= currentGameHitObject && (int)i >= 0; i--) {
		unsigned char	alpha = calcAlpha(beatmap->hitObjects.content[i], totalTicks);

		if (beatmap->hitObjects.content[i].type & HITOBJ_SPINNER) {
			long	duration = *(long *)beatmap->hitObjects.content[i].additionalInfos - beatmap->hitObjects.content[i].timeToAppear;
			long	remaining = *(long *)beatmap->hitObjects.content[i].additionalInfos - totalTicks;
			float	radius = beatmap->hitObjects.content[i].timeToAppear >= totalTicks ? 200 : 2 * (100 - (duration - remaining) * 100.f / duration);

			FrameBuffer_drawCircle(
				&frame_buffer,
				4,
				(sfVector2i){
					320 - radius,
					240 - radius
				},
				radius,
				(sfColor){255, 255, 255, alpha}
			);
			/*sfCircleShape_setFillColor(aproachCircle, (sfColor){0, 0, 0, 0});
			sfCircleShape_setOutlineColor(aproachCircle, (sfColor){255, 255, 255, alpha});
			sfCircleShape_setOutlineThickness(aproachCircle, 4);
			sfCircleShape_setRadius(aproachCircle, radius);
			sfCircleShape_setPosition(aproachCircle, toSfVector2f(
				320 - radius,
				240 - radius
			));
			sfRenderWindow_drawCircleShape(window, aproachCircle, NULL);*/
		} else {
			combo = beginCombo;
			color = currentComboColor;
			for (unsigned j = currentGameHitObject; j < i; j++) {
				if (beatmap->hitObjects.content[j].type & HITOBJ_NEW_COMBO) {
					combo = 0;
					color = (color + 1) % beatmap->colors.length;
				}
				combo++;
			}

			if (beatmap->hitObjects.content[i].type & HITOBJ_SLIDER) {
				FrameBuffer_drawImage(
					&frame_buffer,
					(sfVector2i){
						sliderInfos(
							beatmap->hitObjects.content[i].additionalInfos
						)->curvePoints.content[
							sliderInfos(
								beatmap->hitObjects.content[i].additionalInfos
							)->curvePoints.length - 1
						].x + padding.x,
						sliderInfos(
							beatmap->hitObjects.content[i].additionalInfos
						)->curvePoints.content[
							sliderInfos(
								beatmap->hitObjects.content[i].additionalInfos
							)->curvePoints.length - 1
						].y + padding.y
					},
					Dict_getElement(
						images,
						"sliderendcircle"
					),
					(sfVector2i){
						(54.4f - 4.48f * (float)beatmap->difficulty.circleSize) * 2,
						(54.4f - 4.48f * (float)beatmap->difficulty.circleSize) * 2
					},
					(sfColor){
						beatmap->colors.content[color].red * 0.5,
						beatmap->colors.content[color].green * 0.5,
						beatmap->colors.content[color].blue * 0.5,
						alpha
					},
					true,
					0
				);
				for (unsigned j = 0; j < sliderInfos(
					beatmap->hitObjects.content[i].additionalInfos
				)->curvePoints.length; j++) {
				/*	sfCircleShape_setFillColor(objects, (sfColor){
						beatmap->colors.content[color].red * 0.5,
						beatmap->colors.content[color].green * 0.5,
						beatmap->colors.content[color].blue * 0.5,
						alpha
					});
					sfCircleShape_setPosition(objects, toSfVector2f(
						sliderInfos(
							beatmap->hitObjects.content[i].additionalInfos
						)->curvePoints.content[j].x - 54.4f -
						4.48f * (float)beatmap->difficulty.circleSize + padding.x,
						sliderInfos(
							beatmap->hitObjects.content[i].additionalInfos
						)->curvePoints.content[j].y - 54.4f -
						4.48f * (float)beatmap->difficulty.circleSize + padding.y
					));
					sfRenderWindow_drawCircleShape(window, objects, NULL);*/
				}
			}

			sprintf(buffer, "%u", combo);
			/*
			sfText_setCharacterSize(text, 20);
			sfText_setPosition(text, toSfVector2f(
				beatmap->hitObjects.content[i].position.x -
				getTextSize((unsigned char *)buffer, 20).x / 2 + padding.x,
				beatmap->hitObjects.content[i].position.y -
				getTextSize((unsigned char *)buffer, 20).y / 2 + padding.y
			));
			sfText_setString(text, buffer);
			sfText_setColor(text, (sfColor){0, 0, 0, alpha});
			sfRenderWindow_drawText(window, text, NULL);*/
			FrameBuffer_drawImage(
				&frame_buffer,
				(sfVector2i){
					beatmap->hitObjects.content[i].position.x + padding.x,
					beatmap->hitObjects.content[i].position.y + padding.y
				},
				Dict_getElement(
					images,
					beatmap->hitObjects.content[i].type & HITOBJ_SLIDER ? "sliderstartcircle" : "hitcircle"
				),
				(sfVector2i){
					(54.4f - 4.48f * (float)beatmap->difficulty.circleSize) * 2,
					(54.4f - 4.48f * (float)beatmap->difficulty.circleSize) * 2
				},
				(sfColor){
					beatmap->colors.content[color].red * 0.5,
					beatmap->colors.content[color].green * 0.5,
					beatmap->colors.content[color].blue * 0.5,
					alpha
				},
				true,
				0
			);
			FrameBuffer_drawImage(
				&frame_buffer,
				(sfVector2i){
					beatmap->hitObjects.content[i].position.x + padding.x,
					beatmap->hitObjects.content[i].position.y + padding.y
				},
				Dict_getElement(
					images,
					beatmap->hitObjects.content[i].type & HITOBJ_SLIDER ? "sliderstartcircleoverlay" : "hitcircleoverlay"
				),
				(sfVector2i){
					(54.4f - 4.48f * (float)beatmap->difficulty.circleSize) * 2,
					(54.4f - 4.48f * (float)beatmap->difficulty.circleSize) * 2
				},
				(sfColor){255, 255, 255, alpha},
				true,
				0
			);
			displayApproachCircle(
				(sfColor){
					beatmap->colors.content[color].red,
					beatmap->colors.content[color].green,
					beatmap->colors.content[color].blue,
					alpha
				},
				beatmap->hitObjects.content[i],
				beatmap,
				totalTicks,
				images
			);
		}
	}
}

void	playReplay(OsuReplay *replay, OsuMap *beatmap, sfVector2u size, Dict *sounds, Dict *images)
{
	sfVideoMode	mode = {size.x, size.y, 32};
	sfEvent		event;
	unsigned long	totalTicks = 0;
	unsigned long	currentTimingPoint = 0;
	unsigned int	currentGameEvent = 0;
	unsigned int	currentLifeEvent = 0;
	unsigned int	currentGameHitObject = 0;
	unsigned int	beginCombo = 1;
	unsigned int	currentComboColor = 0;
	sfVector2f	cursorPos = {0, 0};
	unsigned int	pressed = 0;
	int		sound = 0;
	float		life = 1;
	unsigned long	time = 0;
	bool		musicPlayed = false;
	sfClock		*clock = NULL;
	bool		played[beatmap->hitObjects.length];
	float		angle = 0;
	int		bgAlpha = 120;

	memset(played, 0, sizeof(played));
	font = sfFont_createFromFile("arial.ttf");
	window = sfRenderWindow_create(mode, "Osu Replay Player", sfDefaultStyle, NULL);
	if (!window || !font)
		exit(EXIT_FAILURE);

	padding = (sfVector2u){64, 48};

	beatmap->backgroundPath = beatmap->backgroundPath ? strToLower(getFileName(beatmap->backgroundPath)) : NULL;
	printf("%s\n", beatmap->backgroundPath ?: "(null)");
	FrameBuffer_init(&frame_buffer, size);
	sfRenderWindow_setFramerateLimit(window, 60);
	while (sfRenderWindow_isOpen(window)) {
		while (sfRenderWindow_pollEvent(window, &event)) {
			if (event.type == sfEvtClosed)
				sfRenderWindow_close(window);
			/*if (event.type == sfEvtKeyPressed && event.key.code == sfKeySpace) {
				playing = !playing;
				if (music) {
					sfClock_restart(clock);
					sfMusic_pause(music);
				}
			}*/
		}
		FrameBuffer_clear(&frame_buffer, (sfColor){0, 0, 0, 255});
		sfRenderWindow_clear(window, (sfColor){0, 0, 0, 255});

		FrameBuffer_drawImage(&frame_buffer, (sfVector2i){0, 0}, Dict_getElement(images, beatmap->backgroundPath), (sfVector2i){size.x, size.y}, (sfColor){255, 255, 255, bgAlpha}, false, 0);
		time = 0;
		if (clock)
			time = sfTime_asMilliseconds(sfClock_getElapsedTime(clock));
		if ((replay->mods & MODE_DOUBLE_TIME) || (replay->mods & MODE_NIGHTCORE))
			totalTicks = time * 1.5;
		else
			totalTicks = time;

		if ((unsigned)beatmap->generalInfos.audioLeadIn <= totalTicks && music && sfMusic_getStatus(music) != sfPlaying) {
			if (!musicPlayed) {
				sfMusic_play(music);
				if ((replay->mods & MODE_DOUBLE_TIME) || (replay->mods & MODE_NIGHTCORE))
					sfMusic_setPitch(music, 1.5);
				musicPlayed = true;
			}
		}

		while (
			currentLifeEvent < replay->lifeBar.length &&
			replay->lifeBar.content[currentLifeEvent].timeToHappen <= totalTicks
		) {
			life = replay->lifeBar.content[currentLifeEvent].newValue;
			currentLifeEvent++;
		}

		while (
			currentGameEvent < replay->gameEvents.length &&
			replay->gameEvents.content[currentGameEvent].timeToHappen <= (int)totalTicks
		) {
			cursorPos = *(sfVector2f *)&replay->gameEvents.content[currentGameEvent].cursorPos;
			cursorPos.x += padding.x;
			cursorPos.y += padding.y;
			pressed = replay->gameEvents.content[currentGameEvent].keysPressed;
			currentGameEvent++;
		}
		if (
                	currentGameEvent == replay->gameEvents.length &&
                	replay->gameEvents.content[currentGameEvent].timeToHappen <= (int)totalTicks
                )
			sfRenderWindow_close(window);

		if (!played[currentGameHitObject] && beatmap->hitObjects.content[currentGameHitObject].timeToAppear <= totalTicks && beatmap->hitObjects.content[currentGameHitObject].type & HITOBJ_SLIDER) {
			playSound(Dict_getElement(sounds, "drum-hitnormal"));
			played[currentGameHitObject] = true;
		}
		while (
			currentGameHitObject < beatmap->hitObjects.length && (
				(
					!(beatmap->hitObjects.content[currentGameHitObject].type & HITOBJ_SLIDER) &&
					!(beatmap->hitObjects.content[currentGameHitObject].type & HITOBJ_SPINNER) &&
					beatmap->hitObjects.content[currentGameHitObject].timeToAppear + 20 <= totalTicks
				) || (
					beatmap->hitObjects.content[currentGameHitObject].type & HITOBJ_SLIDER &&
					beatmap->hitObjects.content[currentGameHitObject].timeToAppear +
					sliderLength(beatmap, currentGameHitObject, beatmap->timingPoints.content[currentTimingPoint]) <= totalTicks
				) || (
					beatmap->hitObjects.content[currentGameHitObject].type & HITOBJ_SPINNER &&
						(unsigned long)*(long *)beatmap->hitObjects.content[currentGameHitObject].additionalInfos <= totalTicks
				)
			)
		) {
			playSound(Dict_getElement(sounds, "drum-hitnormal"));
			played[currentGameHitObject] = true;
			if (beatmap->hitObjects.content[currentGameHitObject].type & HITOBJ_NEW_COMBO) {
				beginCombo = 0;
				currentComboColor = (currentComboColor + 1) % beatmap->colors.length;
			}
			beginCombo++;
			currentGameHitObject++;
		}

		displayHitObjects(currentComboColor, currentGameHitObject, currentTimingPoint, beatmap, totalTicks, beginCombo, images, sounds);

		char buffer[1000];
		sprintf(buffer, "Time elapsed: %.2f/%.2fs", time / 1000.f, replay->replayLength / ((replay->mods & MODE_DOUBLE_TIME) || (replay->mods & MODE_NIGHTCORE) ? 1500.f : 1000.f));
		/*sfText_setColor(text, (sfColor){255, 255, 255, 255});
		sfText_setCharacterSize(text, 15);
		sfText_setString(text, buffer);
		sfText_setPosition(text, toSfVector2f(400, 0));

		sfRenderWindow_drawText(window, text, NULL);
		sfRenderWindow_drawRectangleShape(window, lifeBar, NULL);
		sfRenderWindow_drawRectangleShape(window, cursor, NULL);*/

		FrameBuffer_drawFilledRectangle(&frame_buffer, (sfVector2i){10, 10}, (sfVector2u){300 * life, 20}, (sfColor){255, 255, 255, 255});
		FrameBuffer_drawImage(&frame_buffer, (sfVector2i){cursorPos.x, cursorPos.y}, Dict_getElement(images, "cursor"), (sfVector2i){-1, -1}, (sfColor){255, 255, 255, 255}, true, 0);

		FrameBuffer_draw(&frame_buffer, window);
		sfRenderWindow_display(window);
		if (!clock)
			clock = sfClock_create();
	}
	FrameBuffer_destroy(&frame_buffer);
}

void	loadBeatmapAssets(OsuMap *beatmap, char *path, Dict *images, Dict *sounds, Dict *loaders)
{
	char		*buffer;

	buffer = concatf("%s/%s", path, beatmap->generalInfos.audioFileName);
	music = sfMusic_createFromFile(buffer);
	loadSkin(path, images, sounds, loaders);
}

LoadingPair	*createPair(void *(*creator)(const char *), void (*destroyer)(void *), enum filetype type)
{
	LoadingPair	*buffer = malloc(sizeof(*buffer));

	if (!buffer)
		display_error("Memory allocation error\n");
	buffer->type = type;
	buffer->creator = creator;
	buffer->destroyer = destroyer;
	return buffer;
}

void	createLoader(Dict *loaders)
{
	Dict_addElement(loaders, "ogg", createPair((void *(*)(const char *))sfSoundBuffer_createFromFile, (void (*)(void *))sfSoundBuffer_destroy, SOUND), free);
	Dict_addElement(loaders, "wav", createPair((void *(*)(const char *))sfSoundBuffer_createFromFile, (void (*)(void *))sfSoundBuffer_destroy, SOUND), free);
	Dict_addElement(loaders, "png", createPair((void *(*)(const char *))sfImage_createFromFile, (void (*)(void *))sfImage_destroy, IMAGE), free);
	Dict_addElement(loaders, "jpg", createPair((void *(*)(const char *))sfImage_createFromFile, (void (*)(void *))sfImage_destroy, IMAGE), free);
}

int	main(int argc, char **args)
{
	OsuReplay	replay;
	OsuMap		beatmap;
	Dict		images = {NULL, NULL, NULL, NULL};
	Dict		sounds = {NULL, NULL, NULL, NULL};
	Dict		loaders = {NULL, NULL, NULL, NULL};
	OsuSkin		skin;

	if (argc != 3) {
		printf("Usage: %s <map.osu> <replay.osr>\n", args[0]);
		return EXIT_FAILURE;
	}
	replay = OsuReplay_parseReplayFile(args[2]);
	if (replay.error) {
		printf("Parsing for replay file '%s' failed:\n%s", args[2], replay.error);
		return EXIT_FAILURE;
	}
	beatmap = OsuMap_parseMapFile(args[1]);
	if (beatmap.error) {
		printf("Parsing for beatmap file '%s' failed:\n%s\n", args[1], beatmap.error);
		return EXIT_FAILURE;
	}
	for (int i = strlen(args[1]) - 1; i >= 0; i--) {
		if (i == 0) {
			args[1] = ".";
			break;
		}
		if (args[1][i] == '\\' || args[1][i] == '/') {
			args[1][i] = 0;
			break;
		}
	}
	createLoader(&loaders);
	if (!loadSkin("assets", &images, &sounds, &loaders))
		display_error("Default skin is invalid or corrupted\n");
	loadBeatmapAssets(&beatmap, args[1], &images, &sounds, &loaders);
	Dict_destroy(&loaders, true);
	playReplay(&replay, &beatmap, (sfVector2u){640, 480}, &sounds, &images);
	Dict_destroy(&images, true);
	Dict_destroy(&sounds, true);
	return EXIT_SUCCESS;
}
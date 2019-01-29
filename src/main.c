#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <SFML/Graphics.h>
#include <SFML/Audio.h>
#include <concatf.h>
#include <osu_replay_parser.h>
#include <osu_map_parser.h>

#define toSfVector2f(x, y)	((sfVector2f){x, y})
#define sliderInfos(elem)	((OsuMap_hitObjectSliderInfos *)elem)
#define sliderLength(beatmap, elem, timeingpt)	sliderInfos(beatmap->hitObjects.content[elem].additionalInfos)->pixelLength / (100 * beatmap->difficulty.sliderMultiplayer) * timeingpt.millisecondsPerBeat
#define BASE_OBJ_ALPHA		200
#define nbOfSound		8

sfRectangleShape*lifeBar;
sfRectangleShape*cursor;
sfCircleShape	*objects;
sfCircleShape	*aproachCircle;
sfFont		*font;
sfText		*text;
sfRenderWindow	*window;
sfMusic		*music;
sfSound		*hitsound[nbOfSound];
sfVector2u	padding;

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

void	displayApproachCircle(sfColor color, OsuMap_hitObject object, OsuMap *beatmap, unsigned long ticks)
{
	if (object.timeToAppear < ticks)
		return;
	sfCircleShape_setFillColor(aproachCircle, (sfColor){0, 0, 0, 0});
	sfCircleShape_setOutlineColor(aproachCircle, color);
	sfCircleShape_setOutlineThickness(aproachCircle, 2);
	sfCircleShape_setRadius(aproachCircle, 54.4f - 4.48f * (float)beatmap->difficulty.circleSize + (object.timeToAppear - ticks) / 8.f);
	sfCircleShape_setPosition(aproachCircle, toSfVector2f(
		object.position.x - (54.4f - 4.48f * (float)beatmap->difficulty.circleSize + (object.timeToAppear - ticks) / 8.f) + padding.x,
		object.position.y - (54.4f - 4.48f * (float)beatmap->difficulty.circleSize + (object.timeToAppear - ticks) / 8.f) + padding.y
	));
	sfRenderWindow_drawCircleShape(window, aproachCircle, NULL);
}

void	displayHitObjects(unsigned currentComboColor, unsigned currentGameHitObject, unsigned currentTimingPoint, OsuMap *beatmap, unsigned long totalTicks, unsigned beginCombo)
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

			sfCircleShape_setFillColor(aproachCircle, (sfColor){0, 0, 0, 0});
			sfCircleShape_setOutlineColor(aproachCircle, (sfColor){255, 255, 255, alpha});
			sfCircleShape_setOutlineThickness(aproachCircle, 4);
			sfCircleShape_setRadius(aproachCircle, radius);
			sfCircleShape_setPosition(aproachCircle, toSfVector2f(
				320 - radius,
				240 - radius
			));
			sfRenderWindow_drawCircleShape(window, aproachCircle, NULL);
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
				for (unsigned j = 0; j < sliderInfos(
					beatmap->hitObjects.content[i].additionalInfos)->curvePoints.length; j++) {
					sfCircleShape_setFillColor(objects, (sfColor){
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
					sfRenderWindow_drawCircleShape(window, objects, NULL);
				}
			}

			sprintf(buffer, "%u", combo);
			sfCircleShape_setRadius(objects, 54.4f - 4.48f * (float)beatmap->difficulty.circleSize);
			sfCircleShape_setFillColor(objects, (sfColor){
				beatmap->colors.content[color].red,
				beatmap->colors.content[color].green,
				beatmap->colors.content[color].blue,
				alpha
			});
			sfCircleShape_setPosition(objects, toSfVector2f(
				beatmap->hitObjects.content[i].position.x -
				(54.4f - 4.48f * (float)beatmap->difficulty.circleSize) + padding.x,
				beatmap->hitObjects.content[i].position.y -
				(54.4f - 4.48f * (float)beatmap->difficulty.circleSize) + padding.y
			));
			sfRenderWindow_drawCircleShape(window, objects, NULL);

			sfText_setCharacterSize(text, 20);
			sfText_setPosition(text, toSfVector2f(
				beatmap->hitObjects.content[i].position.x -
				getTextSize((unsigned char *)buffer, 20).x / 2 + padding.x,
				beatmap->hitObjects.content[i].position.y -
				getTextSize((unsigned char *)buffer, 20).y / 2 + padding.y
			));
			sfText_setString(text, buffer);
			sfText_setColor(text, (sfColor){0, 0, 0, alpha});
			sfRenderWindow_drawText(window, text, NULL);
			displayApproachCircle(
				(sfColor){
					beatmap->colors.content[color].red,
					beatmap->colors.content[color].green,
					beatmap->colors.content[color].blue,
					alpha
				},
				beatmap->hitObjects.content[i],
				beatmap,
				totalTicks
			);
		}
	}
}

void	playReplay(OsuReplay *replay, OsuMap *beatmap)
{
	sfVideoMode	mode = {640, 480, 32};
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

	memset(played, 0, sizeof(played));
	lifeBar = sfRectangleShape_create();
	cursor = sfRectangleShape_create();
	objects = sfCircleShape_create();
	aproachCircle = sfCircleShape_create();
	font = sfFont_createFromFile("arial.ttf");
	text = sfText_create();
	window = sfRenderWindow_create(mode, "Osu Replay Player", sfDefaultStyle, NULL);
	if (!window || !lifeBar || !cursor || !font || !text || !objects || !aproachCircle)
		exit(EXIT_FAILURE);

	padding = (sfVector2u){64, 48};
	sfText_setFont(text, font);

	sfRectangleShape_setSize(lifeBar, (sfVector2f){300, 20});
	sfRectangleShape_setPosition(lifeBar, (sfVector2f){10, 10});
	sfRectangleShape_setFillColor(lifeBar, (sfColor){255, 255, 255, 255});

	sfRectangleShape_setSize(cursor, (sfVector2f){15, 15});
	sfRectangleShape_setPosition(cursor, cursorPos);
	sfRectangleShape_setFillColor(cursor, (sfColor){255, 255, 255, 255});

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
		sfRenderWindow_clear(window, (sfColor){0, 0, 0, 255});

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
		sfRectangleShape_setSize(lifeBar, (sfVector2f){300 * life, 20});

		while (
			currentGameEvent < replay->gameEvents.length &&
			replay->gameEvents.content[currentGameEvent].timeToHappen <= totalTicks
		) {
			cursorPos = *(sfVector2f *)&replay->gameEvents.content[currentGameEvent].cursorPos;
			cursorPos.x += padding.x;
			cursorPos.y += padding.y;
			pressed = replay->gameEvents.content[currentGameEvent].keysPressed;
			currentGameEvent++;
		}
		sfRectangleShape_setPosition(cursor, cursorPos);
		sfRectangleShape_setFillColor(cursor, (sfColor){
			(pressed & INPUT_KEY1) > 0 ? 0 : 255,
			(pressed & INPUT_KEY2) > 0 ? 0 : 255,
			255,
			255
		});

		if (!played[currentGameHitObject] && *hitsound && beatmap->hitObjects.content[currentGameHitObject].timeToAppear <= totalTicks && beatmap->hitObjects.content[currentGameHitObject].type & HITOBJ_SLIDER) {
			sfSound_play(hitsound[sound]);
			sound += 1;
			sound %= nbOfSound;
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
			if (*hitsound) {
				sfSound_play(hitsound[sound]);
				sound += 1;
				sound %= nbOfSound;
			}
			played[currentGameHitObject] = true;
			if (beatmap->hitObjects.content[currentGameHitObject].type & HITOBJ_NEW_COMBO) {
				beginCombo = 0;
				currentComboColor = (currentComboColor + 1) % beatmap->colors.length;
			}
			beginCombo++;
			currentGameHitObject++;
		}

		displayHitObjects(currentComboColor, currentGameHitObject, currentTimingPoint, beatmap, totalTicks, beginCombo);

		char buffer[1000];
		sprintf(buffer, "Time elapsed: %.2f/%.2fs", time / 1000.f, replay->replayLength / ((replay->mods & MODE_DOUBLE_TIME) || (replay->mods & MODE_NIGHTCORE) ? 1500.f : 1000.f));
		sfText_setColor(text, (sfColor){255, 255, 255, 255});
		sfText_setCharacterSize(text, 15);
		sfText_setString(text, buffer);
		sfText_setPosition(text, toSfVector2f(400, 0));

		sfRenderWindow_drawText(window, text, NULL);
		sfRenderWindow_drawRectangleShape(window, lifeBar, NULL);
		sfRenderWindow_drawRectangleShape(window, cursor, NULL);

		sfRenderWindow_display(window);
		if (!clock)
			clock = sfClock_create();
	}
}

void	loadBeatmapAssets(OsuMap *beatmap, char *path)
{
	char		*buffer;
	sfSoundBuffer	*sBuffer;

	buffer = concatf("%s/%s", path, beatmap->generalInfos.audioFileName);
	music = sfMusic_createFromFile(buffer);
	free(buffer);
	buffer = concatf("%s/drum-hitnormal.wav", path);
	sBuffer = sfSoundBuffer_createFromFile(buffer);
	free(buffer);
	if (!sBuffer)
		sBuffer = sfSoundBuffer_createFromFile("assets/drum-hitnormal.wav");
	memset(hitsound, 0, sizeof(hitsound));
	for (int i = 0; sBuffer && i < nbOfSound; i++) {
		hitsound[i] = sfSound_create();
		sfSound_setBuffer(hitsound[i], sBuffer);
	}
}

int	main(int argc, char **args)
{
	OsuReplay	replay;
	OsuMap		beatmap;

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
	loadBeatmapAssets(&beatmap, args[1]);
	playReplay(&replay, &beatmap);
	return EXIT_SUCCESS;
}
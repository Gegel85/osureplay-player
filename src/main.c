#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <osu_replay_parser.h>
#include <osu_map_parser.h>
#include <SFML/Graphics.h>

#define toSfVector2f(x, y)	((sfVector2f){x, y})
#define sliderInfos(elem)	((OsuMap_hitObjectSliderInfos *)elem)
#define BASE_OBJ_ALPHA		200

sfRectangleShape*lifeBar;
sfRectangleShape*cursor;
sfCircleShape	*objects;
sfFont		*font;
sfText		*text;
sfRenderWindow	*window;

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

unsigned	getLastObjToDisplay(unsigned currentGameHitObject, OsuMap *beatmap, unsigned long totalTicks)
{
	unsigned	end = currentGameHitObject;

	while (
		end < beatmap->hitObjects.length &&
		beatmap->hitObjects.content[end].timeToAppear - 800 <= totalTicks &&
		beatmap->hitObjects.content[end].timeToAppear + 20 > totalTicks
	)
		end++;
	return end;
}

void	displayHitObjects(unsigned currentComboColor, unsigned currentGameHitObject, OsuMap *beatmap, unsigned long totalTicks, unsigned beginCombo)
{
	char		buffer[11];
	unsigned	end;
	unsigned	combo;
	unsigned	color;

	end = getLastObjToDisplay(currentGameHitObject, beatmap, totalTicks);
	for (unsigned i = end - 1; i >= currentGameHitObject && (int)i > 0; i--) {
		unsigned char	alpha = calcAlpha(beatmap->hitObjects.content[i], totalTicks);

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
			for (unsigned j = 0; j < sliderInfos(beatmap->hitObjects.content[i].additionalInfos)->curvePoints.length; j++) {
				sfCircleShape_setFillColor(objects, (sfColor){
					beatmap->colors.content[color].red * 0.5,
					beatmap->colors.content[color].green * 0.5,
					beatmap->colors.content[color].blue * 0.5,
					alpha
				});
				sfCircleShape_setPosition(objects, toSfVector2f(
					sliderInfos(beatmap->hitObjects.content[i].additionalInfos)->curvePoints.content[j].x - (54.4f - 4.48f * (float)beatmap->difficulty.circleSize),
					sliderInfos(beatmap->hitObjects.content[i].additionalInfos)->curvePoints.content[j].y - (54.4f - 4.48f * (float)beatmap->difficulty.circleSize)
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
			beatmap->hitObjects.content[i].position.x - (54.4f - 4.48f * (float)beatmap->difficulty.circleSize),
			beatmap->hitObjects.content[i].position.y - (54.4f - 4.48f * (float)beatmap->difficulty.circleSize)
		));
		sfRenderWindow_drawCircleShape(window, objects, NULL);
		sfText_setCharacterSize(text, 20);
		sfText_setPosition(text, toSfVector2f(
			beatmap->hitObjects.content[i].position.x - getTextSize((unsigned char *)buffer, 20).x / 2,
			beatmap->hitObjects.content[i].position.y - getTextSize((unsigned char *)buffer, 20).y / 2
		));
		sfText_setString(text, buffer);
		sfText_setColor(text, (sfColor){0, 0, 0, alpha});
		sfRenderWindow_drawText(window, text, NULL);
	}
}

void	playReplay(OsuReplay *replay, OsuMap *beatmap)
{
	sfVideoMode	mode = {640, 480, 32};
	sfEvent		event;
	unsigned long	totalTicks = 0;
	unsigned long	ticks = 0;
	unsigned int	currentGameEvent = 0;
	unsigned int	currentLifeEvent = 0;
	unsigned int	currentGameHitObject = 0;
	unsigned int	beginCombo = 1;
	unsigned int	currentComboColor = 0;
	sfVector2f	cursorPos = {0, 0};
	unsigned int	pressed = 0;
	float		life = 1;
	bool		playing = false;

	lifeBar = sfRectangleShape_create();
	cursor = sfRectangleShape_create();
	objects = sfCircleShape_create();
	font = sfFont_createFromFile("arial.ttf");
	text = sfText_create();
	window = sfRenderWindow_create(mode, "Osu Replay Player", sfDefaultStyle, NULL);
	if (!window || !lifeBar || !cursor || !font || !text || !objects)
		exit(EXIT_FAILURE);

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
			if (event.type == sfEvtKeyPressed && event.key.code == sfKeySpace)
				playing = !playing;
		}
		sfRenderWindow_clear(window, (sfColor){0, 0, 0, 255});
		if (playing) {
			totalTicks += 16;
			ticks += 16;

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
				replay->gameEvents.content[currentGameEvent].timeToHappen <= ticks
			) {
				ticks -= replay->gameEvents.content[currentGameEvent].timeToHappen;
				cursorPos = *(sfVector2f *)&replay->gameEvents.content[currentGameEvent].cursorPos;
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

			while (
				currentGameHitObject < beatmap->hitObjects.length &&
				beatmap->hitObjects.content[currentGameHitObject].timeToAppear + 20 < totalTicks
			) {
				if (beatmap->hitObjects.content[currentGameHitObject].type & HITOBJ_NEW_COMBO) {
					beginCombo = 0;
					currentComboColor = (currentComboColor + 1) % beatmap->colors.length;
				}
				beginCombo++;
				currentGameHitObject++;
			}
		}
		displayHitObjects(currentComboColor, currentGameHitObject, beatmap, totalTicks, beginCombo);

		sfRenderWindow_drawRectangleShape(window, lifeBar, NULL);
		sfRenderWindow_drawRectangleShape(window, cursor, NULL);

		sfRenderWindow_display(window);
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
	playReplay(&replay, &beatmap);
	return EXIT_SUCCESS;
}
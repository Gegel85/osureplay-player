#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <osu_replay_parser.h>
#include <osu_map_parser.h>
#include <SFML/Graphics.h>

#define toSfVector2f(x, y)	((sfVector2f){x, y})
#define sliderInfos(elem)	((OsuMap_hitObjectSliderInfos *)elem)

void	playReplay(OsuReplay replay, OsuMap beatmap)
{
	sfVideoMode	mode = {640, 480, 32};
	sfFont		*font = sfFont_createFromFile("arial.ttf");
	sfText		*text = sfText_create();
	sfRenderWindow	*window = sfRenderWindow_create(mode, "Osu Replay Player", sfDefaultStyle, NULL);
	sfEvent		event;
	sfRectangleShape*lifeBar = sfRectangleShape_create();
	sfRectangleShape*cursor = sfRectangleShape_create();
	sfRectangleShape*objects = sfRectangleShape_create();
	unsigned long	totalTicks = 0;
	unsigned long	ticks = 0;
	unsigned int	currentGameEvent = 0;
	unsigned int	currentLifeEvent = 0;
	unsigned int	currentGameHitObject = 0;
	unsigned int	beginCombo = 1;
	sfVector2f	cursorPos = {0, 0};
	unsigned int	pressed = 0;
	float		life = 1;
	bool		playing = false;

	if (!window || !lifeBar || !cursor || !font || !text || !objects)
		exit(EXIT_FAILURE);

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
				currentLifeEvent < replay.lifeBar.length &&
				replay.lifeBar.content[currentLifeEvent].timeToHappen <= totalTicks
			) {
				life = replay.lifeBar.content[currentLifeEvent].newValue;
				currentLifeEvent++;
			}
			sfRectangleShape_setSize(lifeBar, (sfVector2f){300 * life, 20});

			while (
				currentGameEvent < replay.gameEvents.length &&
				replay.gameEvents.content[currentGameEvent].timeToHappen <= ticks
			) {
				ticks -= replay.gameEvents.content[currentGameEvent].timeToHappen;
				cursorPos = *(sfVector2f *)&replay.gameEvents.content[currentGameEvent].cursorPos;
				pressed = replay.gameEvents.content[currentGameEvent].keysPressed;
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
				currentGameHitObject < beatmap.hitObjects.length &&
				(unsigned long)beatmap.hitObjects.content[currentGameHitObject].timeToAppear < totalTicks
			) {
				currentGameHitObject++;
			}
			for (
				int i = currentGameHitObject;
				i < beatmap.hitObjects.length &&
				beatmap.hitObjects.content[i].timeToAppear - 800 <= totalTicks &&
				beatmap.hitObjects.content[i].timeToAppear > totalTicks;
				i++
			) {
				unsigned char	alpha = beatmap.hitObjects.content[i].timeToAppear - totalTicks < 400 ? (unsigned char)255 : (unsigned char)((beatmap.hitObjects.content[i].timeToAppear - totalTicks - 400) * 255 / 400);

				sfRectangleShape_setSize(objects, (sfVector2f){40, 40});
				sfRectangleShape_setFillColor(objects, (sfColor){
					(beatmap.hitObjects.content[i].type & HITOBJ_SLIDER) * 255,
					255,
					(beatmap.hitObjects.content[i].type & HITOBJ_SPINNER) * 255,
					alpha
				});
				sfRectangleShape_setPosition(objects, toSfVector2f(
					beatmap.hitObjects.content[i].position.x - 20,
					beatmap.hitObjects.content[i].position.y - 20
				));
				sfRenderWindow_drawRectangleShape(window, objects, NULL);
				if (beatmap.hitObjects.content[i].type & HITOBJ_SLIDER) {
					for (int j = 0; j < sliderInfos(beatmap.hitObjects.content[i].additionalInfos)->curvePoints.length; j++) {
						sfRectangleShape_setFillColor(objects, (sfColor){
							255,
							255 - 50 * j,
							255,
							255
						});
						sfRectangleShape_setPosition(objects, toSfVector2f(
							sliderInfos(beatmap.hitObjects.content[i].additionalInfos)->curvePoints.content[j].x - 20,
							sliderInfos(beatmap.hitObjects.content[i].additionalInfos)->curvePoints.content[j].y - 20
						));
						sfRenderWindow_drawRectangleShape(window, objects, NULL);
					}
				}
			}

		}
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
	playReplay(replay, beatmap);
	return EXIT_SUCCESS;
}
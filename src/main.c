#include <stdlib.h>
#include <osu_replay_parser.h>
#include <SFML/Graphics.h>

void	playReplay(OsuReplay replay)
{
	sfVideoMode	mode = {640, 480, 32};
	sfRenderWindow	*window = sfRenderWindow_create(mode, "Osu Replay Player", sfDefaultStyle, NULL);
	sfEvent		event;
	sfRectangleShape*lifeBar = sfRectangleShape_create();
	sfRectangleShape*cursor = sfRectangleShape_create();
	unsigned long	totalTicks = 0;
	unsigned long	ticks = 0;
	unsigned int	currentGameEvent = 0;
	unsigned int	currentLifeEvent = 0;
	sfVector2f	cursorPos = {0, 0};
	unsigned int	pressed = 0;
	float		life = 1;
	bool		playing = false;

	if (!window || !lifeBar || !cursor)
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
				currentLifeEvent < replay.lifeBar.length - 1 &&
				replay.lifeBar.content[currentLifeEvent].timeToHappen <= totalTicks
				) {
				life = replay.lifeBar.content[currentLifeEvent].newValue;
				currentLifeEvent++;
			}
			sfRectangleShape_setSize(lifeBar, (sfVector2f){300 * life, 20});

			while (
				currentGameEvent < replay.gameEvents.length - 1 &&
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
		}
		sfRenderWindow_drawRectangleShape(window, lifeBar, NULL);
		sfRenderWindow_drawRectangleShape(window, cursor, NULL);

		sfRenderWindow_display(window);
	}
}

int	main(int argc, char **args)
{
	OsuReplay	replay;

	if (argc != 3) {
		printf("Usage: %s <map.osu> <replay.osr>\n", args[0]);
		return EXIT_FAILURE;
	}
	replay = OsuReplay_parseReplayFile(args[2]);
	if (replay.error) {
		printf("Parsing for replay file '%s' failed:\n%s", args[2], replay.error);
		return EXIT_FAILURE;
	}
	playReplay(replay);
	return EXIT_SUCCESS;
}
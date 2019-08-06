#include <stdlib.h>
#include <string.h>
#include <SFML/Audio.h>
#include <SFML/Graphics.h>
#include <concatf.h>
#include <osu_map_parser.h>
#include <osu_replay_parser.h>
#include "slider_calcs.h"
#include "globals.h"
#include "skin.h"
#include "replay_player.h"

void	loadBeatmapAssets(char *path, Dict *images, Dict *sounds, Dict *loaders)
{
	for (int i = strlen(path) - 1; i >= 0; i--) {
		if (i == 0) {
			path = ".";
			break;
		}
		if (path[i] == '\\' || path[i] == '/') {
			path[i] = 0;
			break;
		}
	}
	loadSkin(path, images, sounds, loaders);
}

LoadingPair	*createPair(void *(*creator)(const char *), void (*destroyer)(void *), enum filetype type)
{
	LoadingPair	*buffer = malloc(sizeof(*buffer));

	if (!buffer)
		display_error("Memory allocation error (%luB)\n", (unsigned long)sizeof(*buffer));
	buffer->type = type;
	buffer->creator = creator;
	buffer->destroyer = destroyer;
	return buffer;
}

void	createLoader(Dict *loaders, bool debug)
{
	if (debug) {
		Dict_addElement(loaders, "ogg", createPair((void *(*)(const char *))sfSoundBuffer_createFromFile, (void (*)(void *))sfSoundBuffer_destroy, SOUND), free);
		Dict_addElement(loaders, "wav", createPair((void *(*)(const char *))sfSoundBuffer_createFromFile, (void (*)(void *))sfSoundBuffer_destroy, SOUND), free);
	} else  {
		Dict_addElement(loaders, "ogg", createPair((void *(*)(const char *))loadSoundFile, (void (*)(void *))destroySound, SOUND), free);
		Dict_addElement(loaders, "wav", createPair((void *(*)(const char *))loadSoundFile, (void (*)(void *))destroySound, SOUND), free);
		Dict_addElement(loaders, "mp3", createPair((void *(*)(const char *))loadSoundFile, (void (*)(void *))destroySound, SOUND), free);
	}
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

	printf("Replay player version "VERSION"\n");
	if (argc != 4 && argc != 3) {
		fprintf(stdout, "Usage: %s <map.osu> <replay.osr> [<output file>]\n", args[0]);
		return EXIT_FAILURE;
	}

	printf("Loading replay file %s\n", args[2]);
	replay = OsuReplay_parseReplayFile(args[2]);
	if (replay.error) {
		fprintf(stderr, "\nParsing for replay file '%s' failed:\n%s\n", args[2], replay.error);
		return EXIT_FAILURE;
	}

	printf("Loading beatmap file %s\n", args[1]);
	beatmap = OsuMap_parseMapFile(args[1]);
	if (beatmap.error) {
		fprintf(stderr, "\nParsing for beatmap file '%s' failed:\n%s", args[1], beatmap.error);
		return EXIT_FAILURE;
	}

	printf("Loading skin assets\n");
	createLoader(&loaders, argc == 3);
	if (!loadSkin("assets", &images, &sounds, &loaders))
		display_error("Default skin is invalid or corrupted\n");

	printf("Loading beatmap skin assets\n");
	loadBeatmapAssets(args[1], &images, &sounds, &loaders);

	if (!args[3])
		music = sfMusic_createFromFile(concatf("%s/%s", args[1], beatmap.generalInfos.audioFileName));

	Dict_destroy(&loaders, true);
	for (unsigned i = 0; i < beatmap.hitObjects.length; i++)
		if (beatmap.hitObjects.content[i].type & HITOBJ_SLIDER)
			getRealPointsSliders(&beatmap.hitObjects.content[i]);
	playReplay(&replay, &beatmap, (sfVector2u){640, 480}, &sounds, &images, args[3]);

	if (!args[3])
		sfMusic_destroy(music);
	Dict_destroy(&images, true);
	Dict_destroy(&sounds, true);
	printf("Goodbye !");
	return EXIT_SUCCESS;
}
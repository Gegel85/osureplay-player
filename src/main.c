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

void	loadBeatmapAssets(OsuMap *beatmap, char *path, Dict *images, Dict *sounds, Dict *loaders)
{
	char		*buffer;

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
	buffer = concatf("%s/%s", path, beatmap->generalInfos.audioFileName);
	music = sfMusic_createFromFile(buffer);
	loadSkin(path, images, sounds, loaders);
	free(buffer);
}

LoadingPair	*createPair(void *(*creator)(const char *), void (*destroyer)(void *), enum filetype type)
{
	LoadingPair	*buffer = malloc(sizeof(*buffer));

	if (!buffer)
		display_error("Memory allocation error (%luB)\n", sizeof(*buffer));
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

	if (argc != 4) {
		printf("Usage: %s <map.osu> <replay.osr> <output file>\n", args[0]);
		return EXIT_FAILURE;
	}

	replay = OsuReplay_parseReplayFile(args[2]);
	if (replay.error)
		display_error("\nParsing for replay file '%s' failed:\n%s\n", args[2], replay.error);

	beatmap = OsuMap_parseMapFile(args[1]);
	if (beatmap.error)
		display_error("\nParsing for beatmap file '%s' failed:\n%s", args[1], beatmap.error);

	createLoader(&loaders);
	if (!loadSkin("assets", &images, &sounds, &loaders))
		display_error("Default skin is invalid or corrupted\n");
	loadBeatmapAssets(&beatmap, args[1], &images, &sounds, &loaders);

	Dict_destroy(&loaders, true);
	for (unsigned i = 0; i < beatmap.hitObjects.length; i++)
		if (beatmap.hitObjects.content[i].type & HITOBJ_SLIDER)
			getRealPointsSliders(&beatmap.hitObjects.content[i]);
	playReplay(&replay, &beatmap, (sfVector2u){640, 480}, &sounds, &images, strcmp(args[3], "debug") == 0 ? NULL : args[3]);

	Dict_destroy(&images, true);
	Dict_destroy(&sounds, true);
	return EXIT_SUCCESS;
}
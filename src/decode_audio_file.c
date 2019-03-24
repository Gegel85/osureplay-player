#include <libavutil/opt.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavresample/avresample.h>
#include <sound.h>
#include "audio_parsers.h"
#include "frame_buffer.h"
#include "audio_parsers.h"

Sound	*loadSoundFile(char *path)
{
	FILE	*stream = fopen(path, "rb");
	Sound	*result = NULL;

	for (int i = 0; i < PARSERS_COUNT; i++) {
		rewind(stream);
		if (audioChecker[i](stream)) {
			rewind(stream);
			result = audioParsers[i](stream);
			if (!result)
				display_warning("Cannot load file %s\n", path);
			goto end;
		}
	}
	display_warning("%s is in an unsupported format\n", path);
end:
	fclose(stream);
	return result;
}

void	defaultSoundDestroyer(Sound *sound)
{
	if (sound->data)
		free(*sound->data);
	free(sound->data);
}

void	destroySound(Sound *sound)
{
	if (sound && sound->destroyer)
		sound->destroyer(sound);
	else if (sound)
		defaultSoundDestroyer(sound);
	free(sound);
}
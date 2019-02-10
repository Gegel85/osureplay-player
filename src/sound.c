#include <SFML/Audio.h>
#include <stdbool.h>
#include <string.h>
#include "replay_player.h"
#include "defines.h"

void	playSound(replayPlayerState *state, char *index, double pitch, double speed)
{
	void	*elem = Dict_getElement(state->sounds, index);
	int	i = 0;
	PlayingSound	*best = NULL;

	if (!elem)
		return;
	if (!state->stream) {
		sfSoundBuffer *buffer = elem;
		static sfSound *sounds[nbOfSound];
		static int current = 0;
		static bool first = true;

		if (first) {
			for (; i < nbOfSound; i++)
				sounds[i] = sfSound_create();
			first = false;
		}
		sfSound_setBuffer(sounds[current], buffer);
		sfSound_setPitch(sounds[current], pitch);
		sfSound_play(sounds[current++]);
		current %= nbOfSound;
		return;
	}
	for (; state->playingSounds[i].sound; i++) {
		if (state->playingSounds[i].sound->length < state->playingSounds[i].pos * 2) {
			best = &state->playingSounds[i];
			break;
		}
	}
	if (!best) {
		state->playingSounds = realloc(state->playingSounds, sizeof(*state->playingSounds) * (i + 2));
		if (!state->playingSounds)
			display_error("Memory allocation error (%lu)\n", sizeof(*state->playingSounds) * (i + 2));
		memset(&state->playingSounds[i + 1], 0, sizeof(*state->playingSounds));
		best = &state->playingSounds[i];
	}
	memset(best, 0, sizeof(*best));
	best->sound = elem;
	best->pitch = pitch;
}

void	encodePlayingSounds(replayPlayerState *state)
{
	PlayingSound	*sounds = state->playingSounds;
	static double	total = 0;
	static FILE	*stream = NULL;

	if (!stream)
		stream = fopen("test.mp2", "wb");
	memset(state->audioFrame->data[0], 0, state->audioFrame->nb_samples * 2);
	total += state->audioCodecContext->sample_rate / 60.;
	for (int i = 0; i < state->audioFrame->nb_samples; i++) {
		for (int j = 0; sounds[j].sound; j++) {
			if (sounds[j].sound->length < sounds[j].pos * 2) {
				((short *)state->audioFrame->data[0])[i] += sounds[j].sound->data[(int)sounds[j].pos * 2] * sounds[j].pitch;
				((short *)state->audioFrame->data[1])[i] += sounds[j].sound->data[(int)sounds[j].pos * 2 + 1] * sounds[j].pitch;
				sounds[j].pos += sounds[j].speed;
			}
		}
	}

	state->audioFrame->pts = state->frameNb;

	encodeAudioFrame(state->audioCodecContext, state->audioFrame, state->audioPacket, stream);
}

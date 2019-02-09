#include <SFML/Audio.h>
#include <stdbool.h>
#include <string.h>
#include "replay_player.h"
#include "defines.h"

void	playSound(replayPlayerState *state, char *index, float pitch)
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
		if (state->playingSounds[i].sound->length < state->playingSounds[i].pos) {
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

	total += state->audioCodecContext->sample_rate / 60.;
	for (int sample = 0; total-- >= 1; sample++) {
		for (int i = 0; sounds[i].sound; i++) {
			state->audioFrames[0]->nb_samples
			state->audioFrames[0]->data[0][sample] = 42;
			state->audioFrames[1]->data[0][sample] = 0;
		}
	}
	encodeFrame(state->audioCodecContext, state->audioFrames[0], state->audioPacket, state->stream);
	encodeFrame(state->audioCodecContext, state->audioFrames[1], state->audioPacket, state->stream);
}

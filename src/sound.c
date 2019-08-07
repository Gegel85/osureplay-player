#include <SFML/Audio.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include "replay_player.h"
#include "defines.h"

void	playSound(ReplayPlayerState *state, const char *index, double pitch, double speed)
{
	void	*elem = Dict_getElement(state->sounds, index);
	int	i = 0;
	PlayingSound	*best = NULL;

	if (!elem) {
		display_warning("Cannot play sound %s\n", index);
		return;
	}
	if (!state->videoStream) {
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
		size_t len = 0;

		for (unsigned k = 0; k < state->playingSounds[i].sound->nbChannels; len = len < state->playingSounds[i].sound->length[k++] ? state->playingSounds[i].sound->length[k - 1] : len);
		if (len <= state->playingSounds[i].pos) {
			best = &state->playingSounds[i];
			break;
		}
	}
	if (!best) {
		state->playingSounds = realloc(state->playingSounds, sizeof(*state->playingSounds) * (i + 2));
		if (!state->playingSounds)
			display_error("Memory allocation error (%lu)\n", (unsigned long)sizeof(*state->playingSounds) * (i + 2));
		memset(&state->playingSounds[i + 1], 0, sizeof(*state->playingSounds));
		best = &state->playingSounds[i];
	}
	memset(best, 0, sizeof(*best));
	best->sound = elem;
	best->speed = speed;
	best->pitch = pitch;
}

void	encodePlayingSounds(ReplayPlayerState *state, bool last)
{
	static unsigned	i = 0;
	static int	index = 0;
	PlayingSound	*sounds = state->playingSounds;
	short		*frameSampleBuffer = (short *)state->audioFrame->data[0];

	if (!i)
		memset(frameSampleBuffer, 0, state->audioCodecContext->frame_size * sizeof(*frameSampleBuffer));

	if (last) {
		if (av_frame_make_writable(state->audioFrame) < 0)
			display_error("Frame is not writable");
		encodeAudioFrame(state->audioCodecContext, state->audioFrame, state->audioPacket, state->audioStream);
		memset(frameSampleBuffer, 0, state->audioCodecContext->frame_size * sizeof(*frameSampleBuffer));
		i = 0;
		index = 0;
		return;
	}

	for (; i < SAMPLE_RATE * state->frameNb / 60; i++) {
		for (int j = 0; sounds[j].sound; j++) {
			if (sounds[j].sound->length[0] > sounds[j].pos) {
				frameSampleBuffer[index] += sounds[j].sound->data[0][(int)sounds[j].pos] / 2.;
				sounds[j].pos += sounds[j].speed;
			}
		}
		index++;
		if (index >= state->audioCodecContext->frame_size) {
			if (av_frame_make_writable(state->audioFrame) < 0)
				display_error("Frame is not writable");
			encodeAudioFrame(state->audioCodecContext, state->audioFrame, state->audioPacket, state->audioStream);
			index = 0;
			memset(frameSampleBuffer, 0, state->audioCodecContext->frame_size * sizeof(*frameSampleBuffer));
		}
	}
}

//
// Created by Gegel85 on 24/03/2019.
//

#ifndef OSUREPLAY_PLAYER_WAV_PARSER_H
#define OSUREPLAY_PLAYER_WAV_PARSER_H


#include <stdio.h>
#include "sound.h"

struct wavFileHeader {
	char		riff[4];		// RIFF string
	unsigned	overallSize;		// overall size of file in bytes
	char		wave[4];		// WAVE string
	unsigned	formatType;		// format type. 1-PCM, 3- IEEE float, 6 - 8bit A law, 7 - 8bit mu law
	unsigned	channels;		// no.of channels
	unsigned	sampleRate;		// sampling rate (blocks per second)
	unsigned	byteRate;		// SampleRate * NumChannels * BitsPerSample/8
	unsigned	blockAlign;		// NumChannels * BitsPerSample/8
	unsigned	bitsPerSample;		// bits per sample, 8- 8bits, 16- 16 bits etc
	unsigned	dataSize;		// NumSamples * NumChannels * BitsPerSample/8 - size of the next chunk that will be read
};

bool checkWavFile(FILE *stream);
Sound *parseWavFile(FILE *stream);


#endif //OSUREPLAY_PLAYER_WAV_PARSER_H

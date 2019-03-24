//
// Created by Gegel85 on 24/03/2019.
//

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include "wav_parser.h"
#include "sound.h"
#include "frame_buffer.h"

#define INVALID_HEADER *header.riff = 0, header
#define data1b	((char **)data)
#define data2b	((short **)data)
#define data4b	((int **)data)

bool	checkWavFile(FILE *stream)
{
	char buffer[4];

	memset(&buffer, 0, sizeof(buffer));

	//Read RIFF string
	fread(buffer, sizeof(buffer), 1, stream);
	if (strncmp("RIFF", buffer, 4) != 0)
		return false;

	//Skip file size
	fread(buffer, sizeof(buffer), 1, stream);

	//Read WAVE string
	fread(buffer, sizeof(buffer), 1, stream);
	if (strncmp("WAVE", buffer, 4) != 0)
		return false;

	return true;
}

struct wavFileHeader	parseWavHeader(FILE *stream)
{
	struct wavFileHeader	header;
	unsigned char		buffer[4];
	char			section_type[4];
	unsigned		section_size;
	bool			fmt_found = false;
	long			current_pos;

	memset(&header, 0, sizeof(header));

	//Read RIFF string
	fread(header.riff, sizeof(header.riff), 1, stream);
	if (strncmp("RIFF", header.riff, 4) != 0) {
		display_warning("Error: RIFF string is invalid '%c%c%c%c'\n", header.riff[0], header.riff[1], header.riff[2], header.riff[3]);
		return INVALID_HEADER;
	}

	//Get file size
	fread(buffer, sizeof(buffer), 1, stream);
	header.overallSize = (buffer[3] << 24) | (buffer[2] << 16) | (buffer[1] << 8) | buffer[0];

	//Read WAVE string
	fread(header.wave, sizeof(header.wave), 1, stream);
	if (strncmp("WAVE", header.wave, 4) != 0) {
		display_warning("Error: WAVE string is invalid '%c%c%c%c'\n", header.wave[0], header.wave[1], header.wave[2], header.wave[3]);
		return INVALID_HEADER;
	}

	//Read section type marker
	fread(section_type, sizeof(section_type), 1, stream);
	for (int i = 0; i < 4; i++)
		section_type[i] = isupper(section_type[i]) ? section_type[i] - 'A' + 'a' : section_type[i];

	while (strncmp("data", section_type, 4) != 0) {
		//Get section type size
		fread(buffer, sizeof(buffer), 1, stream);
		section_size = (buffer[3] << 24) | (buffer[2] << 16) | (buffer[1] << 8) | buffer[0];

		current_pos = ftell(stream);
		if (strncmp("fmt ", section_type, 4) == 0) {
			if (fmt_found) {
				display_warning("Error: Two fmt sections found in header %i\n", header.formatType);
				return INVALID_HEADER;
			}
			fmt_found = true;

			//Read format type
			fread(buffer, 2, 1, stream);
			header.formatType = (buffer[1] << 16) | buffer[0];
			if (header.formatType != 1) {
				display_warning("Error: Unsupported format type %i\n", header.formatType);
				return INVALID_HEADER;
			}

			//Read the number of channels
			fread(buffer, 2, 1, stream);
			header.channels = (buffer[1] << 16) | buffer[0];

			//Get sample rate
			fread(buffer, sizeof(buffer), 1, stream);
			header.sampleRate = (buffer[3] << 24) | (buffer[2] << 16) | (buffer[1] << 8) | buffer[0];

			//Get byte rate
			fread(buffer, sizeof(buffer), 1, stream);
			header.byteRate = (buffer[3] << 24) | (buffer[2] << 16) | (buffer[1] << 8) | buffer[0];

			//Get block alignment
			fread(buffer, 2, 1, stream);
			header.blockAlign = (buffer[1] << 8) | buffer[0];

			//Get bits per sample
			fread(buffer, 2, 1, stream);
			header.bitsPerSample = (buffer[1] << 8) | buffer[0];
		}
		fseek(stream, current_pos + section_size, SEEK_SET);

		//Read section type marker
		fread(section_type, sizeof(section_type), 1, stream);
		for (int i = 0; i < 4; i++)
			section_type[i] = isupper(section_type[i]) ? section_type[i] - 'A' + 'a' : section_type[i];
	}

	//Get data size
	fread(buffer, sizeof(buffer), 1, stream);
	header.dataSize = (buffer[3] << 24) | (buffer[2] << 16) | (buffer[1] << 8) | buffer[0];

	unsigned	size_of_each_sample = (header.channels * header.bitsPerSample) / 8;
	unsigned	bytes_in_each_channel = (size_of_each_sample / header.channels);

	if ((bytes_in_each_channel * header.channels) != size_of_each_sample) {
		display_warning("Error: %u x %u != %u\n", bytes_in_each_channel, header.channels, size_of_each_sample);
		return INVALID_HEADER;
	}

	if (header.byteRate != header.sampleRate * header.bitsPerSample * header.channels / 8) {
		display_warning("Error: %u x %u x %u / 8 != %u\n", header.sampleRate, header.bitsPerSample, header.channels, header.byteRate);
		return INVALID_HEADER;
	}

	if (header.blockAlign != header.channels * header.bitsPerSample / 8) {
		display_warning("Error: %u x %u / 8 != %u\n", header.bitsPerSample, header.channels, header.blockAlign);
		return INVALID_HEADER;
	}

	return header;
}

void	loadWavDataChunk(struct wavFileHeader *header, FILE *stream, Sound *sound)
{
	unsigned	num_samples = (8 * header->dataSize) / (header->channels * header->bitsPerSample);
	unsigned	size_of_each_sample = (header->channels * header->bitsPerSample) / 8;
	char		buffer[(header->channels * header->bitsPerSample) / 8];
	unsigned	bytes_in_each_channel = (size_of_each_sample / header->channels);
	void		**data;
	size_t		*length;

	if ((bytes_in_each_channel * header->channels) != size_of_each_sample) {
		display_warning("Error: %u x %ud != %u\n", bytes_in_each_channel, header->channels, size_of_each_sample);
		sound->data = NULL;
		return;
	}

	sound->data = data = malloc(sizeof(*data) * header->channels);
	if (!data)
		display_error("Memory allocation error (%luB)", sizeof(*data) * header->channels);
	memset(data, 0, sizeof(*data) * header->channels);

	*data = malloc(header->bitsPerSample / 8 * num_samples * header->channels);
	if (!*data)
		display_error("Memory allocation error (%uB)", header->bitsPerSample / 8 * num_samples * header->channels);
	memset(*data, 0, header->bitsPerSample / 8 * num_samples * header->channels);

	sound->length = length = malloc(sizeof(*length) * header->channels);
	if (!length)
		display_error("Memory allocation error (%luB)", sizeof(*length) * header->channels);
	memset(length, 0, sizeof(*length) * header->channels);

	for (unsigned i = 0; i < header->channels; i++) {
		data[i] = &data1b[0][header->bitsPerSample / 8 * num_samples * i];
		length[i] = num_samples;
	}
	for (unsigned i = 0; i < num_samples; i++) {
		if (fread(buffer, sizeof(buffer), 1, stream) != 1) {
			display_warning("Unexpected EOF when reading wav file\n");
			free(*data);
			free(data);
			free(length);
			sound->data = NULL;
			return;
		}
		for (unsigned j = 0; j < header->channels; j++) {
			switch (bytes_in_each_channel) {
			case 4:
				data4b[j][i] = (buffer[j * 4 + 3] << 24) | (buffer[j * 4 + 2] << 16) | (buffer[j * 4 + 1] << 8) | buffer[j * 4];
				break;
			case 2:
				data2b[j][i] = (buffer[j * 2 + 1] << 8) | buffer[j * 2];
				break;
			case 1:
				data1b[j][i] = buffer[j];
				break;
			}
		}
	}
}

Sound	*parseWavFile(FILE *stream)
{
	Sound			*sound;
	struct wavFileHeader	header = parseWavHeader(stream);

	if (!*header.riff)
		return NULL;

	sound = malloc(sizeof(*sound));
	if (!sound)
		display_error("Memory allocation error (%luB)", sizeof(*sound));
	memset(sound, 0, sizeof(*sound));

	loadWavDataChunk(&header, stream, sound);
	if (!sound->data) {
		free(sound);
		return NULL;
	}

	sound->bitsPerSample = header.bitsPerSample;
	sound->nbChannels = header.channels;
	sound->sampleRate = header.sampleRate;

	return sound;
}
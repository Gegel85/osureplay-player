#include <libavutil/opt.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
#include "sound.h"
#include "frame_buffer.h"

int decodeAudioFile(const char *path, const int sample_rate, Sound *sound)
{
	// initialize all muxers, demuxers and protocols for libavformat
	// (does nothing if called twice during the course of one program execution)
	av_register_all();

	// get format from audio file
	AVFormatContext *format = avformat_alloc_context();
	if (avformat_open_input(&format, path, NULL, NULL) != 0) {
		display_warning("Could not open file '%s'\n", path);
		return -1;
	}
	if (avformat_find_stream_info(format, NULL) < 0) {
		display_warning("Could not retrieve stream info from file '%s'\n", path);
		return -1;
	}
	sound->sampleRate = sample_rate;
	sound->bitsPerSample = 16;

	// Find the index of the first audio stream
	for (size_t stream_index = 0; stream_index < format->nb_streams; stream_index++) {
		if (format->streams[stream_index]->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
			sound->data = realloc(sound->data, (sound->nbChannels + 1) * sizeof(*sound->data));
			sound->length = realloc(sound->length, sizeof(*sound->length) * (sound->nbChannels + 1));

			if (!sound->data)
				display_error("Memory allocation error (%luB)\n", (sound->nbChannels + 1) * (unsigned long)sizeof(*sound->data));

			if (!sound->length)
				display_error("Memory allocation error (%luB)\n", (sound->nbChannels + 1) * (unsigned long)sizeof(*sound->length));

			short **data = &sound->data[sound->nbChannels];
			size_t *size = &sound->length[sound->nbChannels++];
			AVStream *stream = format->streams[stream_index];

			// find & open codec
			AVCodecContext *codec = stream->codec;
			if (avcodec_open2(codec, avcodec_find_decoder(codec->codec_id), NULL) < 0) {
				display_warning("Failed to open decoder for stream #%lu in file '%s'\n", stream_index, path);
				return -1;
			}

			// prepare resampler
			struct SwrContext* swr = swr_alloc();
			av_opt_set_int(swr, "in_channel_count",  codec->channels, 0);
			av_opt_set_int(swr, "out_channel_count", 1, 0);
			av_opt_set_int(swr, "in_channel_layout",  codec->channel_layout, 0);
			av_opt_set_int(swr, "out_channel_layout", AV_CH_LAYOUT_MONO, 0);
			av_opt_set_int(swr, "in_sample_rate", codec->sample_rate, 0);
			av_opt_set_int(swr, "out_sample_rate", sample_rate, 0);
			av_opt_set_sample_fmt(swr, "in_sample_fmt",  codec->sample_fmt, 0);
			av_opt_set_sample_fmt(swr, "out_sample_fmt", AV_SAMPLE_FMT_S16,  0);
			swr_init(swr);
			if (!swr_is_initialized(swr)) {
				display_warning("Resampler has not been properly initialized\n");
				return -1;
			}

			// prepare to read data
			AVPacket packet;
			av_init_packet(&packet);
			AVFrame* frame = av_frame_alloc();
			if (!frame) {
				display_warning("Error allocating the frame\n");
				return -1;
			}

			// iterate through frames
			*data = NULL;
			*size = 0;
			while (av_read_frame(format, &packet) >= 0) {
				// decode one frame
				int gotFrame;
				if (avcodec_decode_audio4(codec, frame, &gotFrame, &packet) < 0)
					break;
				if (!gotFrame)
					continue;
				// resample frames
				short *buffer;
				av_samples_alloc((uint8_t**) &buffer, NULL, 1, frame->nb_samples, AV_SAMPLE_FMT_S16, 0);
				int frame_count = swr_convert(swr, (uint8_t**) &buffer, frame->nb_samples, (const uint8_t**) frame->data, frame->nb_samples);
				// append resampled frames to data
				*data = realloc(*data, (*size + frame->nb_samples) * sizeof(**data));
				memcpy(*data + *size, buffer, frame_count * sizeof(**data));
				*size += frame_count;
			}

			// clean up
			av_frame_free(&frame);
			swr_free(&swr);
			avcodec_close(codec);
		}
	}
	if (!sound->nbChannels) {
		display_warning("Could not retrieve audio stream from file '%s'\n", path);
		return -1;
	}

	avformat_free_context(format);
	// success
	return 0;

}

Sound	*loadSoundFile(char *path)
{
	Sound	*result = malloc(sizeof(*result));

	if (!result)
		display_error("Memory allocation error (%luB)\n", (unsigned long)sizeof(*result));
	memset(result, 0, sizeof(*result));
	if (decodeAudioFile(path, SAMPLE_RATE, result) == 0)
		return result;
	free(result);
	return NULL;
}

void	defaultSoundDestroyer(Sound *sound)
{
	if (sound->data)
		for (size_t i = 0; i < sound->nbChannels; i++)
			free(sound->data[i]);
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
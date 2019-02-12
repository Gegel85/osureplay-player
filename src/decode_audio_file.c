#include <libavutil/opt.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavresample/avresample.h>
#include <sound.h>
#include "frame_buffer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libavutil/channel_layout.h"
#include "libavutil/frame.h"
#include "libavutil/mem.h"

#include "libavcodec/avcodec.h"

#define AUDIO_INBUF_SIZE 2048
#define AUDIO_REFILL_THRESH 4096

bool	decodeAudioFrame(AVCodecContext *dec_ctx, AVPacket *pkt, AVFrame *frame, int16_t **outbuf, size_t *offset)
{
	int16_t *interleave_buf;
	int	ret;
	size_t	data_size;

	/* send the packet with the compressed data to the decoder */
	if (avcodec_send_packet(dec_ctx, pkt) < 0) {
		display_warning("Error submitting the packet to the decoder\n");
		return false;
	}

	/* read all the output frames (in general there may be any number of them */
	while (true) {
		ret = avcodec_receive_frame(dec_ctx, frame);
		if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
			return true;
		else if (ret < 0) {
			display_warning("Error during decoding\n");
			return false;
		}

		/* the stream parameters may change at any time, check that they are
		 * what we expect */
		if (av_get_channel_layout_nb_channels(frame->channel_layout) != 2 ||
		    frame->format != AV_SAMPLE_FMT_S16P) {
			display_warning("Unsupported frame parameters\n");
			return false;
		}

		/* The decoded data is signed 16-bit planar -- each channel in its own
		 * buffer. We interleave the two channels manually here, but using
		 * libavresample is recommended instead. */
		data_size = sizeof(*interleave_buf) * 2 * frame->nb_samples;
		interleave_buf = malloc(data_size);
		if (!interleave_buf) {
			display_warning("Memory allocation error (%lu)\n", sizeof(*interleave_buf) * 2 * frame->nb_samples);
			return false;
		}
		for (int i = 0; i < frame->nb_samples; i++) {
			interleave_buf[2 * i] = ((int16_t *)frame->data[0])[i];
			interleave_buf[2 * i + 1] = ((int16_t *)frame->data[1])[i];
		}
		memcpy(*outbuf + *offset, interleave_buf, data_size);
		free(interleave_buf);
		*offset += 2 * frame->nb_samples;
	}
}

bool	decode_audio_file(const char *path, int16_t **out, size_t *size, enum AVCodecID id)
{
	const AVCodec *codec;
	AVCodecContext *c = NULL;
	AVCodecParserContext *parser = NULL;
	int ret;
	FILE *file;
	uint8_t inbuf[AUDIO_INBUF_SIZE + AV_INPUT_BUFFER_PADDING_SIZE];
	size_t data_size, len;
	AVPacket *pkt;
	AVFrame *decoded_frame = NULL;

	pkt = av_packet_alloc();

	avcodec_register_all();

	/* find the MPEG audio decoder */
	codec = avcodec_find_decoder(id);
	if (!codec) {
		display_warning("codec not found\\n");
		return false;
	}

	parser = av_parser_init(codec->id);
	if (!parser) {
		display_warning("parser not found\n");
		avcodec_free_context(&c);
		return false;
	}

	c = avcodec_alloc_context3(codec);

	/* open it */
	if (avcodec_open2(c, codec, NULL) < 0) {
		display_warning("could not open codec\n");
		avcodec_free_context(&c);
		av_parser_close(parser);
		return false;
	}

	file = fopen(path, "rb");
	if (!file) {
		display_warning("Cannot open %s: %s\n", path, strerror(errno));
		avcodec_free_context(&c);
		av_parser_close(parser);
		return false;
	}

	/* decode until eof */
	data_size = fread(inbuf, 1, AUDIO_INBUF_SIZE, file);

	decoded_frame = av_frame_alloc();
	if (!decoded_frame) {
		display_warning("Memory allocation error\n");
		fclose(file);
		avcodec_free_context(&c);
		av_parser_close(parser);
		av_packet_free(&pkt);
		return false;
	}
	pkt = av_packet_alloc();

	while (data_size > 0) {
		ret = av_parser_parse2(
			parser, c, &pkt->data, &pkt->size, inbuf, data_size, AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0
		);
		if (ret < 0) {
			display_warning("Error while parsing\n");
			fclose(file);
			avcodec_free_context(&c);
			av_parser_close(parser);
			av_frame_free(&decoded_frame);
			av_packet_free(&pkt);
			return false;
		}

		data_size -= ret;
		if (pkt->size && !decodeAudioFrame(c, pkt, decoded_frame, out, size)) {
			fclose(file);
			avcodec_free_context(&c);
			av_parser_close(parser);
			av_frame_free(&decoded_frame);
			av_packet_free(&pkt);
			return false;
		}
		if (data_size < AUDIO_REFILL_THRESH) {
			memmove(inbuf + ret, inbuf, data_size);
			len = fread(inbuf + data_size, 1, AUDIO_INBUF_SIZE - data_size, file);
			if (len > 0)
				data_size += len;
		}
	}

	/* flush the decoder */
	pkt->data = NULL;
	pkt->size = 0;
	decodeAudioFrame(c, pkt, decoded_frame, out, size);
	fclose(file);
	avcodec_free_context(&c);
	av_parser_close(parser);
	av_frame_free(&decoded_frame);
	av_packet_free(&pkt);
	return true;
}

Sound	*loadWavFile(char *path)
{
	Sound	*sound = malloc(sizeof(*sound));

	if (!sound)
		display_error("Memory allocation error (%lu)\n", sizeof(*sound));
	memset(sound, 0, sizeof(*sound));
	if (decode_audio_file(path, &sound->data, &sound->length, AV_CODEC_ID_MPEG4))
		return sound;
	display_warning("An error occured when reading file %s\n", path);
	free(sound);
	return NULL;
}

Sound	*loadMp3File(char *path)
{
	Sound	*sound = malloc(sizeof(*sound));

	if (!sound)
		display_error("Memory allocation error (%lu)\n", sizeof(*sound));
	memset(sound, 0, sizeof(*sound));
	if (decode_audio_file(path, &sound->data, &sound->length, AV_CODEC_ID_MP3))
		return sound;
	display_warning("An error occured when reading file %s\n", path);
	free(sound);
	return NULL;
}

Sound	*loadOggFile(char *path)
{
	Sound	*sound = malloc(sizeof(*sound));

	if (!sound)
		display_error("Memory allocation error (%lu)\n", sizeof(*sound));
	memset(sound, 0, sizeof(*sound));
	if (decode_audio_file(path, &sound->data, &sound->length, AV_CODEC_ID_VORBIS))
		return sound;
	display_warning("An error occured when reading file %s\n", path);
	free(sound);
	return NULL;
}

void	destroySound(Sound *sound)
{
	if (sound)
		free(sound->data);
	free(sound);
}
//
// Created by Gegel85 on 17/05/2020.
//

#include "libav.hpp"
#include "Sound.hpp"
#include "Exceptions.hpp"

namespace OsuReplayPlayer
{
	Sound::Sound(const std::string &path, int64_t sample_rate)
	{
		this->_buffer.loadFromFile(path);

		// initialize all muxers, demuxers and protocols for libavformat
		// (does nothing if called twice during the course of one program execution)
		av_register_all();

		// get format from audio file
		AVFormatContext *format = avformat_alloc_context();
		int err = avformat_open_input(&format, path.c_str(), nullptr, nullptr);

		if (err != 0)
			throw AvErrorException("Could not open file '" + path + "'", err);

		err = avformat_find_stream_info(format, nullptr);
		if (err < 0)
			throw AvErrorException("Could not retrieve stream info from file '" + path + "'", err);
		this->_sampleRate = sample_rate;

		// Find the index of the first audio stream
		for (size_t stream_index = 0; stream_index < format->nb_streams; stream_index++) {
			if (format->streams[stream_index]->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
				this->_nbChannels++;

				std::vector<short> buff;
				AVStream *stream = format->streams[stream_index];

				// find & open codec
				AVCodecContext *codec = stream->codec;
				err = avcodec_open2(codec, avcodec_find_decoder(codec->codec_id), nullptr);
				if (err < 0)
					throw AvErrorException("Failed to open decoder for stream " + std::to_string(stream_index) + " in file '" + path + "'", err);

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
				if (!swr_is_initialized(swr))
					throw AvErrorException("Resampler has not been properly initialized");

				// prepare to read data
				AVPacket packet;
				av_init_packet(&packet);
				AVFrame* frame = av_frame_alloc();
				if (!frame)
					throw AvErrorException("Error allocating the frame");

				// iterate through frames
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
					auto oldSize = buff.size();
					buff.resize(buff.size() + frame->nb_samples);
					for (auto i = 0; i < frame->nb_samples; i++)
						buff[oldSize + i] = buffer[i];
				}

				// clean up
				av_frame_free(&frame);
				swr_free(&swr);
				avcodec_close(codec);
			}
		}
		if (!this->_nbChannels)
			NoAudioStreamException("Could not retrieve audio stream from file '" + path + "'");

		avformat_free_context(format);
	}

	unsigned int Sound::getNbChannels() const
	{
		return this->_nbChannels;
	}

	int64_t OsuReplayPlayer::Sound::getSampleRate() const
	{
		return this->_sampleRate;
	}

	std::vector<short> Sound::operator[](unsigned int index) const
	{
		return this->_data[index];
	}

	const sf::SoundBuffer &OsuReplayPlayer::Sound::getBuffer() const
	{
		return this->_buffer;
	}
}
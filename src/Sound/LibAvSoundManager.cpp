//
// Created by Gegel85 on 16/06/2020.
//

#include <sstream>
#include "LibAvSoundManager.hpp"
#include "../Exceptions.hpp"

namespace OsuReplayPlayer
{
	LibAvSoundManager::LibAvSoundManager(const std::string &path)
	{
		this->_initCodec();
		this->_initFrame();

		this->_packet = av_packet_alloc();
		if (!this->_packet)
			throw AvErrorException("Memory allocation error");

		this->_stream = fopen(path.c_str(), "wb");
		if (!this->_stream)
			throw SystemCallFailedException("fopen(" + path + "): " + strerror(errno));
	}

	LibAvSoundManager::~LibAvSoundManager()
	{
		this->_flush(true);
		this->_flush(false);

		/* free frames */
		av_frame_free(&this->_frame);

		/* free packets */
		av_packet_free(&this->_packet);

		/* free codec contexts */
		avcodec_free_context(&this->_codecContext);

		fclose(this->_stream);
	}

	void LibAvSoundManager::setVolume(float volume)
	{
		this->_volume = volume;
	}

	unsigned int LibAvSoundManager::playSound(const Sound &sound, double pitch)
	{
		unsigned index = 0;

		while (index < this->_sounds.size()) {
			auto &s = this->_sounds[index];

			if (s.sound.get().getLength() <= s.pos)
				break;
			index++;
		}

		if (index == this->_sounds.size())
			this->_sounds.push_back(PlayingSound{std::reference_wrapper<const Sound>(sound), pitch, 0});
		else
			this->_sounds[index] = PlayingSound{std::reference_wrapper<const Sound>(sound), pitch, 0};
		return index;
	}

	void LibAvSoundManager::stopSound(unsigned int id)
	{
		auto &sound = this->_sounds[id];

		sound.pos = sound.sound.get().getLength();
	}

	void LibAvSoundManager::tick(float time)
	{
		int ret;
		auto *buffer = reinterpret_cast<short *>(this->_frame->data[0]);

		this->_totalTime += time;
		for (; this->_i < SAMPLE_RATE * this->_totalTime; this->_i++) {
			for (auto &sound : this->_sounds) {
				if (sound.sound.get()[0].size() > sound.pos) {
					buffer[this->_index] += sound.sound.get()[0][sound.pos] / 2.;
					sound.pos++;
				}
			}
			this->_index++;
			if (this->_index >= this->_codecContext->frame_size) {
				if ((ret = av_frame_make_writable(this->_frame)) < 0)
					throw AvErrorException("Frame is not writable", ret);
				this->_flush(true);
				this->_index = 0;
				memset(buffer, 0, this->_codecContext->frame_size * sizeof(*buffer));
			}
		}
	}

	void LibAvSoundManager::_initCodec()
	{
		const AVCodec	*codec;
		AVCodecContext	*codecContext;

		/* register all the codecs */
		avcodec_register_all();

		/* find the MP2 encoder */
		codec = avcodec_find_encoder(AV_CODEC_ID_MP2);
		if (!codec)
			throw AvErrorException("MP2 codec not found");
		codecContext = avcodec_alloc_context3(codec);

		/* put sample parameters */
		codecContext->bit_rate = 64000;
		codecContext->sample_fmt = AV_SAMPLE_FMT_S16;

		/* select other audio parameters supported by the encoder */
		codecContext->sample_rate = SAMPLE_RATE;
		codecContext->channel_layout = AV_CH_LAYOUT_MONO;
		codecContext->channels = 1;

		/* open it */
		int err = avcodec_open2(codecContext, codec, nullptr);
		if (err < 0)
			throw AvErrorException("Could not open codec", err);
		this->_codecContext = codecContext;
	}

	void LibAvSoundManager::_initFrame()
	{
		int ret;
		AVFrame	*frame;

		/* frame containing input raw audio */
		frame = av_frame_alloc();
		if (!frame)
			throw AvErrorException("Could not allocate audio frame");
		frame->nb_samples = this->_codecContext->frame_size;
		frame->format = this->_codecContext->sample_fmt;
		frame->channel_layout = this->_codecContext->channel_layout;

		/* allocate the data buffers */
		if ((ret = av_frame_get_buffer(frame, 0)) < 0)
			throw AvErrorException("Could not allocate audio data buffers", ret);

		this->_frame = frame;
		memset(this->_frame->data[0], 0, this->_codecContext->frame_size * sizeof(short));
	}

	void LibAvSoundManager::_flush(bool sendFrame)
	{
		int ret;

		/* send the frame for encoding */
		ret = avcodec_send_frame(this->_codecContext, sendFrame ? this->_frame : nullptr);
		if (ret < 0)
			throw AvErrorException("Error sending the frame to the encoder", ret);

		/* read all the available output packets (in general there may be any
		 * number of them */
		while (ret >= 0) {
			ret = avcodec_receive_packet(this->_codecContext, this->_packet);
			if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
				return;
			else if (ret < 0)
				throw AvErrorException("Error encoding audio frame", ret);

			size_t total = fwrite(this->_packet->data, 1, this->_packet->size, this->_stream);

			if (total != static_cast<size_t>(this->_packet->size)) {
				std::stringstream stream;

				stream << "fwrite(" << std::hex << std::showbase << this->_packet->data << ", 1, " << std::dec << this->_packet->size << ", " << std::hex << this->_stream << ")";
				stream << " wrote " << std::dec << total << " bytes instead of " << this->_packet->size << std::endl;
				throw SystemCallFailedException(stream.str());
			}
			av_packet_unref(this->_packet);
		}
	}
}
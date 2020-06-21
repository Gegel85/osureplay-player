//
// Created by Gegel85 on 21/05/2020.
//

#include <cmath>
#include <cassert>
#include "LibAvRendererSound.hpp"
#include "Exceptions.hpp"

namespace OsuReplayPlayer
{
	LibAvRendererSound::LibAvRendererSound(const std::string &path, const VideoConfig &vidConf, const AudioConfig &audioConf) :
		_size(vidConf.resolution)
	{
		int ret;

		//Create format context
		avformat_alloc_output_context2(&this->_fmtContext, nullptr, nullptr, path.c_str());

		if (!this->_fmtContext)
			throw AvErrorException("Cannot open format corresponding to this file or cannot deduce format from file name.");

		this->_initVideoPart(vidConf);
		this->_initAudioPart(audioConf);

		if (!(this->_fmtContext->oformat->flags & AVFMT_NOFILE) && (ret = avio_open(&this->_fmtContext->pb, path.c_str(), AVIO_FLAG_WRITE)) < 0)
			throw AvErrorException("Cannot open file '" + path + "'", ret);

		/* Write file header */
		if ((ret = avformat_write_header(this->_fmtContext, nullptr)) < 0)
			throw AvErrorException("Cannot write header", ret);

		av_dump_format(this->_fmtContext, 0, path.c_str(), 1);
	}

	LibAvRendererSound::~LibAvRendererSound()
	{
		if (!this->_fmtContext)
			return;

		delete[] this->_buffer;
		delete[] this->_pixelArray;

		this->_flushVideo(false);
		this->_flushAudio(false);

		/* Write file trailer */
		av_write_trailer(this->_fmtContext);

		/* Free frames */
		av_frame_free(&this->_videoStream.frame);

		/* Free packets */
		av_packet_free(&this->_videoStream.packet);

		/* Free codec */
		avcodec_free_context(&this->_videoStream.enc);

		if (!(this->_fmtContext->oformat->flags & AVFMT_NOFILE))
			avio_closep(&this->_fmtContext->pb);

		avformat_free_context(this->_fmtContext);
	}

	sf::Vector2u LibAvRendererSound::getSize() const
	{
		return this->_size;
	}

	void LibAvRendererSound::clear(sf::Color color)
	{
		color.a = 255;
		for (unsigned x = 0; x < this->_size.x; x++)
			for (unsigned y = 0; y < this->_size.y; y++)
				this->_buffer[x + y * this->_size.x] = color;
	}

	void LibAvRendererSound::drawPixel(sf::Vector2i pos, sf::Color color)
	{
		pos.x += this->_padding.x;
		pos.y += this->_padding.y;

		if (pos.x < 0 || static_cast<unsigned>(pos.x) >= this->_size.x)
			return;
		if (pos.y < 0 || static_cast<unsigned>(pos.y) >= this->_size.y)
			return;
		if (this->_pixelArray[pos.y][pos.x].a != 255)
			return;

		double a = color.a / 255.;

		this->_pixelArray[pos.y][pos.x].r += (color.r - this->_pixelArray[pos.y][pos.x].r) * a;
		this->_pixelArray[pos.y][pos.x].g += (color.g - this->_pixelArray[pos.y][pos.x].g) * a;
		this->_pixelArray[pos.y][pos.x].b += (color.b - this->_pixelArray[pos.y][pos.x].b) * a;
	}

	void LibAvRendererSound::drawPoint(sf::Vector2f pos, sf::Color color)
	{
		this->drawPixel({static_cast<int>(pos.x), static_cast<int>(pos.y)}, color);
		if (std::floor(pos.x) != pos.x)
			this->drawPixel({static_cast<int>(pos.x + 1), static_cast<int>(pos.y)}, color);
		if (std::floor(pos.y) != pos.y)
			this->drawPixel({static_cast<int>(pos.x), static_cast<int>(pos.y + 1)}, color);
		if (std::floor(pos.x) != pos.x && std::floor(pos.y) != pos.y)
			this->drawPixel({static_cast<int>(pos.x + 1), static_cast<int>(pos.y + 1)}, color);
	}

	void LibAvRendererSound::drawRectangle(sf::Vector2i pos, sf::Vector2u size, unsigned, sf::Color color)
	{
		for (unsigned x = 0; x < size.x; x++)
			this->drawPoint({static_cast<float>(x) + pos.x, static_cast<float>(pos.y)}, color);
		for (unsigned x = 0; x < size.x; x++)
			this->drawPoint({static_cast<float>(x) + pos.x, static_cast<float>(pos.y) + size.y - 1}, color);
		for (unsigned y = 0; y < size.x; y++)
			this->drawPoint({static_cast<float>(pos.x), static_cast<float>(pos.y) + y}, color);
		for (unsigned y = 0; y < size.x; y++)
			this->drawPoint({static_cast<float>(pos.x) + size.x - 1, static_cast<float>(pos.y) + y}, color);
	}

	void LibAvRendererSound::drawFilledRectangle(sf::Vector2i pos, sf::Vector2u size, sf::Color color)
	{
		for (unsigned x = 0; x < size.x; x++)
			for (unsigned y = 0; y < size.y; y++)
				this->drawPoint({static_cast<float>(x) + pos.x, static_cast<float>(y) + pos.y}, color);
	}

	void LibAvRendererSound::drawImage(sf::Vector2i pos, const sf::Image &image, sf::Vector2i newSize, sf::Color tint, bool centered, float rotation)
	{
		sf::Vector2u size = image.getSize();

		if (!size.x || !size.y)
			return;

		if (rotation != 0)
			rotation = rotation * 2 / 2;

		const sf::Color *array = reinterpret_cast<const sf::Color *>(image.getPixelsPtr());
		sf::Vector2f scale = {
			newSize.x < 0 ? 1 : (static_cast<float>(newSize.x) / size.x),
			newSize.y < 0 ? 1 : (static_cast<float>(newSize.y) / size.y)
		};
		double c;
		double s;
		sf::Color col;

		rotation = rotation * M_PI / 180;
		c = cos(rotation);
		s = sin(rotation);
		if (centered) {
			pos.x -= (size.x * scale.x) / 2;
			pos.y -= (size.y * scale.y) / 2;
		}

		for (unsigned x = 0; x < size.x * scale.x; x++)
			for (unsigned y = 0; y < size.y * scale.y; y++) {
				if (static_cast<unsigned>(y / scale.y) >= size.y || static_cast<unsigned>(x / scale.x) >= size.x)
					col = {0, 0, 0, 0};
				else
					col = {
						static_cast<sf::Uint8>(array[static_cast<int>(y / scale.y) * size.x + static_cast<int>(x / scale.x)].r * (tint.r / 255.)),
						static_cast<sf::Uint8>(array[static_cast<int>(y / scale.y) * size.x + static_cast<int>(x / scale.x)].g * (tint.g / 255.)),
						static_cast<sf::Uint8>(array[static_cast<int>(y / scale.y) * size.x + static_cast<int>(x / scale.x)].b * (tint.b / 255.)),
						static_cast<sf::Uint8>(array[static_cast<int>(y / scale.y) * size.x + static_cast<int>(x / scale.x)].a * (tint.a / 255.)),
					};
				if (rotation == 0)
					this->drawPoint({
						static_cast<float>(pos.x) + x,
						static_cast<float>(pos.y) + y
					}, col);
				else
					this->drawPoint({
						static_cast<float>(c * (x - newSize.x / 2.) - s * (y - newSize.y / 2.) + newSize.x / 2. + pos.x),
						static_cast<float>(s * (x - newSize.x / 2.) + c * (y - newSize.y / 2.) + newSize.y / 2. + pos.y)
					}, col);
			}
	}

	void LibAvRendererSound::drawCircle(unsigned thickness, sf::Vector2i pos, float radius, sf::Color color)
	{
		double	distance = 0;

		for (int x = -thickness; x < (radius * 2 + thickness); x++)
			for (int y = -thickness; y < (radius * 2 + thickness); y++) {
				distance = sqrt(pow(x - radius, 2) + pow(y - radius, 2));
				if (distance >= radius && distance <= radius + thickness) {
					this->drawPoint({static_cast<float>(x + pos.x), static_cast<float>(y + pos.y)}, color);
				}

			}
	}

	void LibAvRendererSound::drawFilledCircle(sf::Vector2i pos, float radius, sf::Color color)
	{
		for (int x = 0; x < radius * 2; x++)
			for (int y = 0; y < radius * 2; y++)
				if (sqrt(pow(radius - x, 2) + pow(radius - y, 2)) <= radius)
					this->drawPoint({static_cast<float>(x + pos.x), static_cast<float>(y + pos.y)}, color);
	}

	void LibAvRendererSound::renderFrame()
	{
		this->_prepareVideoFrame();
		this->_videoStream.frame->pts = this->_videoStream.nextPts++;
		this->_flushVideo(true);
	}

	void LibAvRendererSound::setGlobalPadding(sf::Vector2i padding)
	{
		this->_padding = padding;
	}

	void LibAvRendererSound::_prepareVideoFrame()
	{
		int ret = 0;

		/* make sure the frame data is writable */
		if ((ret = av_frame_make_writable(this->_videoStream.frame)) < 0)
			throw AvErrorException("The frame data is not writable", ret);

		/* prepare the frame */
		/* Y */
		for (unsigned y = 0; y < this->_size.y; y++) {
			for (unsigned x = 0; x < this->_size.x; x++) {
				this->_videoStream.frame->data[0][y * this->_videoStream.frame->linesize[0] + x] =
					0.299 * this->_pixelArray[y][x].r +
					0.587 * this->_pixelArray[y][x].g +
					0.114 * this->_pixelArray[y][x].b;
			}
		}

		/* Cb Cr */
		for (unsigned y = 0; y < this->_size.y / 2; y++) {
			for (unsigned x = 0; x < this->_size.x / 2; x++) {
				this->_videoStream.frame->data[1][y * this->_videoStream.frame->linesize[1] + x] =
					-0.1687 * this->_pixelArray[y * 2][x * 2].r +
					-0.3313 * this->_pixelArray[y * 2][x * 2].g +
					0.5 *     this->_pixelArray[y * 2][x * 2].b + 128;
				this->_videoStream.frame->data[2][y * this->_videoStream.frame->linesize[2] + x] =
					0.5 *     this->_pixelArray[y * 2][x * 2].r +
					-0.4187 * this->_pixelArray[y * 2][x * 2].g +
					-0.0813 * this->_pixelArray[y * 2][x * 2].b + 128;
			}
		}
	}

	void LibAvRendererSound::_initVideoStream(sf::Vector2u size, unsigned fps, size_t bitRate, const std::map<std::string, std::string> &opts)
	{
		AVCodec *codec;
		AVCodecContext *c;
		int ret;

		/* find the encoder */
		codec = avcodec_find_encoder(this->_fmtContext->oformat->video_codec);
		if (!codec)
			throw AvErrorException("Could not find encoder for '" + std::string(avcodec_get_name(this->_fmtContext->oformat->video_codec)) + "'");

		this->_videoStream.stream = avformat_new_stream(this->_fmtContext, nullptr);
		if (!this->_videoStream.stream)
			throw AvErrorException( "Could not allocate stream");

		this->_videoStream.stream->id = this->_fmtContext->nb_streams - 1;
		this->_videoStream.enc = c = avcodec_alloc_context3(codec);

		if (!c)
			throw AvErrorException("Could not alloc an encoding context");

		c->codec_id = this->_fmtContext->oformat->video_codec;
		c->bit_rate = bitRate;

		/* Resolution must be a multiple of two. */
		c->width    = size.x;
		c->height   = size.y;

		/* timebase: This is the fundamental unit of time (in seconds) in terms
		 * of which frame timestamps are represented. For fixed-fps content,
		 * timebase should be 1/framerate and timestamp increments should be
		 * identical to 1. */
		this->_videoStream.stream->time_base = { 1, static_cast<int>(fps) };
		c->time_base     = this->_videoStream.stream->time_base;
		c->gop_size      = 12; /* emit one intra frame every twelve frames at most */
		c->pix_fmt       = AV_PIX_FMT_YUV420P;

		if (c->codec_id == AV_CODEC_ID_MPEG1VIDEO) {
			/* Needed to avoid using macroblocks in which some coeffs overflow.
			 * This does not happen with normal video, it just happens here as
			 * the motion of the chroma plane does not match the luma plane. */
			c->mb_decision = 2;
		}

		/* Some formats want stream headers to be separate. */
		if (this->_fmtContext->oformat->flags & AVFMT_GLOBALHEADER)
			c->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

		AVDictionary *opt = nullptr;

		for (auto &pair : opts)
			av_dict_set(&opt, pair.first.c_str(), pair.second.c_str(), 0);

		/* open the codec */
		ret = avcodec_open2(c, codec, &opt);
		av_dict_free(&opt);

		if (ret < 0)
			throw AvErrorException("Could not open video codec", ret);

		/* copy the stream parameters to the muxer */
		ret = avcodec_parameters_from_context(this->_videoStream.stream->codecpar, c);
		if (ret < 0)
			throw AvErrorException("Could not copy the stream parameters", ret);
	}

	void LibAvRendererSound::_initVideoFrame()
	{
		this->_videoStream.frame = av_frame_alloc();

		if (!this->_videoStream.frame)
			throw AvErrorException("Could not allocate video frame");

		this->_videoStream.frame->format = this->_videoStream.enc->pix_fmt;
		this->_videoStream.frame->width = this->_videoStream.enc->width;
		this->_videoStream.frame->height = this->_videoStream.enc->height;
		this->_videoStream.frame->quality = 1;

		int err = av_frame_get_buffer(this->_videoStream.frame, 32);

		if (err < 0)
			throw AvErrorException("Could not allocate video data buffers", err);
	}

	void LibAvRendererSound::_flushVideo(bool sendFrame)
	{
		/* send the frame to the encoder */
		int ret = avcodec_send_frame(this->_videoStream.enc, sendFrame ? this->_videoStream.frame : nullptr);

		if (ret < 0)
			throw AvErrorException("Error sending a video frame for encoding", ret);

		while (ret >= 0) {
			ret = avcodec_receive_packet(this->_videoStream.enc, this->_videoStream.packet);
			if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
				return;
			else if (ret < 0)
				throw AvErrorException("Error during video encoding", ret);

			av_packet_rescale_ts(this->_videoStream.packet, this->_videoStream.enc->time_base, this->_videoStream.stream->time_base);
			this->_videoStream.packet->stream_index = this->_videoStream.stream->index;
			if ((ret = av_interleaved_write_frame(this->_fmtContext, this->_videoStream.packet)) < 0)
			//if ((ret = av_write_frame(this->_fmtContext, this->_packet)) < 0)
				throw AvErrorException("Cannot write video data in file", ret);
		}
	}

	void LibAvRendererSound::_initVideoPart(const VideoConfig &vidConf)
	{
		this->_initVideoStream(vidConf.resolution, vidConf.frameRate, vidConf.bitRate, vidConf.opts);
		this->_initVideoFrame();

		this->_videoStream.packet = av_packet_alloc();
		if (!this->_videoStream.packet)
			throw AvErrorException("Memory allocation error");

		this->_buffer = new sf::Color[vidConf.resolution.x * vidConf.resolution.y];
		this->_pixelArray = new sf::Color *[vidConf.resolution.y];
		for (unsigned y = 0; y < vidConf.resolution.y; y++)
			this->_pixelArray[y] = &this->_buffer[vidConf.resolution.x * y];
		LibAvRendererSound::clear(sf::Color::Black);
	}





	void LibAvRendererSound::setVolume(float volume)
	{
		this->_volume = volume;
	}

	unsigned int LibAvRendererSound::playSound(const Sound &sound, double pitch)
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

	void LibAvRendererSound::stopSound(unsigned int id)
	{
		auto &sound = this->_sounds[id];

		sound.pos = sound.sound.get().getLength();
	}

	void LibAvRendererSound::tick(unsigned currentFrame, unsigned framePerSeconds)
	{
		int ret;
		auto *buffer = reinterpret_cast<short *>(this->_audioStream.tmpFrame->data[0]);

		for (; this->_i < currentFrame * SAMPLE_RATE / framePerSeconds; this->_i++) {
			for (auto &sound : this->_sounds) {
				if (sound.sound.get()[0].size() > sound.pos) {
					int diff = sound.sound.get()[0][sound.pos] * this->_volume / 2.;

					buffer[this->_index] = std::max(INT16_MIN, std::min(INT16_MAX, buffer[this->_index] + diff));
					sound.pos += sound.pitch;
				}
			}
			this->_index++;
			if (this->_index >= this->_audioStream.enc->frame_size) {
				if ((ret = av_frame_make_writable(this->_audioStream.frame)) < 0)
					throw AvErrorException("Frame is not writable", ret);
				this->_audioStream.frame->pts = this->_audioStream.nextPts++;
				this->_flushAudio(true);
				this->_index = 0;
				memset(buffer, 0, this->_audioStream.enc->frame_size * sizeof(*buffer));
			}
		}
	}

	void LibAvRendererSound::_initAudioPart(const AudioConfig &audioConf)
	{
		this->_initAudioStream(audioConf.bitRate, audioConf.sampleRate, audioConf.opts);
		this->_initAudioFrame(audioConf.sampleRate);

		this->_audioStream.packet = av_packet_alloc();
		if (!this->_audioStream.packet)
			throw AvErrorException("Memory allocation error");
	}

	void LibAvRendererSound::_initAudioStream(size_t bitRate, size_t sampleRate, const std::map<std::string, std::string> &opts)
	{
		AVCodec *codec;
		AVCodecContext *c;
		int ret;

		/* find the encoder */
		codec = avcodec_find_encoder(this->_fmtContext->oformat->audio_codec);
		if (!codec)
			throw AvErrorException("Could not find encoder for '" + std::string(avcodec_get_name(this->_fmtContext->oformat->audio_codec)) + "'");

		this->_audioStream.stream = avformat_new_stream(this->_fmtContext, nullptr);
		if (!this->_audioStream.stream)
			throw AvErrorException( "Could not allocate stream");

		this->_audioStream.stream->id = this->_fmtContext->nb_streams - 1;
		this->_audioStream.enc = c = avcodec_alloc_context3(codec);

		if (!c)
			throw AvErrorException("Could not alloc an encoding context");

		c->sample_fmt  = codec->sample_fmts ? codec->sample_fmts[0] : AV_SAMPLE_FMT_FLTP;
		c->bit_rate    = bitRate;
		c->sample_rate = sampleRate;
		if (codec->supported_samplerates) {
			c->sample_rate = codec->supported_samplerates[0];
			for (int i = 0; codec->supported_samplerates[i]; i++) {
				if (static_cast<size_t>(codec->supported_samplerates[i]) == sampleRate)
					c->sample_rate = sampleRate;
			}
		}

		c->channel_layout = AV_CH_LAYOUT_MONO;
		if (codec->channel_layouts) {
			c->channel_layout = codec->channel_layouts[0];
			for (int i = 0; codec->channel_layouts[i]; i++) {
				if (codec->channel_layouts[i] == AV_CH_LAYOUT_MONO)
					c->channel_layout = AV_CH_LAYOUT_MONO;
			}
		}
		c->channels  = av_get_channel_layout_nb_channels(c->channel_layout);
		c->time_base = AVRational{ 1, c->sample_rate };
		this->_audioStream.stream->time_base = c->time_base;

		/* open it */
		AVDictionary *opt = nullptr;

		for (auto &pair : opts)
			av_dict_set(&opt, pair.first.c_str(), pair.second.c_str(), 0);

		/* open the codec */
		ret = avcodec_open2(c, codec, &opt);
		av_dict_free(&opt);
		if (ret < 0)
			throw AvErrorException("Could not open audio codec", ret);

		/* copy the stream parameters to the muxer */
		ret = avcodec_parameters_from_context(this->_audioStream.stream->codecpar, c);
		if (ret < 0)
			throw AvErrorException("Could not copy the stream parameters", ret);

		/* create resampler context */
		this->_audioStream.swrCtx = swr_alloc();
		if (!this->_audioStream.swrCtx)
			throw AvErrorException("Could not allocate resampler context");

		/* set options */
		av_opt_set_int       (this->_audioStream.swrCtx, "in_channel_count",   c->channels,       0);
		av_opt_set_int       (this->_audioStream.swrCtx, "in_sample_rate",     c->sample_rate,    0);
		av_opt_set_sample_fmt(this->_audioStream.swrCtx, "in_sample_fmt",      AV_SAMPLE_FMT_S16, 0);
		av_opt_set_int       (this->_audioStream.swrCtx, "out_channel_count",  c->channels,       0);
		av_opt_set_int       (this->_audioStream.swrCtx, "out_sample_rate",    c->sample_rate,    0);
		av_opt_set_sample_fmt(this->_audioStream.swrCtx, "out_sample_fmt",     c->sample_fmt,     0);

		/* initialize the resampling context */
		if ((ret = swr_init(this->_audioStream.swrCtx)) < 0)
			throw AvErrorException("Failed to initialize the resampling context", ret);
	}

	void LibAvRendererSound::_initAudioFrame(size_t sampleRate)
	{
		int ret;
		AVFrame	*frame;

		/* frame containing input raw audio */
		frame = av_frame_alloc();
		if (!frame)
			throw AvErrorException("Could not allocate audio frame");
		frame->nb_samples = this->_audioStream.enc->frame_size;
		frame->format = this->_audioStream.enc->sample_fmt;
		frame->channel_layout = this->_audioStream.enc->channel_layout;
		frame->sample_rate = sampleRate;

		/* allocate the data buffers */
		if ((ret = av_frame_get_buffer(frame, 0)) < 0)
			throw AvErrorException("Could not allocate audio data buffers", ret);
		this->_audioStream.frame = frame;



		/* frame containing input raw audio */
		frame = av_frame_alloc();
		if (!frame)
			throw AvErrorException("Could not allocate audio frame");
		frame->nb_samples = this->_audioStream.enc->frame_size;
		frame->format = AV_SAMPLE_FMT_S16;
		frame->channel_layout = this->_audioStream.enc->channel_layout;
		frame->sample_rate = sampleRate;

		/* allocate the data buffers */
		if ((ret = av_frame_get_buffer(frame, 0)) < 0)
			throw AvErrorException("Could not allocate audio data buffers", ret);
		this->_audioStream.tmpFrame = frame;
		memset(this->_audioStream.tmpFrame->data[0], 0, this->_audioStream.enc->frame_size * sizeof(short));
	}

	void LibAvRendererSound::_flushAudio(bool sendFrame)
	{
		int ret;

		if (sendFrame) {
			/* convert samples from native format to destination codec format, using the resampler */
			/* compute destination number of samples */
			int dst_nb_samples = av_rescale_rnd(
				swr_get_delay(this->_audioStream.swrCtx, this->_audioStream.enc->sample_rate) +
				this->_audioStream.frame->nb_samples,
				this->_audioStream.enc->sample_rate,
				this->_audioStream.enc->sample_rate,
				AV_ROUND_UP
			);
			assert(dst_nb_samples == this->_audioStream.frame->nb_samples);

			/* when we pass a frame to the encoder, it may keep a reference to it
			 * internally;
			 * make sure we do not overwrite it here
			 */
			ret = av_frame_make_writable(this->_audioStream.frame);
			if (ret < 0)
				throw AvErrorException("Audio frame is not writable", ret);

			/* convert to destination format */
			ret = swr_convert(
				this->_audioStream.swrCtx,
				this->_audioStream.frame->data,
				dst_nb_samples,
				const_cast<const uint8_t **>(this->_audioStream.tmpFrame->data),
				this->_audioStream.tmpFrame->nb_samples
			);
			if (ret < 0)
				throw AvErrorException("Error while converting", ret);
			this->_audioStream.frame->pts = av_rescale_q(
				this->_audioStream.sampleCount,
				AVRational{1, this->_audioStream.enc->sample_rate},
				this->_audioStream.enc->time_base
			);
			this->_audioStream.sampleCount += dst_nb_samples;
		}

		/* send the frame to the encoder */
		ret = avcodec_send_frame(this->_audioStream.enc, sendFrame ? this->_audioStream.frame : nullptr);
		if (ret < 0)
			throw AvErrorException("Error sending an audio frame for encoding", ret);

		while (ret >= 0) {
			ret = avcodec_receive_packet(this->_audioStream.enc, this->_audioStream.packet);
			if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
				return;
			else if (ret < 0)
				throw AvErrorException("Error during audio encoding", ret);

			av_packet_rescale_ts(this->_audioStream.packet, this->_audioStream.enc->time_base, this->_audioStream.stream->time_base);
			this->_audioStream.packet->stream_index = this->_audioStream.stream->index;
			if ((ret = av_interleaved_write_frame(this->_fmtContext, this->_audioStream.packet)) < 0)
				//if ((ret = av_write_frame(this->_fmtContext, this->_packet)) < 0)
				throw AvErrorException("Cannot write audio data in file", ret);
		}
	}
}

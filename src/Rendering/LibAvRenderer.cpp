//
// Created by Gegel85 on 21/05/2020.
//

#include <cmath>
#include <sstream>
#include "LibAvRenderer.hpp"
#include "../Exceptions.hpp"

namespace OsuReplayPlayer
{
	LibAvRenderer::LibAvRenderer(const std::string &path, sf::Vector2u size, unsigned fps, size_t bitRate) :
		_size(size)
	{
		int ret;

		//Create format context
		avformat_alloc_output_context2(&this->_fmtContext, nullptr, nullptr, path.c_str());

		if (!this->_fmtContext)
			throw AvErrorException("Cannot open format corresponding to this file or cannot deduce format from file name.");

		this->_initStream(size, fps, bitRate);
		this->_initFrame();

		this->_packet = av_packet_alloc();
		if (!this->_packet)
			throw AvErrorException("Memory allocation error");

		if (!(this->_fmtContext->oformat->flags & AVFMT_NOFILE) && (ret = avio_open(&this->_fmtContext->pb, path.c_str(), AVIO_FLAG_WRITE)) < 0)
			throw AvErrorException("Cannot open file '" + path + "'", ret);

		/* Write file header */
		if ((ret = avformat_write_header(this->_fmtContext, nullptr)) < 0)
			throw AvErrorException("Cannot write header", ret);

		av_dump_format(this->_fmtContext, 0, path.c_str(), 1);

		this->_buffer = new sf::Color[size.x * size.y];
		this->_pixelArray = new sf::Color *[size.y];
		for (unsigned y = 0; y < size.y; y++)
			this->_pixelArray[y] = &this->_buffer[size.x * y];
		LibAvRenderer::clear(sf::Color::Black);
	}

	LibAvRenderer::~LibAvRenderer()
	{
		if (!this->_fmtContext)
			return;

		delete[] this->_buffer;
		delete[] this->_pixelArray;

		this->_flush(false);

		/* Write file trailer */
		av_write_trailer(this->_fmtContext);

		/* Free frames */
		av_frame_free(&this->_frame);

		/* Free packets */
		av_packet_free(&this->_packet);

		/* Free codec contexts */
		avcodec_close(this->_stream->codec);

		if (!(this->_fmtContext->oformat->flags & AVFMT_NOFILE))
			avio_close(this->_fmtContext->pb);

		av_free(this->_fmtContext);
	}

	sf::Vector2u LibAvRenderer::getSize() const
	{
		return this->_size;
	}

	void LibAvRenderer::clear(sf::Color color)
	{
		color.a = 255;
		for (unsigned x = 0; x < this->_size.x; x++)
			for (unsigned y = 0; y < this->_size.y; y++)
				this->_buffer[x + y * this->_size.x] = color;
	}

	void LibAvRenderer::drawPixel(sf::Vector2i pos, sf::Color color)
	{
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

	void LibAvRenderer::drawPoint(sf::Vector2f pos, sf::Color color)
	{
		this->drawPixel({static_cast<int>(pos.x), static_cast<int>(pos.y)}, color);
		if (std::floor(pos.x) != pos.x)
			this->drawPixel({static_cast<int>(pos.x + 1), static_cast<int>(pos.y)}, color);
		if (std::floor(pos.y) != pos.y)
			this->drawPixel({static_cast<int>(pos.x), static_cast<int>(pos.y + 1)}, color);
		if (std::floor(pos.x) != pos.x && std::floor(pos.y) != pos.y)
			this->drawPixel({static_cast<int>(pos.x + 1), static_cast<int>(pos.y + 1)}, color);
	}

	void LibAvRenderer::drawRectangle(sf::Vector2i pos, sf::Vector2u size, unsigned, sf::Color color)
	{
		for (unsigned x = 0; x < size.x; x++)
			this->drawPoint({static_cast<float>(x + pos.x), static_cast<float>(pos.y)}, color);
		for (unsigned x = 0; x < size.x; x++)
			this->drawPoint({static_cast<float>(x + pos.x), static_cast<float>(pos.y + size.y - 1)}, color);
		for (unsigned y = 0; y < size.x; y++)
			this->drawPoint({static_cast<float>(pos.x), static_cast<float>(pos.y + y)}, color);
		for (unsigned y = 0; y < size.x; y++)
			this->drawPoint({static_cast<float>(pos.x + size.x - 1), static_cast<float>(pos.y + y)}, color);
	}

	void LibAvRenderer::drawFilledRectangle(sf::Vector2i pos, sf::Vector2u size, sf::Color color)
	{
		for (unsigned x = 0; x < size.x; x++)
			for (unsigned y = 0; y < size.y; y++)
				this->drawPoint({static_cast<float>(x + pos.x), static_cast<float>(y + pos.y)}, color);
	}

	void LibAvRenderer::drawImage(sf::Vector2i pos, const sf::Image &image, sf::Vector2i newSize, sf::Color tint, bool centered, float rotation)
	{
		const sf::Color *array = reinterpret_cast<const sf::Color *>(image.getPixelsPtr());
		sf::Vector2u size = image.getSize();
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
						static_cast<float>(pos.x + x),
						static_cast<float>(pos.y + y)
					}, col);
				else
					this->drawPoint({
						static_cast<float>(c * (x - newSize.x / 2.) - s * (y - newSize.y / 2.) + newSize.x / 2. + pos.x),
						static_cast<float>(s * (x - newSize.x / 2.) + c * (y - newSize.y / 2.) + newSize.y / 2. + pos.y)
					}, col);
			}
	}

	void LibAvRenderer::drawCircle(unsigned thickness, sf::Vector2i pos, float radius, sf::Color color)
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

	void LibAvRenderer::drawFilledCircle(sf::Vector2i pos, float radius, sf::Color color)
	{
		for (int x = 0; x < radius * 2; x++)
			for (int y = 0; y < radius * 2; y++)
				if (sqrt(pow(radius - x, 2) + pow(radius - y, 2)) <= radius)
					this->drawPoint({static_cast<float>(x + pos.x), static_cast<float>(y + pos.y)}, color);
	}

	void LibAvRenderer::renderFrame()
	{
		this->_prepareFrame();
		this->_flush(true);
	}

	void LibAvRenderer::_prepareFrame()
	{
		/* make sure the frame data is writable */
		if (av_frame_make_writable(this->_frame) < 0)
			throw InvalidStateException("The frame data is not writable\n");

		/* prepare the frame */
		/* Y */
		for (int y = 0; y < this->_stream->codec->height; y++) {
			for (int x = 0; x < this->_stream->codec->width; x++) {
				this->_frame->data[0][y * this->_frame->linesize[0] + x] =
					0.299 * this->_pixelArray[y][x].r +
					0.587 * this->_pixelArray[y][x].g +
					0.114 * this->_pixelArray[y][x].b;
			}
		}

		/* Cb Cr */
		for (int y = 0; y < this->_stream->codec->height / 2; y++) {
			for (int x = 0; x < this->_stream->codec->width / 2; x++) {
				this->_frame->data[1][y * this->_frame->linesize[1] + x] =
					-0.1687 * this->_pixelArray[y * 2][x * 2].r +
					-0.3313 * this->_pixelArray[y * 2][x * 2].g +
					0.5 *     this->_pixelArray[y * 2][x * 2].b + 128;
				this->_frame->data[2][y * this->_frame->linesize[2] + x] =
					0.5 *     this->_pixelArray[y * 2][x * 2].r +
					-0.4187 * this->_pixelArray[y * 2][x * 2].g +
					-0.0813 * this->_pixelArray[y * 2][x * 2].b + 128;
			}
		}
	}

	void LibAvRenderer::_initStream(sf::Vector2u size, unsigned fps, size_t bitRate)
	{
		int ret;
		AVCodec *codec;
		AVStream *stream = avformat_new_stream(this->_fmtContext, nullptr);
		AVCodecContext *codecContext;
		AVCPBProperties *properties;

		if (!stream)
			throw AvErrorException("Couldn't alloc video stream");

		codecContext = stream->codec;
		codecContext->codec_id = this->_fmtContext->oformat->video_codec;
		codecContext->codec_type = AVMEDIA_TYPE_VIDEO;

		/* Put sample parameters */
		codecContext->bit_rate = bitRate;

		/* Resolution must be a multiple of two */
		codecContext->width = size.x;
		codecContext->height = size.y;

		/* Frames per second */
		codecContext->time_base = {1, static_cast<int>(fps)};
		codecContext->framerate = {static_cast<int>(fps), 1};

		codecContext->pix_fmt = AV_PIX_FMT_YUV420P;

		/* Emit one intra frame every 12 frames at most */
		codecContext->gop_size = 12;
		if (codecContext->codec_id == AV_CODEC_ID_MPEG1VIDEO)
			codecContext->mb_decision = 2;

		if (this->_fmtContext->oformat->flags & AVFMT_GLOBALHEADER)
			codecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

		/* Find the codec */
		codec = avcodec_find_encoder(codecContext->codec_id);
		if (!codec)
			throw AvErrorException("Cannot find codec");

		/* Open it */
		if ((ret = avcodec_open2(codecContext, codec, nullptr)) < 0)
			throw AvErrorException("Cannot open codec", ret);

		properties = (AVCPBProperties *)av_stream_new_side_data(stream, AV_PKT_DATA_CPB_PROPERTIES, sizeof(*properties));

		properties->avg_bitrate = 900;
		properties->max_bitrate = 1000;
		properties->min_bitrate = 0;
		properties->buffer_size = 2 * 1024 * 1024;
		properties->vbv_delay = UINT64_MAX;
		this->_stream = stream;
	}

	void LibAvRenderer::_initFrame()
	{
		AVFrame	*frame = av_frame_alloc();

		if (!frame)
			throw AvErrorException("Could not allocate video frame");
		frame->format = this->_stream->codec->pix_fmt;
		frame->width = this->_stream->codec->width;
		frame->height = this->_stream->codec->height;
		frame->quality = 1;

		int err = av_frame_get_buffer(frame, 32);

		if (err < 0)
			throw AvErrorException("Could not allocate video data buffers", err);
		this->_frame = frame;
	}

	void LibAvRenderer::_flush(bool sendFrame)
	{
		/* send the frame to the encoder */
		int ret = avcodec_send_frame(this->_stream->codec, sendFrame ? this->_frame : nullptr);

		if (ret < 0)
			throw AvErrorException("Error sending a frame for encoding", ret);

		while (ret >= 0) {
			ret = avcodec_receive_packet(this->_stream->codec, this->_packet);
			if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
				return;
			else if (ret < 0)
				throw AvErrorException("Error during encoding", ret);
			//if ((ret = av_interleaved_write_frame(this->_fmtContext, this->_packet)) < 0)
			if ((ret = av_write_frame(this->_fmtContext, this->_packet)) < 0)
				throw AvErrorException("Cannot write in file", ret);
		}
	}
}

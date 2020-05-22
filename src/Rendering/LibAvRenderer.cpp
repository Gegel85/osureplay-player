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
		this->_buffer = new sf::Color[size.x * size.y];
		this->_pixelArray = new sf::Color *[size.y];
		for (unsigned y = 0; y < size.y; y++)
			this->_pixelArray[y] = &this->_buffer[size.x * y];
		LibAvRenderer::clear(sf::Color::Black);
		this->_initVideoCodec(size, fps, bitRate);
		this->_initVideoFrame();

		this->_videoPacket = av_packet_alloc();
		if (!this->_videoPacket)
			throw AvErrorException("Memory allocation error");

		this->_videoStream = fopen((path + ".mp4").c_str(), "wb");
		if (!this->_videoStream)
			throw SystemCallFailedException("fopen(" + path + ".mp4): " + strerror(errno));
	}

	LibAvRenderer::~LibAvRenderer()
	{
		delete[] this->_buffer;
		delete[] this->_pixelArray;
		if (this->_videoStream) {
			this->_flush();

			uint8_t endcode[] = {0, 0, 1, 0xb7};

			/* add sequence end code to have a real MPEG file */
			fwrite(endcode, 1, sizeof(endcode), this->_videoStream);

			fclose(this->_videoStream);
		}

		/* free frames */
		av_frame_free(&this->_videoFrame);

		/* free packets */
		av_packet_free(&this->_videoPacket);

		/* free codec contexts */
		avcodec_free_context(&this->_videoCodecContext);
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
		if (pos.x < 0 || pos.x >= this->_size.x)
			return;
		if (pos.y < 0 || pos.y >= this->_size.y)
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

	void LibAvRenderer::drawRectangle(sf::Vector2i pos, sf::Vector2u size, unsigned thickness, sf::Color color)
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
				if (static_cast<int>(y / scale.y) >= size.y || static_cast<int>(x / scale.x) >= size.x)
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
		this->_convert();

		/* send the frame to the encoder */
		int ret = avcodec_send_frame(this->_videoCodecContext, this->_videoFrame);
		if (ret < 0)
			throw AvErrorException("Error sending a frame for encoding", ret);

		while (ret >= 0) {
			ret = avcodec_receive_packet(this->_videoCodecContext, this->_videoPacket);
			if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
				return;
			else if (ret < 0)
				throw AvErrorException("Error during encoding", ret);

			size_t total = fwrite(this->_videoPacket->data, 1, this->_videoPacket->size, this->_videoStream);

			if (total != this->_videoPacket->size) {
				std::stringstream stream;

				stream << "fwrite(" << std::hex << std::showbase << this->_videoPacket->data << ", 1, " << std::dec << this->_videoPacket->size << ", " << std::hex << this->_videoStream << ")";
				stream << " wrote " << std::dec << total << " bytes instead of " << this->_videoPacket->size << std::endl;
				throw SystemCallFailedException(stream.str());
			}
			av_packet_unref(this->_videoPacket);
		}
	}

	void LibAvRenderer::_convert()
	{
		/* make sure the frame data is writable */
		if (av_frame_make_writable(this->_videoFrame) < 0)
			throw InvalidStateException("The frame data is not writable\n");

		/* prepare the frame */
		/* Y */
		for (int y = 0; y < this->_videoCodecContext->height; y++) {
			for (int x = 0; x < this->_videoCodecContext->width; x++) {
				this->_videoFrame->data[0][y * this->_videoFrame->linesize[0] + x] =
					0.299 * this->_pixelArray[y][x].r +
					0.587 * this->_pixelArray[y][x].g +
					0.114 * this->_pixelArray[y][x].b;
			}
		}

		/* Cb Cr */
		for (int y = 0; y < this->_videoCodecContext->height / 2; y++) {
			for (int x = 0; x < this->_videoCodecContext->width / 2; x++) {
				this->_videoFrame->data[1][y * this->_videoFrame->linesize[1] + x] =
					-0.1687 * this->_pixelArray[y * 2][x * 2].r +
					-0.3313 * this->_pixelArray[y * 2][x * 2].g +
					0.5 *     this->_pixelArray[y * 2][x * 2].b + 128;
				this->_videoFrame->data[2][y * this->_videoFrame->linesize[2] + x] =
					0.5 *     this->_pixelArray[y * 2][x * 2].r +
					-0.4187 * this->_pixelArray[y * 2][x * 2].g +
					-0.0813 * this->_pixelArray[y * 2][x * 2].b + 128;
			}
		}
	}

	void LibAvRenderer::_initVideoCodec(sf::Vector2u size, unsigned frameRate, size_t bitRate)
	{
		const AVCodec *codec;
		AVCodecContext *codecContext;

		/* find the video encoder */
		codec = avcodec_find_encoder(AV_CODEC_ID_MPEG1VIDEO);
		if (!codec)
			throw AvErrorException("MPEG1VIDEO codec not found");
		codecContext = avcodec_alloc_context3(codec);

		/* put sample parameters */
		codecContext->bit_rate = bitRate;

		/* resolution must be a multiple of two */
		codecContext->width = size.x;
		codecContext->height = size.y;

		/* frames per second */
		codecContext->time_base = (AVRational){1, static_cast<int>(frameRate)};
		codecContext->framerate = (AVRational){static_cast<int>(frameRate), 1};

		codecContext->gop_size = 10; /* emit one intra frame every ten frames */
		codecContext->max_b_frames = 1;
		codecContext->pix_fmt = AV_PIX_FMT_YUV420P;

		/* open it */
		int err = avcodec_open2(codecContext, codec, nullptr);
		if (err < 0)
			throw AvErrorException("Couldn't open video codec", err);
		this->_videoCodecContext = codecContext;
	}

	void LibAvRenderer::_initVideoFrame()
	{
		AVFrame	*frame = av_frame_alloc();

		if (!frame)
			throw AvErrorException("Could not allocate video frame");
		frame->format = this->_videoCodecContext->pix_fmt;
		frame->width = this->_videoCodecContext->width;
		frame->height = this->_videoCodecContext->height;
		frame->quality = 1;

		int err = av_frame_get_buffer(frame, 32);

		if (err < 0)
			throw AvErrorException("Could not allocate video data buffers", err);
		this->_videoFrame = frame;
	}

	void LibAvRenderer::_flush()
	{
		/* send the frame to the encoder */
		int ret = avcodec_send_frame(this->_videoCodecContext, nullptr);
		if (ret < 0)
			throw AvErrorException("Error sending a frame for encoding", ret);

		while (ret >= 0) {
			ret = avcodec_receive_packet(this->_videoCodecContext, this->_videoPacket);
			if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
				return;
			else if (ret < 0)
				throw AvErrorException("Error during encoding", ret);

			size_t total = fwrite(this->_videoPacket->data, 1, this->_videoPacket->size, this->_videoStream);

			if (total != this->_videoPacket->size) {
				std::stringstream stream;

				stream << "fwrite(" << std::hex << std::showbase << this->_videoPacket->data << ", 1, " << std::dec << this->_videoPacket->size << ", " << std::hex << this->_videoStream << ")";
				stream << " wrote " << std::dec << total << " bytes instead of " << this->_videoPacket->size << std::endl;
				throw SystemCallFailedException(stream.str());
			}
			av_packet_unref(this->_videoPacket);
		}
	}
}

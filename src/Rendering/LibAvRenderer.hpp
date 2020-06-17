//
// Created by Gegel85 on 21/05/2020.
//

#ifndef OSUREPLAY_PLAYER_LIBAVRENDERER_HPP
#define OSUREPLAY_PLAYER_LIBAVRENDERER_HPP

#include <map>
#include "../libav.hpp"
#include "RenderTarget.hpp"

namespace OsuReplayPlayer
{
	class LibAvRenderer : public RenderTarget {
	private:
		AVFormatContext *_fmtContext;
		AVCodecContext *_enc;
		sf::Vector2u _size;
		sf::Color **_pixelArray;
		sf::Color *_buffer;
		AVStream *_stream;
		AVPacket *_packet;
		AVFrame *_frame;
		unsigned _nextPts = 0;

		void _initStream(sf::Vector2u size, unsigned fps, size_t bitRate, const std::map<std::string, std::string> &opts = {});
		void _initFrame();
		void _prepareFrame();
		void _flush(bool sendFrame);

	public:
		LibAvRenderer(const std::string &path, sf::Vector2u size, unsigned fps, size_t bitRate);
		~LibAvRenderer() override;
		sf::Vector2u getSize() const override;
		void clear(sf::Color color) override;
		void drawPixel(sf::Vector2i pos, sf::Color color) override;
		void drawPoint(sf::Vector2f pos, sf::Color color) override;
		void drawRectangle(sf::Vector2i pos, sf::Vector2u size, unsigned thickness, sf::Color color) override;
		void drawFilledRectangle(sf::Vector2i pos, sf::Vector2u size, sf::Color color) override;
		void drawImage(sf::Vector2i pos, const sf::Image &image, sf::Vector2i newSize, sf::Color tint, bool centered, float rotation) override;
		void drawCircle(unsigned thickness, sf::Vector2i pos, float radius, sf::Color color) override;
		void drawFilledCircle(sf::Vector2i pos, float radius, sf::Color color) override;
		void renderFrame() override;
	};
}


#endif //OSUREPLAY_PLAYER_LIBAVRENDERER_HPP

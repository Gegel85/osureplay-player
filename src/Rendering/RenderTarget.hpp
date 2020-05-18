//
// Created by Gegel85 on 18/05/2020.
//

#ifndef OSUREPLAY_PLAYER_RENDERTARGET_HPP
#define OSUREPLAY_PLAYER_RENDERTARGET_HPP


#include <osu_map_parser.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Image.hpp>

namespace OsuReplayPlayer
{
	class RenderTarget {
	public:
		virtual ~RenderTarget() = default;
		virtual void clear(sf::Color color) = 0;
		virtual void drawPixel(sf::Vector2u pos, sf::Color color) = 0;
		virtual void drawPoint(sf::Vector2f pos, sf::Color color) = 0;
		virtual void drawRectangle(sf::Vector2i pos, sf::Vector2u size, sf::Color color) = 0;
		virtual void drawFilledRectangle(sf::Vector2i pos, sf::Vector2u size, sf::Color color) = 0;
		virtual void drawImage(sf::Vector2i pos, sf::Image &image, sf::Vector2i newSize, sf::Color tint, bool centered, float rotation) = 0;
		virtual void drawCircle(unsigned thinkness, sf::Vector2i pos, int radius, sf::Color color) = 0;
		virtual void drawFilledCircle(sf::Vector2i pos, int radius, sf::Color color) = 0;
		virtual void draw(RenderTarget &window) = 0;
	};
}


#endif //OSUREPLAY_PLAYER_RENDERTARGET_HPP

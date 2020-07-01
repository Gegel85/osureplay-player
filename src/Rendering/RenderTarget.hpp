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
		virtual bool isValid() const;
		virtual sf::Vector2u getSize() const = 0;
		virtual void clear(sf::Color color) = 0;
		virtual void clear(sf::Vector2i pos, const sf::Image &image, sf::Vector2f newSize, unsigned char dimPercent) = 0;
		virtual void drawPixel(sf::Vector2i pos, sf::Color color) = 0;
		virtual void drawPoint(sf::Vector2f pos, sf::Color color) = 0;
		virtual void drawRectangle(sf::Vector2i pos, sf::Vector2u size, unsigned thickness, sf::Color color) = 0;
		virtual void drawFilledRectangle(sf::Vector2i pos, sf::Vector2u size, sf::Color color) = 0;
		virtual void drawImage(sf::Vector2i pos, const sf::Image &image, sf::Vector2f newSize, sf::Color tint, bool centered, float rotation) = 0;
		virtual void drawCircle(unsigned thickness, sf::Vector2i pos, float radius, sf::Color color) = 0;
		virtual void drawFilledCircle(sf::Vector2i pos, float radius, sf::Color color) = 0;
		virtual void renderFrame() = 0;
		virtual void setGlobalPadding(sf::Vector2i padding) = 0;
		virtual sf::Vector2i getGlobalPadding() = 0;
	};
}


#endif //OSUREPLAY_PLAYER_RENDERTARGET_HPP

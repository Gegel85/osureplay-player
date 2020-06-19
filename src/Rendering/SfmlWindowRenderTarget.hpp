//
// Created by Gegel85 on 18/05/2020.
//

#ifndef OSUREPLAY_PLAYER_SFMLWINDOWRENDERTARGET_HPP
#define OSUREPLAY_PLAYER_SFMLWINDOWRENDERTARGET_HPP


#include <SFML/Graphics.hpp>
#include "RenderTarget.hpp"

namespace OsuReplayPlayer
{
	class SFMLWindowRenderTarget : public RenderTarget, public sf::RenderWindow {
	private:
		sf::RectangleShape _rect;
		sf::CircleShape _circle;
		sf::Texture _texture;
		sf::Sprite _sprite;
	public:
		SFMLWindowRenderTarget(sf::Vector2u size, const std::string &title);

		bool isValid() const override;
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
		void setGlobalPadding(sf::Vector2i padding) override;
	};
}


#endif //OSUREPLAY_PLAYER_SFMLWINDOWRENDERTARGET_HPP

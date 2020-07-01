//
// Created by Gegel85 on 18/05/2020.
//

#include "SfmlWindowRenderTarget.hpp"

namespace OsuReplayPlayer
{
	SFMLWindowRenderTarget::SFMLWindowRenderTarget(sf::Vector2u size, const std::string &title) :
		sf::RenderWindow({size.x, size.y}, title)
	{

	}

	sf::Vector2u SFMLWindowRenderTarget::getSize() const
	{
		return sf::RenderWindow::getSize();
	}

	void SFMLWindowRenderTarget::clear(sf::Color color)
	{
		sf::RenderWindow::clear(color);
	}

	void SFMLWindowRenderTarget::drawPixel(sf::Vector2i pos, sf::Color color)
	{
		this->_rect.setFillColor(color);
		this->_rect.setOutlineThickness(0);
		this->_rect.setPosition(pos.x, pos.y);
		this->_rect.setSize({1, 1});
		this->draw(this->_rect);
	}

	void SFMLWindowRenderTarget::drawPoint(sf::Vector2f pos, sf::Color color)
	{
		this->_rect.setFillColor(color);
		this->_rect.setOutlineThickness(0);
		this->_rect.setPosition(pos.x, pos.y);
		this->_rect.setSize({1, 1});
		this->draw(this->_rect);
	}

	void SFMLWindowRenderTarget::drawRectangle(sf::Vector2i pos, sf::Vector2u size, unsigned thickness, sf::Color color)
	{
		this->_rect.setFillColor(sf::Color::Transparent);
		this->_rect.setOutlineColor(color);
		this->_rect.setOutlineThickness(thickness);
		this->_rect.setPosition(pos.x, pos.y);
		this->_rect.setSize({
			static_cast<float>(size.x),
			static_cast<float>(size.y)
		});
		this->draw(this->_rect);
	}

	void SFMLWindowRenderTarget::drawFilledRectangle(sf::Vector2i pos, sf::Vector2u size, sf::Color color)
	{
		this->_rect.setFillColor(color);
		this->_rect.setOutlineThickness(0);
		this->_rect.setPosition(pos.x, pos.y);
		this->_rect.setSize({
			static_cast<float>(size.x),
			static_cast<float>(size.y)
		});
		this->draw(this->_rect);
	}

	void SFMLWindowRenderTarget::drawImage(sf::Vector2i pos, const sf::Image &image, sf::Vector2i newSize, sf::Color tint, bool centered, float rotation)
	{
		sf::Vector2u size = image.getSize();

		if (!size.x || !size.y)
			return;

		sf::Vector2f scale = {
			newSize.x < 0 ? 1 : static_cast<float>(newSize.x) / size.x,
			newSize.y < 0 ? 1 : static_cast<float>(newSize.y) / size.y
		};

		if (centered)
			this->_sprite.setOrigin(size.x / 2.f, size.y / 2.f);
		else
			this->_sprite.setOrigin(0, 0);
		this->_texture.loadFromImage(image);
		this->_sprite.setTexture(this->_texture, true);
		this->_sprite.setPosition(pos.x, pos.y);
		this->_sprite.setScale(scale);
		this->_sprite.setColor(tint);
		this->_sprite.setRotation(rotation);
		this->draw(this->_sprite);
	}

	void SFMLWindowRenderTarget::drawCircle(unsigned thickness, sf::Vector2i pos, float radius, sf::Color color)
	{
		this->_circle.setFillColor(sf::Color::Transparent);
		this->_circle.setOutlineColor(color);
		this->_circle.setOutlineThickness(thickness);
		this->_circle.setPosition(pos.x, pos.y);
		this->_circle.setRadius(radius);
		this->draw(this->_circle);
	}

	void SFMLWindowRenderTarget::drawFilledCircle(sf::Vector2i pos, float radius, sf::Color color)
	{
		this->_circle.setFillColor(color);
		this->_circle.setOutlineThickness(0);
		this->_circle.setPosition(pos.x, pos.y);
		this->_circle.setRadius(radius);
		this->draw(this->_circle);
	}

	bool SFMLWindowRenderTarget::isValid() const
	{
		return this->isOpen();
	}

	void SFMLWindowRenderTarget::renderFrame()
	{
		sf::Event event;

		while (this->pollEvent(event))
			if (event.type == sf::Event::Closed)
				this->close();
		this->display();
	}

	void SFMLWindowRenderTarget::setGlobalPadding(sf::Vector2i padding)
	{
		sf::View view{sf::FloatRect(-padding.x, -padding.y, this->getView().getSize().x, this->getView().getSize().y)};

		this->_padding = padding;
		this->setView(view);
	}

	sf::Vector2i SFMLWindowRenderTarget::getGlobalPadding()
	{
		return this->_padding;
	}

	void SFMLWindowRenderTarget::clear(sf::Vector2i pos, const sf::Image &image, sf::Vector2i newSize, unsigned char dimPercent)
	{
		sf::Color color{
			static_cast<sf::Uint8>(255 * (100 - dimPercent) / 100),
			static_cast<sf::Uint8>(255 * (100 - dimPercent) / 100),
			static_cast<sf::Uint8>(255 * (100 - dimPercent) / 100),
			255
		};

		this->drawImage(pos, image, newSize, color, false, 0);
	}
}

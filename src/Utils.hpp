//
// Created by Gegel85 on 18/05/2020.
//

#ifndef OSUREPLAY_PLAYER_UTILS_HPP
#define OSUREPLAY_PLAYER_UTILS_HPP


#include <string>
#include "Rendering/RenderTarget.hpp"
#include "OsuSkin.hpp"

namespace OsuReplayPlayer::Utils
{
	std::string getLastExceptionName();
	std::vector<std::string> splitString(const std::string &str, const std::string &delim);
	void displayStr(const std::string &str, RenderTarget &target, const OsuSkin &skin, sf::Vector2i pos, unsigned char alpha, unsigned textSize, const std::string &font);
	sf::Vector2u getTextSize(const std::string &str, unsigned charSize);
	unsigned char stouc(const std::string &str);
}


#endif //OSUREPLAY_PLAYER_UTILS_HPP

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
	void displayStr(const std::string &str, RenderTarget &target, const OsuSkin &skin, sf::Vector2i pos, unsigned char alpha, unsigned textSize, const std::string &font);
	sf::Vector2u getTextSize(const std::string &str, unsigned charSize);

	template<typename T>
	void removeDuplicate(std::vector<T> &v)
	{
		auto end = v.end();

		for (auto it = v.begin(); it != end; ++it)
			end = std::remove(it + 1, end, *it);
		v.erase(end, v.end());
	}
}


#endif //OSUREPLAY_PLAYER_UTILS_HPP

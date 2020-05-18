//
// Created by Gegel85 on 17/05/2020.
//

#ifndef OSUREPLAY_PLAYER_OSUSKIN_HPP
#define OSUREPLAY_PLAYER_OSUSKIN_HPP


#include <string>
#include <map>
#include <SFML/Graphics.hpp>
#include "Sound.hpp"

namespace OsuReplayPlayer
{
	class OsuSkin {
	private:
		std::map<std::string, Sound> _sounds;
		std::map<std::string, sf::Image> _images;

		static const std::map<std::string, std::function<void (OsuSkin *, const std::string &)>> _handlers;

	public:
		void addImage(const std::string &path);
		void addSound(const std::string &path);
		void addFolder(const std::string &path);
	};
}


#endif //OSUREPLAY_PLAYER_OSUSKIN_HPP
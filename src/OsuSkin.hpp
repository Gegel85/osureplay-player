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
		Sound _emptySound;
		sf::Image _emptyImage;
		std::map<std::string, Sound> _sounds;
		std::map<std::string, sf::Image> _images;
		std::vector<std::string> _skinned;

		static const std::map<std::string, std::function<void (OsuSkin *, const std::string &)>> _handlers;

	public:
		void addImage(const std::string &path, const std::string &id = "");
		void addSound(const std::string &path, const std::string &id = "");
		void addFolder(const std::string &path);
		const Sound &getSound(const std::string &name) const;
		const sf::Image &getImage(const std::string &name) const;
		bool hasSound(const std::string &name) const;
		bool hasImage(const std::string &name) const;
		bool isImageSkinned(const std::string &name) const;
	};
}


#endif //OSUREPLAY_PLAYER_OSUSKIN_HPP

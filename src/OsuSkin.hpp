//
// Created by Gegel85 on 17/05/2020.
//

#ifndef OSUREPLAY_PLAYER_OSUSKIN_HPP
#define OSUREPLAY_PLAYER_OSUSKIN_HPP


#include <string>
#include <map>
#include <SFML/Graphics.hpp>
#include "Sound.hpp"
#include "Exceptions.hpp"
#include "SimpleIni.h"

namespace OsuReplayPlayer
{
	class OsuSkin {
	private:
		Sound _emptySound;
		sf::Image _emptyImage;
		std::vector<std::string> _skinned;
		std::map<std::string, Sound> _sounds;
		std::map<std::string, sf::Image> _images;
		std::map<std::pair<std::string, std::string>, std::string> _properties;
		mutable std::map<std::pair<std::string, std::string>, long> _propertiesLongCache;
		mutable std::map<std::pair<std::string, std::string>, bool> _propertiesBoolCache;
		mutable std::map<std::pair<std::string, std::string>, double> _propertiesDoubleCache;
		mutable std::map<std::pair<std::string, std::string>, sf::Color> _propertiesColorCache;

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

		template<typename T>
		T getProperty(const std::string &section, const std::string &name) const;

		template<typename T>
		T getProperty(const std::string &section, const std::string &name, const T &defaultValue) const
		{
			try {
				return this->getProperty<T>(section, name);
			} catch (PropertyNotSetException &) {
				return defaultValue;
			}
		}

		bool isPropertySet(const std::string &section, const std::string &name) const;
	};
}


#endif //OSUREPLAY_PLAYER_OSUSKIN_HPP

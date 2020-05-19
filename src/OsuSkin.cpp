//
// Created by Gegel85 on 17/05/2020.
//

#include <filesystem>
#include <iostream>
#include "OsuSkin.hpp"
#include "Exceptions.hpp"
#include "Utils.hpp"

namespace OsuReplayPlayer
{
	void OsuSkin::addFolder(const std::string &path)
	{
		std::cout << path << std::endl;
		for (auto &entry : std::filesystem::directory_iterator(path)) {
			if (entry.is_directory())
				continue;

			const auto &p = entry.path();
			const auto &extension = p.extension().string();

			if (OsuSkin::_handlers.find(extension) == OsuSkin::_handlers.end()) {
#ifdef _DEBUG
				std::cout << "File " << p.string() << " ignored because the extension '" << extension << "' is not recognized as a media format." << std::endl;
#endif
				continue;
			}

#ifdef _DEBUG
			std::cout << "Loading file " << p.string() << std::endl;
#endif
			if (p.has_extension())
				try {
					OsuSkin::_handlers.at(extension)(this, p.string());
				} catch (std::exception &e) {
					std::cerr << p.string() << " ignored: " << Utils::getLastExceptionName() << ": " << e.what() << std::endl;
				}
		}
	}

	const std::map<std::string, std::function<void (OsuSkin *, const std::string &)>> OsuSkin::_handlers{
		{".ogg", [](OsuSkin *skin, const std::string &path){
			skin->addSound(path);
		}},
		{".wav", [](OsuSkin *skin, const std::string &path){
			skin->addSound(path);
		}},
		{".mp3", [](OsuSkin *skin, const std::string &path){
			skin->addSound(path);
		}},
		{".png", [](OsuSkin *skin, const std::string &path){
			skin->addImage(path);
		}},
		{".jpg", [](OsuSkin *skin, const std::string &path){
			skin->addImage(path);
		}}
	};

	void OsuSkin::addImage(const std::string &path)
	{
		auto filename = std::filesystem::path(path).filename();
		std::string name = filename.string();

		if (filename.has_extension())
			name = name.substr(0, name.size() - filename.extension().string().size());

		if (this->_images.find(name) != this->_images.end())
			std::cerr << "Warning: " << path << " will replace the previously loaded image " << name << "." << std::endl;

		sf::Image image;

		if (!image.loadFromFile(path))
			throw ImageLoadingFailedException("Failed to load image " + path);
		this->_images[name] = image;
	}

	void OsuSkin::addSound(const std::string &path)
	{
		auto filename = std::filesystem::path(path).filename();
		std::string name = filename.string();

		if (filename.has_extension())
			name = name.substr(0, name.size() - filename.extension().string().size());

		if (this->_sounds.find(name) != this->_sounds.end())
			std::cerr << "Warning: " << path << " will replace the previously loaded sound " << name << "." << std::endl;
		this->_sounds.emplace(name, path);
	}

	const Sound &OsuSkin::getSound(const std::string &name) const
	{
		try {
			return this->_sounds.at(name);
		} catch (std::out_of_range &) {
			return this->_emptySound;
		}
	}

	const sf::Image &OsuSkin::getImage(const std::string &name) const
	{
		try {
			return this->_images.at(name);
		} catch (std::out_of_range &) {
			return this->_emptyImage;
		}
	}
}
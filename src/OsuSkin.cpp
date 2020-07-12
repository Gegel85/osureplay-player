//
// Created by Gegel85 on 17/05/2020.
//

#include <filesystem>
#include <iostream>
#include <csetjmp>
#include <osu_map_parser.hpp>
#include <fstream>
#include "OsuSkin.hpp"
#include "Utils.hpp"

extern "C" {
	void OsuMap_deleteEmptyLines(char **lines);
	OsuMapCategory *OsuMap_getCategories(char **lines, char *error_buffer, jmp_buf jump_buffer);
	char **OsuMap_splitString(char *str, char separator, char *error_buffer, jmp_buf jump_buffer);
}

namespace OsuReplayPlayer
{
	void OsuSkin::addFolder(const std::string &path)
	{
		std::cout << "Loading skin in folder " << path << std::endl;
		for (auto &entry : std::filesystem::directory_iterator(path)) {
			if (entry.is_directory())
				continue;

			const auto &p = entry.path();
			const auto &extension = p.extension().string();

			if (OsuSkin::_handlers.find(extension) == OsuSkin::_handlers.end()) {
				std::cerr << "File " << p.string() << " ignored because the extension '" << extension << "' is not recognized as a media format." << std::endl;
				continue;
			}

			std::cout << "Loading file " << p.string() << std::endl;
			if (p.has_extension())
				try {
					OsuSkin::_handlers.at(extension)(this, p.string());
				} catch (std::exception &e) {
					std::cerr << p.string() << " ignored: " << Utils::getLastExceptionName() << ": " << e.what() << std::endl;
				}
		}

		std::cout << "Loading file " << path << "/skin.ini" << std::endl;

		std::ifstream stream{path + "/skin.ini"};
		std::string string{
			std::istreambuf_iterator<char>{stream},
			std::istreambuf_iterator<char>{}
		};

		if (stream.fail()) {
			std::cerr << "Cannot load " << path << "/skin.ini: " << strerror(errno) << std::endl;
			return;
		}

		auto str = strdup(string.c_str());
		char error[PATH_MAX + 1024];
		jmp_buf jump_buffer;
		char **lines = nullptr;
		OsuMapCategory *categories = nullptr;

		//Init the error handler
		if (setjmp(jump_buffer)) {
			std::cerr << "Cannot load " << path << "/skin.ini: " << error << std::endl;
			for (int i = 0; categories && categories[i].lines; i++)
				free(categories[i].lines);
			free(categories);
			free(lines);
			free(str);
			return;
		}

		lines = OsuMap_splitString(str, '\n', error, jump_buffer);
		OsuMap_deleteEmptyLines(lines);

		if (!*lines) {
			free(lines);
			free(str);
			return;
		}
		categories = OsuMap_getCategories(lines, error, jump_buffer);

		for (int i = 0; categories && categories[i].lines; i++) {
			std::cout << "[" << categories[i].name << "]" << std::endl;
			for (int j = 0; categories[i].lines[j]; j++) {
				char *elem = strchr(categories[i].lines[j], ':');

				if (!elem)
					continue;

				*elem = '\0';
				do {
					elem++;
				} while (std::isspace(*elem) && *elem);

				this->_properties[{
					categories[i].name,
					categories[i].lines[j]
				}] = elem;
			}
		}

		for (int i = 0; categories && categories[i].lines; i++)
			free(categories[i].lines);
		free(categories);
		free(lines);
		free(str);
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

	void OsuSkin::addImage(const std::string &path, const std::string &id)
	{
		auto filename = std::filesystem::path(path).filename();
		std::string name = id.empty() ? filename.string() : id;

		if (filename.has_extension() && id.empty())
			name = name.substr(0, name.size() - filename.extension().string().size());

		if (this->_images.find(name) != this->_images.end()) {
			std::cerr << "Warning: " << path << " will replace the previously loaded image " << name << "." << std::endl;
			this->_skinned.push_back(name);
		}

		sf::Image image;

		if (!image.loadFromFile(path))
			throw ImageLoadingFailedException("Failed to load image " + path);
		this->_images[name] = image;
	}

	void OsuSkin::addSound(const std::string &path, const std::string &id)
	{
		auto filename = std::filesystem::path(path).filename();
		std::string name = id.empty() ? filename.string() : id;

		if (filename.has_extension() && id.empty())
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

	bool OsuSkin::hasSound(const std::string &name) const
	{
		return this->_sounds.find(name) != this->_sounds.end();
	}

	bool OsuSkin::hasImage(const std::string &name) const
	{
		return this->_images.find(name) != this->_images.end();
	}

	bool OsuSkin::isImageSkinned(const std::string &name) const
	{
		return std::find(this->_skinned.begin(), this->_skinned.end(), name) != this->_skinned.end();
	}

	template <> std::string OsuSkin::getProperty<std::string>(const std::string &section, const std::string &name) const
	{
		try {
			return this->_properties.at({section.c_str(), name.c_str()});
		} catch (std::out_of_range &) {
			throw PropertyNotSetException("[" + section + "]: " + name + " is not set.");
		}
	}

	template <> bool OsuSkin::getProperty<bool>(const std::string &section, const std::string &name) const
	{
		try {
			return this->_propertiesBoolCache.at({section, name});
		} catch (std::out_of_range &e) {}

		auto elem = this->getProperty<std::string>(section, name);

		if (elem == "false")
			return this->_propertiesBoolCache[{section, name}] = false;
		if (elem == "true")
			return this->_propertiesBoolCache[{section, name}] = true;

		try {
			return this->_propertiesBoolCache[{section, name}] = std::stoul(elem);
		} catch (std::exception &e) {
			throw InvalidBoolException("[" + section + "] " + name + ": " + e.what());
		}
	}

	template <> double OsuSkin::getProperty<double>(const std::string &section, const std::string &name) const
	{
		try {
			return this->_propertiesDoubleCache.at({section, name});
		} catch (std::out_of_range &e) {}

		try {
			return this->_propertiesDoubleCache[{section, name}] = std::stod(this->getProperty<std::string>(section, name));
		} catch (std::exception &e) {
			throw InvalidDoubleException("[" + section + "] " + name + ": " + e.what());
		}
	}

	template <> long OsuSkin::getProperty<long>(const std::string &section, const std::string &name) const
	{
		try {
			return this->_propertiesLongCache.at({section, name});
		} catch (std::out_of_range &e) {}

		try {
			return this->_propertiesLongCache[{section, name}] = std::stol(this->getProperty<std::string>(section, name));
		} catch (std::exception &e) {
			throw InvalidLongException("[" + section + "] " + name + ": " + e.what());
		}
	}

	template <> sf::Color OsuSkin::getProperty<sf::Color>(const std::string &section, const std::string &name) const
	{
		try {
			return this->_propertiesColorCache.at({section, name});
		} catch (std::out_of_range &e) {}

		auto property = this->getProperty<std::string>(section, name);
		auto color = Utils::splitString(property, ",");

		if (color.size() < 3)
			throw InvalidColorException("[" + section + "] " + name + ": \"" + property + "\" is not a valid color.");

		try {
			return this->_propertiesColorCache[{section, name}] = {
				Utils::stouc(color[0]),
				Utils::stouc(color[1]),
				Utils::stouc(color[2]),
				color.size() == 3 ? static_cast<unsigned char>(255) : Utils::stouc(color[3])
			};
		} catch (std::exception &e) {
			throw InvalidColorException("[" + section + "] " + name + ": " + e.what());
		}
	}

	bool OsuSkin::isPropertySet(const std::string &section, const std::string &name) const
	{
		try {
			this->getProperty<std::string>(section, name);
			return true;
		} catch (PropertyNotSetException &) {
			return false;
		}
	}
}
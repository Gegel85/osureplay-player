//
// Created by Gegel85 on 18/05/2020.
//

#ifdef __GNUG__
#include <cxxabi.h>
#endif
#include "Utils.hpp"

namespace OsuReplayPlayer::Utils {
	std::string getLastExceptionName()
	{
#ifdef __GNUG__
		int status;
		char *value;
		std::string name;

		auto val = abi::__cxa_current_exception_type();

		if (!val)
			return "No exception";

		value = abi::__cxa_demangle(val->name(), nullptr, nullptr, &status);
		name = value;
		free(value);
		return name;
#else
		return "Unknown exception";
#endif
	}

	void displayStr(const std::string &str, RenderTarget &target, const OsuSkin &skin, sf::Vector2i pos, unsigned char alpha, unsigned textSize, const std::string &font)
	{
		sf::Vector2u size  = getTextSize(str, textSize);
		std::string displayed;

		for (char c : str) {
			displayed += c;
			target.drawImage(
				{
					static_cast<int>(pos.x - size.x / 2 + getTextSize(displayed, textSize).x - 7.5),
					static_cast<int>(pos.y - size.y / 2 + getTextSize(displayed, textSize).y - 7.5),
				},
				skin.getImage(font + "-" + c),
				{static_cast<float>(textSize), static_cast<float>(textSize)},
				{255, 255, 255, alpha},
				true,
				0
			);
		}
	}

	sf::Vector2u getTextSize(const std::string &str, unsigned charSize)
	{
		unsigned     buffer = 0;
		sf::Vector2u size = {0, charSize};

		for (char c : str) {
			if (c == '\n') {
				size.y += charSize;
				size.x = std::max(size.x, buffer);
				buffer = 0;
			} else
				buffer += charSize;
		}
		size.x = std::max(size.x, buffer);
		return size;
	}

	unsigned char stouc(const std::string &str)
	{
		unsigned long elem = std::stoul(str);

		if (elem > 255)
			throw std::out_of_range("Utils::stouc");
		return elem;
	}

	std::vector<std::string> splitString(const std::string &str, const std::string &delim)
	{
		std::vector<std::string> tokens;
		size_t prev = 0;
		size_t pos = 0;

		do {
			pos = str.find(delim, prev);
			if (pos == std::string::npos)
				pos = str.length();

			std::string token = str.substr(prev, pos - prev);

			tokens.push_back(token);
			prev = pos + delim.length();
		} while (pos < str.length() && prev < str.length());
		return tokens;
	}
}
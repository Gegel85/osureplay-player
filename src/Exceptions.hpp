//
// Created by Gegel85 on 17/05/2020.
//

#ifndef OSUREPLAY_PLAYER_EXCEPTIONS_HPP
#define OSUREPLAY_PLAYER_EXCEPTIONS_HPP


#include <exception>
#include <string>
#include "libav.hpp"

namespace OsuReplayPlayer
{
	class BaseException : public std::exception {
	private:
		std::string _msg;

	public:
		BaseException(const std::string &msg) : _msg(msg) {};
		const char *what() const noexcept override { return this->_msg.c_str(); };
	};

	class InvalidBeatmapException : public BaseException {
	public:
		InvalidBeatmapException(const std::string &msg) : BaseException(msg) {};
	};

	class InvalidReplayException : public BaseException {
	public:
		InvalidReplayException(const std::string &msg) : BaseException(msg) {};
	};

	class ImageLoadingFailedException : public BaseException {
	public:
		ImageLoadingFailedException(const std::string &msg) : BaseException(msg) {};
	};

	class NoAudioStreamException : public BaseException {
	public:
		NoAudioStreamException(const std::string &msg) : BaseException(msg) {};
	};

	class AvErrorException : public BaseException {
	public:
		AvErrorException(const std::string &msg) :
			BaseException(msg) {};
		AvErrorException(const std::string &msg, int averror) :
			BaseException(msg + ": " + getAvErrorCode(averror))
		{};
	};

	class InvalidSliderException : public BaseException {
	public:
		InvalidSliderException(const std::string &msg) : BaseException(msg) {};
	};

	class NotImplementedException : public BaseException {
	public:
		NotImplementedException(const std::string &msg) : BaseException(msg) {};
	};
}


#endif //OSUREPLAY_PLAYER_EXCEPTIONS_HPP

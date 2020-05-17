//
// Created by Gegel85 on 17/05/2020.
//

#include "libav.hpp"

std::string getAvErrorCode(int num)
{
	char buffer[50];

	if (av_strerror(num, buffer, sizeof(buffer)) < 0)
		return std::string("Unknown error");
	return std::string(buffer);
}
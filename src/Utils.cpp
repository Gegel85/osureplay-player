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
}
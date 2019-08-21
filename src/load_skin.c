#include <stdbool.h>
#ifdef _MSC_VER
#include <io.h>
#include <Windows.h>
#else
#include <unistd.h>
#include <dirent.h>
#endif
#include <string.h>
#include <malloc.h>
#include <concatf.h>
#include <skin.h>
#include <ctype.h>
#include <errno.h>
#include "frame_buffer.h"
#include "dict.h"

char	*getFileName(char *path)
{
	bool	del = false;

	for (size_t i = strlen(path) - 1; i > 0; i--) {
		if (path[i] == '.' && !del) {
			del = true;
			path[i] = '\0';
		} else if (path[i] == '/' || path[i] == '\\')
			return &path[i];
	}
	return path;
}

const char	*getFileExtension(const char *path)
{
	for (size_t i = strlen(path) - 1; i > 0; i--)
		if (path[i] == '.')
			return &path[i] + 1;
	return NULL;
}

char	*strToLower(char *str)
{
	for (int i = 0; str[i]; i++)
		str[i] = tolower(str[i]);
	return str;
}

#ifdef _MSC_VER

bool	loadSkin(const char *folder, Dict *images, Dict *sounds, Dict *loaders)
{
	HANDLE		*hFind = INVALID_HANDLE_VALUE;
	char		*path;
	LoadingPair	*pair;
	WIN32_FIND_DATA entry;

	if (hFind == INVALID_HANDLE_VALUE) {
		wchar_t *s = NULL;

		FormatMessageW(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			GetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPWSTR)&s,
			0,
			NULL
		);

		display_warning("Cannot open '%s': %S\n", folder, s);
		return false;
	}

	do {
		if (*entry.cFileName == '.')
			continue;

		pair = Dict_getElement(loaders, getFileExtension(entry.cFileName));
		if (!pair)
			continue;

		path = concatf("%s/%s", folder, entry.cFileName);
		if (!path)
			display_error("Out of memory");

		char *index = strToLower(getFileName(entry.cFileName));
		void *data = pair->creator(path);

		if (!data)
			continue;

		if (pair->type == IMAGE) {
			if (!Dict_addElement(images, index, data, pair->destroyer))
				display_error("Out of memory");
		}
		else if (pair->type == SOUND) {
			if (!Dict_addElement(sounds, index, data, pair->destroyer))
				display_error("Out of memory");
		}
		free(path);
	} while (FindNextFile(hFind, &entry));

	if (GetLastError() == ERROR_NO_MORE_FILES) {
		wchar_t *s = NULL;

		FormatMessageW(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			GetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPWSTR)&s,
			0,
			NULL
		);

		display_warning("Error: %S\n", s);
		FindClose(hFind);
		return false;
	}

	FindClose(hFind);
	return true;
}

#else

bool	loadSkin(const char *folder, Dict *images, Dict *sounds, Dict *loaders)
{
	DIR		*dirstream = opendir(folder);
	char		*path;
	LoadingPair	*pair;

	if (!dirstream) {
		display_warning("Cannot open '%s': %s\n", folder, strerror(errno));
		return false;
	}
	for (struct dirent *entry = readdir(dirstream); entry; entry = readdir(dirstream)) {
		if (*entry->d_name == '.')
			continue;

		pair = Dict_getElement(loaders, getFileExtension(entry->d_name));
		if (!pair)
			continue;

		path = concatf("%s/%s", folder, entry->d_name);
		if (!path)
			display_error("Out of memory");

		char *index = strToLower(getFileName(entry->d_name));
		void *data = pair->creator(path);

		if (!data)
			continue;

		if (pair->type == IMAGE) {
			if (!Dict_addElement(images, index, data, pair->destroyer))
				display_error("Out of memory");
		} else if (pair->type == SOUND) {
			if (!Dict_addElement(sounds, index, data, pair->destroyer))
				display_error("Out of memory");
		}
		free(path);
	}
	closedir(dirstream);
	return true;
}

#endif
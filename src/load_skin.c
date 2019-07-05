#include <stdbool.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <malloc.h>
#include <concatf.h>
#include <skin.h>
#include <ctype.h>
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
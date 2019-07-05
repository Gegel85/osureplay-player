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
		path = concatf("%s/%s", folder, entry->d_name);
		pair = Dict_getElement(loaders, getFileExtension(entry->d_name));
		if (pair && pair->type == IMAGE)
			Dict_addElement(images, strToLower(getFileName(entry->d_name)), pair->creator(path), pair->destroyer);
		else if (pair && pair->type == SOUND)
			Dict_addElement(sounds, strToLower(getFileName(entry->d_name)), pair->creator(path), pair->destroyer);
		else
			display_warning("Unsupported file %s couldn't be loaded\n", path);
		free(path);
	}
	closedir(dirstream);
	return true;
}
/*
** EPITECH PROJECT, 2017
** my_putstr
** File description:
** display a string
*/

#include "concatf.h"
#ifdef _MSC_VER
#include <io.h>
#else
#include <unistd.h>
#endif

int	my_putstr_stderr(char const *str)
{
	return (write(2, str, my_strlen_rec(str)));
}

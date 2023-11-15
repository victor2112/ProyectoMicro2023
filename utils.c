#include "utils.h"

void removeChar(char *input_str, char char_to_remove)
{
    char *src, *dst;
    for (src = dst = input_str; *src != '\0'; src++)
    {
        *dst = *src;
        if (*dst != char_to_remove)
            dst++;
    }
    *dst = '\0';
}

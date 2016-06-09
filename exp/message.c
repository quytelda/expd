/* message.c - EXP messages and message parsing
 * Copyright (C) 2016 Quytelda Kahja
 *
 * This file is part of EXPd.
 *
 * EXPd is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * EXPd is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with EXPd.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <string.h>
#include <assert.h>
#include "message.h"

/**
 * strim() - Trim leading and trailing whitespace from a string.
 * @str the string to trim (should not be NULL, must be null-terminated)
 * Returns a pointer to the beginning of the trimmed string on success,
 * or NULL on failure.  This function DOES modify the string pointed to by @str.
 */
static char * strim(const char * str)
{
    assert(str);
    return NULL;
}

/**
 * strto_message() - Parse a string into a struct exp_message.
 * @str the string to parse (should not be NULL)
 * Returns a struct exp_message corresponding to @str on success,
 * or NULL on failure.  This function MAY modify the string pointed to by @str.
 * strto_message() is the inverse of message_tostr().
 */
struct exp_message * strto_message(const char * str)
{
    assert(str);
    return NULL;
}

/**
 * message_tostr() - Convert a struct exp_message into string form.
 * @msg A pointer to the struct exp_message to stringify (should not be NULL)
 * Returns a pointer to a string corresponding to @msg on success,
 * or NULL on failure.
 * message_tostr() is the inverse of strto_message().
 */
char * message_tostr(const struct exp_message * msg)
{
    assert(msg);
    return NULL;
}

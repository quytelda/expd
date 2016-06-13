/* user.h - EXP user objects
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
#ifndef __USER_H
#define __USER_H

#include <arpa/inet.h>

#define ANON_USERNAME "anonymous"

struct exp_user
{
    int sid, uid;
    char * username;
    char * nickname;
};

#endif /* __USER_H */

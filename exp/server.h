/* server.h - EXP server objects
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

#ifndef __SERVER_H
#define __SERVER_H

#include <arpa/inet.h>

struct exp_server
{
    char * name;

    int fd;
    struct sockaddr_storage * addr;
};

#endif /* __SERVER_H */

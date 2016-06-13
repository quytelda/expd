/* client.h - EXP client class agnostic features
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

#ifndef __EXP_CLIENT_H
#define __EXP_CLIENT_H

#include "user.h"
#include "server.h"

typedef enum { USER_CLASS, SERVER_CLASS } conn_class_t;

struct exp_client
{
    conn_class_t class;

    union
    {
	struct exp_user user;
	struct exp_server server;
    } data;
};

#endif /* __EXP_CLIENT_H */

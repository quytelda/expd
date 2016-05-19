/* protocol.h - EXP protocol definitions
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

#ifndef __PROTOCOL_H
#define __PROTOCOL_H

#include "user.h"
#include "server.h"

#define RSP_SUCCESS        00
#define RSP_FAILURE        01
#define RSP_TARGET_EXISTS  02
#define RSP_TARGET_UNKNOWN 03

typedef enum { SRC_SERVER, SRC_USER } exp_source_t;

struct query
{
    exp_source_t source_type;
    union source
    {
	struct exp_user   user;
	struct exp_server server;
    } source;

    char * command;
    char * arguments[];
};

struct response
{
    int numeric;
    char * source;
    char * command;
    char * comment;
};

char * query_to_str(const struct query * qry);
struct query * str_to_query(const char * str);

char * response_to_str(const struct response * rsp);
struct response * str_to_response(const char * str);

#endif

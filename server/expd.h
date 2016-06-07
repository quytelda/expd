/* expd.h - EXPd server daemon headers
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

#ifndef __SERVER_EXPD_H
#define __SERVER_EXPD_H

#include "exp/client.h"
#include "exp/message.h"

#define MAX_CLIENTS 1024

/* Just in case SOMAXCONN is undefined; We prefer the OS's value. */
#ifndef SOMAXCONN
    #define SOMAXCONN 128
#endif

struct server_config
{
    int port;
};

void start_expd(struct server_config *);
void stop_expd(void);

int send_client(struct exp_client client, const char * str);
int msg_client(struct exp_client client, struct exp_message);
int drop_client(struct exp_client client);

#endif /* __SERVER_EXPD_H */

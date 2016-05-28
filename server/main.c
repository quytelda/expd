/* main.c - Extensible eXchange Protocol Daemon
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

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include "expd.h"

int main(int argc, char * argv[])
{
    struct server_config * config =
	(struct server_config *) calloc(1, sizeof(struct server_config));
    config->port = 3333;

    start_expd(config);

    sigset_t sigmask;
    sigemptyset(&sigmask);
    sigsuspend(&sigmask);

    return 0;
}

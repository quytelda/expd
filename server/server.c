/*
 * server.c - EXPd server daemon
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
#include <unistd.h>
#include <sys/socket.h>
#include <errno.h>
#include <arpa/inet.h>

#include "server.h"

/**
 * setup_socket() - Open and configure a new TCP server socket
 * @port The port on which to bind the socket
 * Creates and initializes a new TCP server socket bound to @port,
 * and returns it's file descriptor.
 */
static int setup_socket(int port)
{
    	struct sockaddr_in serv_addr =
	{
	    .sin_family      = AF_INET,
	    .sin_addr.s_addr = htonl(INADDR_ANY),
	    .sin_port        = htons(port),
	};

    	int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(socket_fd < 0)
	{
	    fprintf(stderr, "Unable to open new TCP socket (errno %d).\n", errno);
	    return -errno;
	}

	int err = bind(socket_fd, (struct sockaddr *) &serv_addr, sizeof(struct sockaddr_in));
	if(err < 0)
	{
	    fprintf(stderr, "Unable to bind TCP socket (errno %d).\n", errno);
	    return -errno;
	}

	return socket_fd;
}

/**
 * start_server() - Launch the server daemon
 * @config The server configuration options to use on startup
 * Run the server daemon with the provided configuration.
 */
void start_server(struct server_config * config)
{
    // set up listening ports
    int sock_fd = setup_socket(config->port);
    if(sock_fd < 0)
    {
	fprintf(stderr, "Unable to initialize sockets.\n");
	exit(1);
    }

    if(listen(sock_fd, CLIENT_BACKLOG) < 0)
    {
	fprintf(stderr, "Unable to listen to for incoming connections (errno %d).\n", errno);
	exit(1);
    }

    // client accept loop
    for(;;)
    {
	struct sockaddr saddr;
	socklen_t saddr_len;
	int client_fd = accept(sock_fd, &saddr, &saddr_len);
	if(client_fd < 0)
	    fprintf(stderr, "Failed to accept incoming connection (errno %d).\n", errno);

	close(client_fd);
    }
}

/**
 * stop_server() - Terminate the server daemon
 * Cleanly exit the running daemon.
 */
void stop_server(void)
{
}

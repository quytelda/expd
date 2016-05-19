/*
 * expd.c - EXPd server daemon
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
#include <string.h>
#include <sys/socket.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/epoll.h>

#include "expd.h"

#define EPOLL_MAX_EVENTS 64
#define EPOLL_TIMEOUT -1

#define IN_BUFSIZE 1024

static int sock_fd, epoll_fd;

/**
 * setup_socket() - Open and configure a new TCP server socket
 * @port The port on which to bind the socket
 * Creates and initializes a new TCP server socket bound to @port,
 * and returns it's file descriptor.
 *
 * When  successful,  setup_socket()  returns  zero.
 * When an error occurs, setup_socket() returns -1 and errno is set appropriately.
 */
static int setup_socket(int port)
{
    struct sockaddr_in serv_addr =
    {
	.sin_family      = AF_INET,
	.sin_addr.s_addr = htonl(INADDR_ANY),
	.sin_port        = htons(port),
    };

    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(sock_fd < 0) return -1;

    if(bind(sock_fd, (struct sockaddr *) &serv_addr, sizeof(struct sockaddr_in)) < 0)
	return -1;

    if(listen(sock_fd, CLIENT_BACKLOG) < 0)
	return -1;

    return sock_fd;
}

/**
 * accept_client() - Accept a new client from the connection queue.
 * Calls accept() to accept a new client in the queue, which is
 * assumed to be ready.
 */
static void accept_client(void)
{
    int peer_fd = accept(sock_fd, NULL, NULL);
    if(peer_fd < 0)
    {
	fprintf(stderr, "Failed to accept client connection (errno %d).\n", errno);
	return;
    }

    struct epoll_event peer_fd_ev =
    {
	.events = EPOLLIN | EPOLLRDHUP | EPOLLHUP | EPOLLET,
	.data.fd = peer_fd,
    };
    if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, peer_fd, &peer_fd_ev) < 0)
	fprintf(stderr, "Failed to add epoll monitor (errno %d).\n", errno);
}

/**
 * handle_client_event() - Handle epoll events from a network client.
 * @event epoll event from the client's file descriptor
 * If the client has sent data, we parse it and send it to the dispatch.
 * If the client has closed the connection, we clean up the connection.
 * Otherwise, the event is ignored.
 */
static void handle_client_event(struct epoll_event event)
{
    if(event.events & (EPOLLHUP | EPOLLRDHUP)) // client hung up
    {
	if(close(event.data.fd) < 0)
	    fprintf(stderr, "Failed to close client socket (errno %d).\n", errno);
    }
    else // data was received
    {
	char buffer[IN_BUFSIZE];
	bzero(buffer, IN_BUFSIZE);

	int count = read(event.data.fd, &buffer, IN_BUFSIZE);
	if(count < 0)
	{
	    fprintf(stderr, "Failed to read from client socket (errno %d).\n", errno);
	    return;
	}

	printf("%s", buffer);
    }
}

/**
 * start_server() - Launch the server daemon
 * @config The server configuration options to use on startup
 * Run the server daemon with the provided configuration.
 */
void start_server(struct server_config * config)
{
    // set up sockets to listen for clients
    sock_fd = setup_socket(config->port);
    if(sock_fd < 0)
    {
	fprintf(stderr, "Unable to open listening socket (errno %d)\n.", errno);
	exit(1);
    }

    // set up epoll interface
    epoll_fd = epoll_create1(0);
    if(epoll_fd < 0)
    {
	fprintf(stderr, "Unable to create epoll interface (errno %d)\n.", errno);
	exit(1);
    }

    struct epoll_event sock_connect_ev =
    {
	.events  = EPOLLIN,
	.data.fd = sock_fd,
    };
    if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sock_fd, &sock_connect_ev) < 0)
    {
	fprintf(stderr, "Failed to configure epoll (errno %d).\n", errno);
	exit(1);
    }

    // epoll event loop
    int quantity;
    struct epoll_event * events =
	(struct epoll_event *) malloc(sizeof(struct epoll_event));
    for(;;)
    {
	quantity = epoll_wait(epoll_fd, events, EPOLL_MAX_EVENTS, EPOLL_TIMEOUT);

	for(int i = 0; i < quantity; i++)
	{
	    if(events[i].data.fd == sock_fd)
		accept_client();
	    else
		handle_client_event(events[i]);
	}
    }
}

/**
 * stop_server() - Terminate the server daemon
 * Cleanly exit the running daemon.
 */
void stop_server(void)
{
    close(epoll_fd);
}

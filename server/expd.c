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
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "expd.h"
#include "debug.h"

#define EPOLL_MAX_EVENTS 64
#define EPOLL_TIMEOUT -1
#define IN_BUFSIZE 1024

static int quit = 0;
static pthread_t accept_thread;
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
 * accept_clients() - Accept new clients from the connection queue.
 * Repeatedly calls accept() to accept new clients in the queue, which blocks
 * until a new client is ready.  This function is intended to be run in a pthread
 * and should never return.
 */
static void * accept_clients(void * arg)
{
    while(!quit)
    {
	int peer_fd = accept(sock_fd, NULL, NULL);
	if(quit)
	    return NULL;
	else if(peer_fd < 0)
	    continue_with_errno("Failed to accept client connection");

	struct epoll_event * peer_fd_ev = calloc(1, sizeof(struct epoll_event));
	if(!peer_fd_ev)
	    continue_with_errno("Unable to allocate memory");

	peer_fd_ev->events = EPOLLIN | EPOLLRDHUP | EPOLLHUP | EPOLLET;
	peer_fd_ev->data.fd = peer_fd;
	if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, peer_fd, peer_fd_ev) < 0)
	    print_errno("Failed to add epoll monitor");
    }

    return NULL;
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
	    print_errno("Failed to close client socket");
    }
    else // data was received
    {
	/* TODO: calloc this buffer */
	char buffer[IN_BUFSIZE];
	bzero(buffer, IN_BUFSIZE);

	int count = read(event.data.fd, &buffer, IN_BUFSIZE);
	if(count < 0)
	{
	    fprintf(stderr, "Failed to read from client socket (errno %d).\n", errno);
	    return;
	}

	printf("%s", buffer);
	if(strncmp(buffer, "DIE", 3) == 0)
	    stop_server();
    }
}

/**
 * start_server() - Launch the server daemon
 * @config The server configuration options to use on startup
 * Run the server daemon with the provided configuration.
 * TODO: should this return or not?
 */
void start_server(struct server_config * config)
{
#ifdef DEBUG
    puts("* Starting server...");
#endif

    int err;

    /* 
     * Set up sockets to listen for clients.
     * We want to accept connection in parellel to handling events,
     * so we will launch a new pthread to handle that socket.
     */
    sock_fd = setup_socket(config->port);
    if(sock_fd < 0)
	leave_with_errno("Unable to open listening socket");

    err = pthread_create(&accept_thread, NULL, accept_clients, NULL);
    if(err)
	leave_with_errno("Unable to create client accept thread");

    // set up epoll interface
    epoll_fd = epoll_create1(0);
    if(epoll_fd < 0)
	leave_with_errno("Unable to create epoll interface");

    /*
     * This is the main event loop for the Epoll instance.
     * Loop waiting for events until the server is exited.
     * XXX: should this start *before* accept() is called?
     */
#ifdef DEBUG
    puts("* Entering primary event loop");
#endif
    int quantity;
    struct epoll_event * events =
	(struct epoll_event *) malloc(sizeof(struct epoll_event));
    if(!events) leave_with_errno("Failed to allocate space");

    while(!quit)
    {
	quantity = epoll_wait(epoll_fd, events, EPOLL_MAX_EVENTS, EPOLL_TIMEOUT);
	if(quantity < 0)
	    continue_with_errno("An error occurred while waiting for events");

	for(int i = 0; i < quantity; i++)
	    handle_client_event(events[i]);
    }

#ifdef DEBUG
    puts("* Exiting server...");
#endif

leave:
    err = quit ^ 1;
    stop_server();
    exit(err ? 1 : 0);
}

/**
 * stop_server() - Terminate the server daemon
 * Cleanly exit the running daemon.
 * TODO: should stop_server even bother checking error codes?
 */
void stop_server(void)
{
    quit = 1;

    // stop accepting new clients
    if(shutdown(sock_fd, SHUT_RDWR) < 0)
    {
	print_errno("Failed to close accept() socket");
	goto cleanup_epoll;
    }

    close(sock_fd);
    pthread_join(accept_thread, NULL);

cleanup_epoll:
    close(epoll_fd);
}

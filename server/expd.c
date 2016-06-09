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
#include <signal.h>
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
static pthread_t epoll_thread, accept_thread;
static int accept_fd  = -1;
static int epoll_fd = -1;

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

    int accept_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(accept_fd < 0) return -1;

    int val = 1;
    setsockopt(accept_fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(int));

    if(bind(accept_fd, (struct sockaddr *) &serv_addr, sizeof(struct sockaddr_in)) < 0)
	return -1;

    if(listen(accept_fd, SOMAXCONN) < 0)
	return -1;

    return accept_fd;
}

/**
 * accept_incoming() - Accept new clients from the connection queue.
 * Repeatedly calls accept() to accept new clients in the queue, which
 * blocks until a new client is ready.  This function is intended to be run in
 * a pthread and should never actually return.
 * It is intended to terminate only when the pthread receives SIGINT.
 */
static void * accept_incoming(void * arg)
{
    struct server_config * config = (struct server_config *) arg;
    accept_fd = setup_socket(config->port);
    if(accept_fd < 0)
	leave_with_errno("Failed to open accept() socket");

#ifdef DEBUG
    printf("* Listening for connections on port %d.\n", config->port);
#endif

    for(;;)
    {
	int peer_fd = accept(accept_fd, NULL, NULL);
	if(quit)
	    break;
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

#ifdef DEBUG
    puts("* Quitting accept() thread.");
#endif

leave:
    if(accept_fd > 0)
	close(accept_fd);
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
	    stop_expd();
    }
}

/**
 * do_epoll() - manage client connections using the Epoll inteface
 * Create a new epoll instance that will be used to manage client connections.
 * Then loop forever calling epoll_wait() to listen for client events.
 * This function is intended to run in a pthread and should never actually return.
 * It is only intended to exit when the pthread receives SIGINT.
 */
static void * do_epoll(void * arg)
{
    // set up epoll interface
    epoll_fd = epoll_create1(0);
    if(epoll_fd < 0)
	leave_with_errno("Unable to create epoll interface");

#ifdef DEBUG
    puts("* Starting epoll monitor.");
#endif

    int quantity;
    struct epoll_event * events =
	(struct epoll_event *) malloc(sizeof(struct epoll_event));
    if(!events) leave_with_errno("Failed to allocate space");

    for(;;)
    {
	quantity = epoll_wait(epoll_fd, events, EPOLL_MAX_EVENTS, EPOLL_TIMEOUT);
	if(quit)
	    break;
	else if(quantity < 0)
	    continue_with_errno("An error occurred while waiting for events");

	for(int i = 0; i < quantity; i++)
	    handle_client_event(events[i]);
    }

#ifdef DEBUG
    puts("* Quitting epoll monitor thread.");
#endif

leave:
    if(epoll_fd > 0)
	close(epoll_fd);
    free(events);
    return NULL;
}

/**
 * start_expd() - Launch the server daemon
 * @config The server configuration options to use on startup
 * Run the server daemon with the provided configuration.
 * TODO: should this return or not?
 */
void start_expd(struct server_config * config)
{
    int err;

    /* 
     * We want to accept connection in parellel to handling events,
     * so we will launch a new pthread to handle each task.
     */
    err = pthread_create(&epoll_thread, NULL, do_epoll, config);
    if(err)
	return_with_errno("Unable to create epoll monitoring thread");

    err = pthread_create(&accept_thread, NULL, accept_incoming, config);
    if(err)
	return_with_errno("Unable to create client accept thread");
}

/**
 * stop_expd() - Terminate the server daemon
 * Cleanly exit the running daemon.
 * TODO: should stop_expd even bother checking error codes?
 */
void stop_expd(void)
{
#ifdef DEBUG
    puts("* Stopping server.");
#endif

    // TODO: notify clients of the shutdown
    // TODO: close client connections

    // "shutting down" flag
    quit = 1;

    // don't accept any more new clients
    if(!shutdown(accept_fd, SHUT_RDWR))
    {
	pthread_join(accept_thread, NULL);
    }
    else
    {
	print_errno("Unable to dismantle accept() socket");
	pthread_cancel(accept_thread);
    }
    close(accept_fd);

    // take down epoll
    if(!close(epoll_fd))
    {
	pthread_join(epoll_thread, NULL);
    }
    else
    {
	print_errno("Unable to dismantle accept() socket");
	pthread_cancel(epoll_fd);
    }

#ifdef DEBUG
    puts("* All threads have stopped.");
#endif
}

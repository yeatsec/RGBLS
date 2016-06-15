#include <unistd.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <strings.h>
#include "opc_client.h"

#define MAX_STRIP_LEN	256
#define	MAX_MSG_SIZE	3844 // 256 * 3 + 4

int sock_fd; // file descriptor for socket
struct sockaddr_in6 sa;
char * server_address;
int port;

int initialized = 0;

int opc_client_init(int _port, char * _server_address)
{
	// set up client communications
	port = _port;
	server_address = _server_address;
	if (initialized)
	{
		printf("opc_client already initialized\n");
		return 1;
	}
	sock_fd = socket(AF_INET6, SOCK_DGRAM, 0);
	if (sock_fd < 0)
	{
		printf("socket error\n");
		return 2;
	}
	bzero(&sa, sizeof(struct sockaddr_in6));
	sa.sin6_family = AF_INET6;
	if (inet_pton(AF_INET6, server_address, &sa.sin6_addr) == 0)
	{
		printf("inet_pton\n");
		return 3;
	}
	sa.sin6_port = htons(port);
	// ready to send datagram
	initialized = 1;
	printf("client initialized\n");
	return 0;
}

int opc_client_send_formatted(char _channel, char _command, rgb  _rgb_arr[])
{
	if (!initialized)
	{
		printf("attempted send w/o initialization\n");
		return 1;
	}
	char msg[MAX_MSG_SIZE];
	msg[0] = _channel;
	msg[1] = _command;
	msg[2] = (char) ((MAX_STRIP_LEN * 5 * 3)/256);
	msg[3] = (char) ((MAX_STRIP_LEN * 5 * 3)%256);

	for (int i = 0; i < MAX_STRIP_LEN * 5; ++i)
	{
		int base_index = 3*i + 4;	// BRG format
		msg[base_index] = _rgb_arr[i].blue;
		msg[base_index + 1] = _rgb_arr[i].red;
		msg[base_index + 2] = _rgb_arr[i].green;
	}

	// sendto
	if (sendto(sock_fd, msg, 4+(3*(MAX_STRIP_LEN * 5)), 0,(struct sockaddr *)&sa, sizeof(struct sockaddr_in6)) < 0)
	{
		printf("datagram sendto failed!\n");
		return 2;
	}
	return 0;
}

int opc_client_close()
{
	if (!initialized)
	{
		printf("attempted close on uninitialized opc_client\n");
		return 1;
	}
	close(sock_fd);
	bzero(&sa, sizeof(struct sockaddr_in6));
	port = 0;
	server_address = NULL;
	initialized = 0;
	return 0;
}

void opc_client_rgb_strip_init(rgb_strip * _strip, int _size)
{
	_strip->size = _size;
	_strip->rgb_leds = (rgb *)malloc(_size*sizeof(rgb));
	for (int i = 0; i < _size; ++i)
	{
		(_strip->rgb_leds)[i].red=0;
		(_strip->rgb_leds)[i].green=0;
		(_strip->rgb_leds)[i].blue=0;
	}
}

void opc_client_rgb_strip_destroy(rgb_strip * _strip)
{
	free(_strip->rgb_leds);
	_strip->size = 0;
}



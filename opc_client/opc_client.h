/*
* Declares interface for a UDP OPC client that formats and sends messages
*
*/

#ifndef _OPC_CLIENT_H_
#define	_OPC_CLIENT_H_

typedef struct rgb_struct {
	char red;
	char green;
	char blue;
} rgb;

typedef struct rgb_strip_struct {
	rgb rgb_leds[];
	int size;
} rgb_strip;

int opc_client_init(int _port, char * _server_address);

int opc_client_send_formatted(char _channel, char _command, rgb_strip * _strip_ptr);

int opc_client_close();

void opc_client_rgb_strip_init(rgb_strip * _strip, int _size);

void opc_client_rgb_strip_destroy(rgb_strip * _strip);

#endif	// _OPC_CLIENT_H_

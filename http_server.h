#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>

#pragma comment(lib, "Ws2_32.lib")

typedef void (*request_handler_t)(SOCKET client_socket, const char* request_body);

// Initialize the server
int http_server_init(int port);

// Cleanup and stop the server
void http_server_cleanup();

// Register a handler for GET requests
void http_server_register_get(const char* path, request_handler_t handler);

// Register a handler for POST requests
void http_server_register_post(const char* path, request_handler_t handler);

// Register a handler for PUT requests
void http_server_register_put(const char* path, request_handler_t handler);

// Register a handler for DELETE requests
void http_server_register_delete(const char* path, request_handler_t handler);

// Start the server and begin listening for connections
void http_server_start();

#endif // HTTP_SERVER_H

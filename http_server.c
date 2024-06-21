#include "http_server.h"
#include <string.h>
#include <stdlib.h>

#define MAX_PATHS 100
#define BUFFER_SIZE 4096

typedef struct {
    char path[100];
    request_handler_t handler;
} route_t;

typedef struct {
    route_t get_routes[MAX_PATHS];
    int get_route_count;
    route_t post_routes[MAX_PATHS];
    int post_route_count;
    route_t put_routes[MAX_PATHS];
    int put_route_count;
    route_t delete_routes[MAX_PATHS];
    int delete_route_count;
} http_server_t;

static http_server_t server;
static SOCKET server_socket;

// Function prototypes
static void handle_request(SOCKET client_socket);
static void process_request(SOCKET client_socket, const char* method, const char* path, const char* request_body);

static long get_current_time_ms() {
    LARGE_INTEGER frequency;
    LARGE_INTEGER current_time;

    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&current_time);

    return (long)((current_time.QuadPart * 1000) / frequency.QuadPart);
}

int http_server_init(int port) {
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        printf("WSAStartup failed: %d\n", result);
        return -1;
    }

    server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_socket == INVALID_SOCKET) {
        printf("socket failed: %ld\n", WSAGetLastError());
        WSACleanup();
        return -1;
    }

    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        printf("bind failed: %ld\n", WSAGetLastError());
        closesocket(server_socket);
        WSACleanup();
        return -1;
    }

    if (listen(server_socket, SOMAXCONN) == SOCKET_ERROR) {
        printf("listen failed: %ld\n", WSAGetLastError());
        closesocket(server_socket);
        WSACleanup();
        return -1;
    }

    server.get_route_count = 0;
    server.post_route_count = 0;
    server.put_route_count = 0;
    server.delete_route_count = 0;

    return 0;
}

void http_server_cleanup() {
    closesocket(server_socket);
    WSACleanup();
}

void http_server_register_get(const char* path, request_handler_t handler) {
    if (server.get_route_count < MAX_PATHS) {
        strcpy(server.get_routes[server.get_route_count].path, path);
        server.get_routes[server.get_route_count].handler = handler;
        server.get_route_count++;
    }
}

void http_server_register_post(const char* path, request_handler_t handler) {
    if (server.post_route_count < MAX_PATHS) {
        strcpy(server.post_routes[server.post_route_count].path, path);
        server.post_routes[server.post_route_count].handler = handler;
        server.post_route_count++;
    }
}

void http_server_register_put(const char* path, request_handler_t handler) {
    if (server.put_route_count < MAX_PATHS) {
        strcpy(server.put_routes[server.put_route_count].path, path);
        server.put_routes[server.put_route_count].handler = handler;
        server.put_route_count++;
    }
}

void http_server_register_delete(const char* path, request_handler_t handler) {
    if (server.delete_route_count < MAX_PATHS) {
        strcpy(server.delete_routes[server.delete_route_count].path, path);
        server.delete_routes[server.delete_route_count].handler = handler;
        server.delete_route_count++;
    }
}

void http_server_start() {
    printf("Server is running...\n");

    while (1) {
        struct sockaddr_in client_addr;
        int client_addr_size = sizeof(client_addr);
        SOCKET client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_size);
        if (client_socket == INVALID_SOCKET) {
            printf("accept failed: %ld\n", WSAGetLastError());
            continue;
        }

        handle_request(client_socket);
    }
}

static void handle_request(SOCKET client_socket) {
    char buffer[BUFFER_SIZE];
    int bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_received == SOCKET_ERROR) {
        printf("recv failed: %d\n", WSAGetLastError());
        closesocket(client_socket);
        return;
    }

    buffer[bytes_received] = '\0';

    char method[10];
    char path[100];
    sscanf(buffer, "%s %s", method, path);

    char* body = strstr(buffer, "\r\n\r\n");
    if (body) {
        body += 4; // Skip the \r\n\r\n
    } else {
        body = "";
    }

    process_request(client_socket, method, path, body);
    closesocket(client_socket);
}

static void process_request(SOCKET client_socket, const char* method, const char* path, const char* request_body) {
    int i;
    if (strcmp(method, "GET") == 0) {
        for (i = 0; i < server.get_route_count; i++) {
            if (strcmp(path, server.get_routes[i].path) == 0) {
                server.get_routes[i].handler(client_socket, request_body);
                return;
            }
        }
    } else if (strcmp(method, "POST") == 0) {
        for (i = 0; i < server.post_route_count; i++) {
            if (strcmp(path, server.post_routes[i].path) == 0) {
                server.post_routes[i].handler(client_socket, request_body);
                return;
            }
        }
    } else if (strcmp(method, "PUT") == 0) {
        for (i = 0; i < server.put_route_count; i++) {
            if (strcmp(path, server.put_routes[i].path) == 0) {
                server.put_routes[i].handler(client_socket, request_body);
                return;
            }
        }
    } else if (strcmp(method, "DELETE") == 0) {
        for (i = 0; i < server.delete_route_count; i++) {
            if (strcmp(path, server.delete_routes[i].path) == 0) {
                server.delete_routes[i].handler(client_socket, request_body);
                return;
            }
        }
    }

    const char* response = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
    send(client_socket, response, strlen(response), 0);
}

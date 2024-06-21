#include "http_server.h"

void handle_get_root(SOCKET client_socket, const char* request_body) {
    const char* response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 13\r\n\r\nHello, World!";
    send(client_socket, response, strlen(response), 0);
}

void handle_post_data(SOCKET client_socket, const char* request_body) {
    const char* response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 14\r\n\r\nPOST request!";
    send(client_socket, response, strlen(response), 0);
}

int main() {
    if (http_server_init(8080) != 0) {
        return 1;
    }

    http_server_register_get("/", handle_get_root);
    http_server_register_post("/data", handle_post_data);

    http_server_start();

    http_server_cleanup();

    return 0;
}

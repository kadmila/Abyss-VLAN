#ifndef PLATFORM_H
#define PLATFORM_H

#define VERSION_TEXT "0.1.0"

#include <stdint.h>

struct console_arguments {
    char    cidr[64];
    char    config_file[4096];
};
int argument_parse(int argc, char **argv, struct console_arguments *args);

// platform-specific initialization and cleanup
int platform_init(void);
void platform_cleanup(void);

struct platform_virtual_adapter;
typedef const void * ADAPTER_RECV_HANDLE;
typedef const void * ADAPTER_SEND_HANDLE;

struct platform_socket;
typedef const void * SOCKET_RECV_HANDLE;
typedef const void * SOCKET_SEND_HANDLE;

struct platform_virtual_adapter* platform_create_adapter(
    const char *adapter_name, const char *cidr);
int platform_adapter_recv(
    struct platform_virtual_adapter *adapter, ADAPTER_RECV_HANDLE* handle, 
    const uint8_t **buffer, uint32_t *length, uint32_t timeout_ms);
void platform_adapter_recv_close(
    struct platform_virtual_adapter *adapter, ADAPTER_RECV_HANDLE handle);
int platform_adapter_send_prepare(
    struct platform_virtual_adapter *adapter, ADAPTER_SEND_HANDLE* handle, 
    uint8_t **buffer, uint32_t length);
int platform_adapter_send_commit(
    struct platform_virtual_adapter *adapter, ADAPTER_SEND_HANDLE handle);
void platform_close_adapter(
    struct platform_virtual_adapter *adapter);

struct platform_socket* platform_socket_open(const char *bind_address);
int platform_socket_recv(
    struct platform_socket *socket, SOCKET_RECV_HANDLE* handle, 
    const uint8_t **buffer, uint32_t *length, uint32_t timeout_ms);
void platform_socket_recv_close(
    struct platform_socket *socket, SOCKET_RECV_HANDLE handle);
int platform_socket_send_prepare(
    struct platform_socket *socket, SOCKET_SEND_HANDLE* handle, 
    uint8_t **buffer, uint32_t length);
int platform_socket_send_commit(
    struct platform_socket *socket, SOCKET_SEND_HANDLE handle);
void platform_socket_close(
    struct platform_socket *socket);

#endif
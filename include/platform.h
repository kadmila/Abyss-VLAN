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
int avlan_init(void);
void avlan_cleanup(void);

struct avlan_adapter;
struct avlan_adapter* avlan_create_adapter(const char *adapter_name, const char *cidr);
int avlan_adapter_poll(struct avlan_adapter *adapter, uint8_t *buffer, uint32_t buffer_size, uint32_t *bytes_received);
int avlan_adapter_wait(struct avlan_adapter *adapter, uint32_t timeout_ms);
int avlan_adapter_write(struct avlan_adapter *adapter, const uint8_t *buffer, uint32_t length);
void avlan_close_adapter(struct avlan_adapter *adapter);

#endif
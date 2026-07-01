#include <stdio.h>
#include <stdlib.h>

#include "../include/defaults.h"
#include "../include/platform.h"

int main(int argc, char *argv[])
{
    struct console_arguments args;
    if (argument_parse(argc, argv, &args) != 0) {
        return 1;
    }

    printf("cidr: %s\n", args.cidr);
    printf("config: %s\n", args.config_file);

    if (platform_init() != 0) 
    {
        return 1;
    }

    struct platform_virtual_adapter *adapter = platform_create_adapter("Abyss VLAN", args.cidr);
    if (adapter == NULL) {
        fprintf_s(stderr, "Failed to create adapter\n");
        goto fail;
    }

    uint8_t packet_buffer[MAX_PACKET_SIZE];
    uint32_t bytes_received;
    while(platform_adapter_poll(adapter, (uint8_t *)packet_buffer, sizeof(packet_buffer), &bytes_received) == 0) {
        if (bytes_received == 0)
        {
            platform_adapter_wait(adapter, -1);
            continue;
        }

        for (size_t i = 0; i < bytes_received; i += HEX_BYTES_PER_LINE)  {
            char line[HEX_BYTES_LINE_LENGTH] = { 0 };
            uint32_t linesize = MIN(HEX_BYTES_PER_LINE, bytes_received - i);
            for (uint32_t j = 0; j < linesize; ++j) {
                sprintf_s(line + (size_t)(j * 3), _countof(line) - (size_t)(j * 3), "%02X ", packet_buffer[i + j]);
            }
            printf("%s\n", line);
        }
    }
    
    platform_close_adapter(adapter);

    platform_cleanup();
    return 0;

fail:
    platform_cleanup();
    return 1;
}
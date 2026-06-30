#ifndef PLATFORM_H
#define PLATFORM_H

#include <stdint.h>

struct console_arguments {
    uint8_t virtual_ip[4];
    char    config_path[4096];
    
};

int run (void);

int argument_parse(int argc, const char **argv, struct console_arguments *args);

#endif
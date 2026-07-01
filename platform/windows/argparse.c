#include "../../include/platform.h"

#include <string.h>
#include <ws2tcpip.h>

#include "../../include/defaults.h"

#include "../../third_party/argtable3/argtable3.h"

struct arg_lit *help, *version;
struct arg_str *cidr;
struct arg_file *config;
struct arg_end *end;

int argument_parse(int argc, char **argv, struct console_arguments *args) {
    void *argtable[] = {
        help    = arg_litn(NULL, "help", 0, 1, "display this help and exit"),
        version = arg_litn(NULL, "version", 0, 1, "display version info and exit"),
        cidr    = arg_strn(NULL, "cidr", "<cidr>", 1, 1, "virtual IP address and prefix (e.g., 10.6.1.1/24)"),
        config  = arg_filen("c", "config", "<file>", 1, 1, "config file path"),
        end     = arg_end(20),
    };

    int exitcode = 0;
    char progname[] = "AbyssVlan.exe";

    int nerrors;
    nerrors = arg_parse(argc,argv,argtable);

    // special case: '--help' takes precedence over error reporting
    if (help->count > 0)
    {
        printf("Usage: %s", progname);
        arg_print_syntax(stdout, argtable, "\n");
        arg_print_glossary(stdout, argtable, "  %-25s %s\n");
        exitcode = 0;
        goto exit;
    }
    if (version->count > 0)
    {
        printf("%s version %s\n", progname, VERSION_TEXT);
        exitcode = 0;
        goto exit;
    }

    // parse error handling
    if (nerrors > 0)
    {
        arg_print_errors(stdout, end, progname);
        printf("Try '%s --help' for more information.\n", progname);
        exitcode = 1;
        goto exit;
    }
    
    // parse cidr
    strcpy_s(args->cidr, sizeof(args->cidr), cidr->sval[0]);

    // parse config file path
    strcpy_s(args->config_file, sizeof(args->config_file), config->filename[0]);

exit:
    /* deallocate each non-null entry in argtable[] */
    arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
    return exitcode;
}
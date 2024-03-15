#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include <getopt.h>

#include "emulator.h"
#include "program.h"
#include "types.h"
#include "die.h"

static struct option long_options[] =
{
    {"help", no_argument,    NULL, 'h'},
    {"verbose", no_argument, NULL, 'v'},
    {0}
};

void usage(bool full)
{
    if (full)
        die(
            1,
            "usage: %s [-h] file\n"
            "Executes .mqa files (compiled code for MQ CPU)\n"
            "Arguments:\n"
            "  file           file to execute\n"
            "Options:\n"
            "  -h, --help     show this help message and quit\n"
            "  -v, --verbose  show additional information\n",
            program_name
        );
    else
        die(
            1,
            "Try '%s --help' for more information\n",
            program_name
        );
}

int main(i32 argc, s8 **argv)
{
    program_name = argv[0];

    u8  flags = 0;
    i32 ch;
    while ((ch = getopt_long(argc, argv, "hv", long_options, NULL)) != -1)
    {
        switch (ch)
        {
        case 'v':
            flags |= EMULATOR_VERBOSE;
            break;
        case 'h':
            usage(true);
        default:
            usage(false);
        }
    }
    if (argv[optind] == NULL)
    {
        fprintf(stderr, "%s: missing file operand\n", program_name);
        usage(false);
    }

    emulator_init(argv[optind], flags);
        emulator_read_instructions();
        emulator_run();
    emulator_free();

    return 0;
}

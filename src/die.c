#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include "program.h"
#include "types.h"

void die(i32 code, s8 *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    exit(code);
}

void error(void)
{
    die(errno, "%s: %s\n", program_name, strerror(errno));
}

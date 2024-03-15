#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include "commands.h"
#include "emulator.h"
#include "program.h"
#include "types.h"
#include "die.h"

#define MEM_SIZE   65536
#define STACK_SIZE 256
#define PORTS      256

typedef struct flags
{
    u8 halted    : 1,
       interrupt : 1,
       carry     : 1;
} flags;

#define F_HALTED    0b00000001
#define F_INTERRUPT 0b00000010
#define F_CARRY     0b00000100

#define I_OPCODE    0b0000000001111111
#define I_ARG       0b0111111110000000
#define I_FLAG      0b1000000000000000

#define I_ARG_OFF   7
#define I_FLAG_OFF  8

typedef struct mq
{
    /* Registers     Description        */
    u16 r_acc;    /* Accumulator        */
    u16 r_pc;     /* Program Counter    */
    u16 r_usp;    /* User Stack Pointer */
    u16 r_csp;    /* Call Stack Pointer */
    u16 r_rp;     /* ROM Page           */
    u16 r_cp;     /* Cache Page         */
    u16 r_flags;  /* Flags Register     */

    /* Memory        Size Description */
    u16 *m_rom;   /* 2^16 ROM         */
    u8  *m_cache; /* 2^16 Cache       */
    u8  *m_us;    /* 2^8  User Stack  */
    u8  *m_cs;    /* 2^8  Call Stack  */
    u8  *m_ports; /* 2^8  I/O Ports   */

    /* Emulator variables    Description*/
    u8          flags;    /* Emulator flags      */
    s8         *filename; /* File being executed */
} mq;

static mq pc = {0};

void emulator_init(s8 *filename, u8 flags)
{
    memset(&pc, 0, sizeof(pc));

    pc.m_rom    = calloc(MEM_SIZE, sizeof(pc.m_rom));
    pc.m_cache  = calloc(MEM_SIZE, sizeof(pc.m_cache));
    pc.m_us     = calloc(STACK_SIZE, sizeof(pc.m_us));
    pc.m_cs     = calloc(STACK_SIZE, sizeof(pc.m_cs));
    pc.m_ports  = calloc(PORTS, sizeof(pc.m_ports));

    pc.flags    = flags;
    pc.filename = filename;
}

void emulator_free(void)
{
    free(pc.m_rom);
    free(pc.m_cache);
    free(pc.m_us);
    free(pc.m_cs);
    free(pc.m_ports);
}

void emulator_read_instructions(void)
{
    if (pc.flags & EMULATOR_VERBOSE) fputs("Loading the instructions...\n", stderr);

    i32 size, high, low;
    FILE *file = fopen(pc.filename, "r");
    if (errno) error();
        fseek(file, 0, SEEK_END);
        size = ftell(file);
        fseek(file, 0, SEEK_SET);
        if (size > MEM_SIZE)
        {
            fclose(file);
            die(1, "%s: File is larger than MQ can handle\n", program_name);
        } else if (size & 1)
        {
            fclose(file);
            die(1, "%s: Not a mqa executable\n", program_name);
        }

        for (u16 i = 0; i < MEM_SIZE; ++i)
        {
            if ((high = fgetc(file)) == -1 || (low = fgetc(file)) == -1)
            {
                if (errno)
                {
                    fclose(file);
                    error();
                }
                else
                {
                    break;
                }
            }
            pc.m_rom[i] = (high << 8) + low;
            if (pc.flags & EMULATOR_VERBOSE)
                fprintf(
                    stderr,
                    (pc.m_rom[i] & I_FLAG) ? "%s $0x%02X\n" : "%s  0x%02X\n",
                    strinstruction(pc.m_rom[i] & I_OPCODE),
                    (pc.m_rom[i] & I_ARG) >> I_ARG_OFF
                );
        }
    fclose(file);
}

void emulator_run(void)
{}

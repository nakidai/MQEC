#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>

#include "instructions.h"
#include "emulator.h"
#include "program.h"
#include "types.h"
#include "luts.h"
#include "die.h"
#include "mq.h"

typedef struct __attribute__((packed)) mqa_header
{
    u32 cpu_version;
    u16 include_size;
    u32 code_size;
} mqa_header;

static mq pc = {0};
static mqa_header header = {};

void emulator_finish()
{
    if (pc.flags & EMULATOR_VERBOSE)
        die(0, "Finished after %ld instructions\n", pc.ticks);
    exit(0);
}

u8 *get_cache(u8 offset)
{
    return pc.m_cache + (pc.r_cp << 8) + offset;
}

u16 *get_rom(u8 offset)
{
    return pc.m_rom + (pc.r_rp << 8) + offset;
}

void emulator_init(s8 *filename, u8 flags)
{
    memset(&pc, 0, sizeof(pc));

    pc.m_rom    = calloc(MEM_SIZE, sizeof(pc.m_rom));
    pc.m_cache  = calloc(MEM_SIZE, sizeof(pc.m_cache));
    pc.m_us     = calloc(STACK_SIZE, sizeof(pc.m_us));
    pc.m_cs     = calloc(STACK_SIZE, sizeof(pc.m_cs));
    pc.m_ports  = calloc(PORTS, sizeof(pc.m_ports));
    if (errno) error();

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

void parse_header(FILE *file)
{
    if (fread(&header, 1, sizeof(header), file) != sizeof(header))
    {
        if (errno)
            error();
        else
            die(1, "%s: Cannot read header\n", program_name);
    }

    if (pc.flags & EMULATOR_VERBOSE)
        fprintf(
            stderr,
            "Header:\n"
            "  MQ Version:   %c%c%c%c\n"
            "  Include size: %d\n"
            "  Code size:    %d\n",
            *((s8 *)&header), *(((s8 *)&header) + 1), *(((s8 *)&header) + 2), *(((s8 *)&header) + 3),
            header.include_size,
            header.code_size
        );
    if (pc.flags & EMULATOR_WARNING && header.include_size)
        fputs(
            "File has include section, which is not implemented yet\n",
            stderr
        );
}

void emulator_read_instructions(void)
{
    if (pc.flags & EMULATOR_VERBOSE)
        fprintf(stderr, "Loading %s...\n", pc.filename);

    FILE *file = fopen(pc.filename, "r");
    if (errno) error();
        parse_header(file);
        if (header.code_size & 1)
        {
            fclose(file);
            die(1, "%s: Not a mqa executable\n", program_name);
        }
        if (pc.flags & EMULATOR_VERBOSE)
            fputs("Offset Opcode  Arg\n", stderr);

        /* fseek because modules (and imports too) are not implemented yet */
        fseek(file, header.include_size, SEEK_CUR);

        for (u16 i = 0;; ++i)
        {
            if (fread(pc.m_rom + i, 1, sizeof(*pc.m_rom), file) != sizeof(*pc.m_rom))
            // if ((high = fgetc(file)) == -1 || (low = fgetc(file)) == -1)
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
            // pc.m_rom[i] = (high << 8) + low;
            if (pc.flags & EMULATOR_VERBOSE)
                fprintf(
                    stderr,
                    (pc.m_rom[i] & I_FLAG) ? "0x%04X %-6s $0x%02X\n" : "0x%04X %-6s  0x%02X\n",
                    i,
                    emulator_strinstruction(pc.m_rom[i] & I_OPCODE),
                    (pc.m_rom[i] & I_ARG) >> I_ARG_OFF
                );
            if (i == header.code_size) break;
        }
    fclose(file);

    if (pc.flags & EMULATOR_VERBOSE)
        fputs("Instructions were loaded successfully!\n", stderr);
}

void emulator_run(void)
{
    u16 instruction;
    u8 flag, data, bus, opcode;

    for (;;++pc.ticks)
    {
        instruction = *get_rom(pc.r_pc);

        flag   = (instruction & I_FLAG) >> I_FLAG_OFF;
        data   = (instruction & I_ARG) >> I_ARG_OFF;
        opcode = (instruction & I_OPCODE) >> I_OPCODE_OFF;
        bus    = (flag && opcode != 2) ? *get_cache(data) : data;

        emulator_do_tick(&pc, opcode, bus);

        ++pc.r_pc;

        if (pc.flags & EMULATOR_DEBUG)
            fprintf(
                stderr,
                "I: %d|%d|%d(%s)\n"
                "ACC: 0x%02X PC:  0x%02X%02X\n"
                "CS:  0x%02X US:    0x%02X\n"
                "CP:  0x%02X Bus:   0x%02X\n"
                "Flags: 0b%08b\n",
                flag, data, opcode, emulator_strinstruction(opcode),
                pc.r_acc, pc.r_rp, pc.r_pc,
                pc.r_csp, pc.r_usp,
                pc.r_cp, bus,
                pc.r_flags
            );
        if (pc.flags & EMULATOR_PAUSE)
            getchar();
    }
}

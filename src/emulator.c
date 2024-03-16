#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>

#include "emulator.h"
#include "program.h"
#include "types.h"
#include "luts.h"
#include "die.h"

#define MEM_SIZE     65536
#define STACK_SIZE   256
#define PORTS        256

#define F_HALTED     0b00000001
#define F_INT        0b00000010
#define F_CARRY      0b00000100

#define F_HALTED_OFF 0
#define F_INT_OFF    1
#define F_CARRY_OFF  2

#define I_OPCODE     0b0000000001111111
#define I_ARG        0b0111111110000000
#define I_FLAG       0b1000000000000000

#define I_OPCODE_OFF 0
#define I_ARG_OFF    7
#define I_FLAG_OFF   15

typedef void (*instruction)(u8);

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
    u32         ticks;    /* Ticks executed */
} mq;

static mq pc = {0};

void emulator_finish()
{
    if (pc.flags & EMULATOR_VERBOSE)
        die(0, "Finished after %ld instructions\n", pc.ticks);
    exit(0);
}

static u8 *get_cache(u8 offset)
{
    return pc.m_cache + (pc.r_cp << 8) + offset;
}

static u16 *get_rom(u8 offset)
{
    return pc.m_rom + (pc.r_rp << 8) + offset;
}

static void i_nop(u8 bus) {}

static void i_lra(u8 bus)
{
    pc.r_acc = bus;
}

static void i_sra(u8 bus)
{
    *get_cache(bus) = pc.r_acc;
}

static void i_call(u8 bus)
{
    pc.m_cs[pc.r_csp] = pc.r_pc;
    ++pc.r_csp;
    pc.r_pc = (pc.r_rp << 8) + bus - 1;
}

static void i_ret(u8 bus)
{
    --pc.r_csp;
    pc.r_pc = pc.m_cs[pc.r_csp] - 1;
}

static void i_jmp(u8 bus)
{
    pc.r_pc = (pc.r_rp << 8) + bus - 1;
}

static void i_jmpp(u8 bus)
{
    if (pc.r_acc != 0)
        pc.r_pc = (pc.r_rp << 8) + bus - 1;
}

static void i_jmpz(u8 bus)
{
    if (pc.r_acc == 0)
        pc.r_pc = (pc.r_rp << 8) + bus - 1;
}

static void i_jmpn(u8 bus)
{
    if (pc.r_acc & 0b1000000)
        pc.r_pc = (pc.r_rp << 8) + bus - 1;
}

static void i_jmpc(u8 bus)
{
    if (pc.r_flags & F_CARRY)
        pc.r_pc = (pc.r_rp << 8) + bus - 1;
}

static void i_ccf(u8 bus)
{
    pc.r_flags &= ~F_CARRY;
}

static void i_lrp(u8 bus)
{
    pc.r_acc = *get_cache(bus);
}

static void i_ccp(u8 bus)
{
    pc.r_cp = bus;
}

static void i_crp(u8 bus)
{
    pc.r_rp = bus;
}

static void i_push(u8 bus)
{
    pc.m_us[pc.r_usp] = pc.r_acc;
    ++pc.r_usp;
}

static void i_pop(u8 bus)
{
    --pc.r_usp;
    pc.r_acc = pc.m_us[pc.r_usp];
}

static void i_and(u8 bus)
{
    pc.r_acc = pc.r_acc & bus;
}

static void i_or(u8 bus)
{
    pc.r_acc = pc.r_acc | bus;
}

static void i_xor(u8 bus)
{
    pc.r_acc = pc.r_acc ^ bus;
}

static void i_not(u8 bus)
{
    pc.r_acc = ~pc.r_acc;
}

static void i_lsc(u8 bus)
{
    u16 action;

    action = (pc.r_acc << bus) + ((pc.r_flags & F_CARRY) >> F_CARRY_OFF);
    if (action & ~255)
        pc.r_flags |= F_CARRY;
    pc.r_acc = action & 255;
}

static void i_rsc(u8 bus)
{
    u8 t = pc.r_acc;
    pc.r_acc = (pc.r_acc >> bus) + (((pc.r_flags & F_CARRY) >> F_CARRY_OFF) << 7);
    if (((t >> bus) << bus) != t)
        pc.r_flags |= F_CARRY;
}

static void i_cmp(u8 bus)
{
    u8 negative = (pc.r_acc & 0b10000000) & (bus & 0b10000000);

    if (pc.r_acc > bus)
        pc.r_acc = 1;
    else if (pc.r_acc == bus)
        pc.r_acc = 0;
    else
        pc.r_acc = 255;

    if (negative)
        pc.r_acc = ~pc.r_acc;
}

static void i_cmpu(u8 bus)
{
    if (pc.r_acc > bus)
        pc.r_acc = 1;
    else if (pc.r_acc == bus)
        pc.r_acc = 0;
    else
        pc.r_acc = 255;
}

static void i_adc(u8 bus)
{
    u16 action = pc.r_acc + bus + ((pc.r_flags & F_CARRY) >> F_CARRY_OFF);

    if (action & ~255)
        pc.r_flags |= F_CARRY;
    pc.r_acc = action & 255;
}

static void i_sbc(u8 bus)
{
    u16 action = pc.r_acc - bus - ((pc.r_flags & F_CARRY) >> F_CARRY_OFF);

    if (action & ~255)
        pc.r_flags |= F_CARRY;
    pc.r_acc = action & 255;
}

static void i_inc(u8 bus)
{
    ++pc.r_acc;
    if (pc.r_acc == 0)
        pc.r_flags |= F_CARRY;
}

static void i_dec(u8 bus)
{
    --pc.r_acc;
    if (pc.r_acc == 255)
        pc.r_flags |= F_CARRY;
}

static void i_abs(u8 bus)
{
    if (pc.r_acc & 0b10000000)
        pc.r_acc = ~pc.r_acc + 1;
}

static void i_mul(u8 bus)
{
    pc.r_acc *= bus;
}

static void i_div(u8 bus)
{
    pc.r_acc /= bus;
}

static void i_mod(u8 bus)
{
    pc.r_acc %= bus;
}

static void i_tse(u8 bus)
{
    pc.r_acc = emulator_lut_sin(bus);
}

static void i_tce(u8 bus)
{
    pc.r_acc = emulator_lut_cos(bus);
}

static void i_add(u8 bus)
{
    u16 action = pc.r_acc + bus;

    if (action & ~255)
        pc.r_flags |= F_CARRY;
    pc.r_acc = action & 255;
}

static void i_sub(u8 bus)
{
    u16 action = pc.r_acc - bus;

    if (action & ~255)
        pc.r_flags |= F_CARRY;
    pc.r_acc = action & 255;
}

static void i_rpl(u8 bus)
{
    pc.r_acc = emulator_lut_rpl(bus);
}

static void i_mulh(u8 bus)
{
    u16 action = pc.r_acc * bus;
    pc.r_acc = (action & 0b1111111100000000) >> 8;
}

static void i_ui(u8 bus)
{
    s8 input[8] = {0};
    bool is_number = true;

    if (fgets(input, 8, stdin) == (s8 *)EOF)
        if (errno) error();
    if (input[0] == 0)
    {
        pc.r_acc = 0;
        return;
    }

    for (s8 *i = input; *i != 0; ++i)
    {
        if (!isdigit(*i))
        {
            is_number = false;
            break;
        }
    }

    if (is_number)
        pc.r_acc = atoi(input);
    else
        pc.r_acc = input[0];
}

static void i_uo(u8 bus)
{
    printf("%d\n", pc.r_acc);
}

static void i_uoc(u8 bus)
{
    putchar(pc.r_acc);
}

static void i_uocr(u8 bus)
{
    putchar(pc.r_acc);
    putchar('\n');
}

static void i_prw(u8 bus)
{
    pc.m_ports[bus] = pc.r_acc;
}

static void i_prr(u8 bus)
{
    pc.r_acc = pc.m_ports[bus];
}

static void i_halt(u8 bus)
{
    pc.r_flags |= F_HALTED;
    emulator_finish();
}

static void i_not_implemented(u8 bus)
{
    fprintf(stderr, "Unimplemented instruction at 0x%02X%02X\n", pc.r_rp, pc.r_pc);
}

static void i_not_exist(u8 bus)
{
    die(1, "Non-existent instruction at 0x%02X%02X\n", pc.r_rp, pc.r_pc);
}

static instruction select_instruction(u8 opcode)
{
    if (pc.flags & EMULATOR_DEPRECATED)
        switch (opcode)
        {
        case 48:
        case 49:
        case 50:
        case 51:
            fprintf(
                stderr, "Deprecated %s at 0x%02X%02X\n",
                emulator_strinstruction(opcode), pc.r_rp, pc.r_pc
            );
        }
    switch (opcode)
    {
        case 0:
            return i_nop;
        case 1:
            return i_lra;
        case 2:
            return i_sra;
        case 3:
            return i_call;
        case 4:
            return i_ret;
        case 5:
            return i_jmp;
        case 6:
            return i_jmpp;
        case 7:
            return i_jmpz;
        case 8:
            return i_jmpn;
        case 9:
            return i_jmpc;
        case 10:
            return i_ccf;
        case 11:
            return i_lrp;
        case 12:
            return i_ccp;
        case 13:
            return i_crp;
        case 14:
            return i_push;
        case 15:
            return i_pop;
        case 16:
            return i_and;
        case 17:
            return i_or;
        case 18:
            return i_xor;
        case 19:
            return i_not;
        case 20:
            return i_lsc;
        case 21:
            return i_rsc;
        case 22:
            return i_cmp;
        case 23:
            return i_cmpu;
        /* ... */
        case 32:
            return i_adc;
        case 33:
            return i_sbc;
        case 34:
            return i_inc;
        case 35:
            return i_dec;
        case 36:
            return i_abs;
        case 37:
            return i_mul;
        case 38:
            return i_div;
        case 39:
            return i_mod;
        case 40:
            return i_tse;
        case 41:
            return i_tce;
        case 42:
            return i_add;
        case 43:
            return i_sub;
        case 44:
            return i_rpl;
        case 45:
            return i_mulh;
        /* ... */
        case 48:
            return i_ui;
        case 49:
            return i_uo;
        case 50:
            return i_uoc;
        case 51:
            return i_uocr;
        /* ... */
        case 112:
            return i_prw;
        case 113:
            return i_prr;
        /* ... */
        case 126:
            return i_not_implemented;
        case 127:
            return i_halt;
    }
    return i_not_exist;
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

    printf("%d\n", sizeof(pc) + MEM_SIZE*3 + STACK_SIZE*3 + PORTS);
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
    if (pc.flags & EMULATOR_VERBOSE)
        fputs("Loading the instructions...\n", stderr);

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

        for (u16 i = 0;; ++i)
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
                    (pc.m_rom[i] & I_FLAG) ? "0x%04X %s $0x%02X\n" : "0x%04X %s  0x%02X\n",
                    i,
                    emulator_strinstruction(pc.m_rom[i] & I_OPCODE),
                    (pc.m_rom[i] & I_ARG) >> I_ARG_OFF
                );
            if (i == 65535) break;
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
        instruction = pc.m_rom[pc.r_pc];

        flag   = (instruction & I_FLAG) >> I_FLAG_OFF;
        data   = (instruction & I_ARG) >> I_ARG_OFF;
        opcode = (instruction & I_OPCODE) >> I_OPCODE_OFF;
        bus    = (flag && opcode != 2) ? *get_cache(data) : data;

        select_instruction(opcode)(bus);

        ++pc.r_pc;

        if (pc.flags & EMULATOR_DEBUG)
            fprintf(
                stderr,
                "Instruction: %d|%d|%d(%s)\n"
                "ACC: %02X PC:  %02X%02X\n"
                "CS:  %02X US:  %02X\n"
                "CP:  %02X Bus: %02X\n",
                flag, data, opcode, emulator_strinstruction(opcode),
                pc.r_acc, pc.r_rp, pc.r_pc,
                pc.r_csp, pc.r_usp,
                pc.r_cp, bus 
            );
        if (pc.flags & EMULATOR_PAUSE)
            getchar();
    }
}

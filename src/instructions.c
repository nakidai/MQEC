#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>

#include "emulator.h"
#include "types.h"
#include "luts.h"
#include "die.h"
#include "mq.h"

static u8 _bus;
static mq *_pc;

static void i_nop(void) {}

static void i_lra(void)
{
    _pc->r_acc = _bus;
}

static void i_sra(void)
{
    *get_cache(_bus) = _pc->r_acc;
}

static void i_call(void)
{
    _pc->m_cs[_pc->r_csp] = _pc->r_pc;
    ++_pc->r_csp;
    _pc->r_pc = (_pc->r_rp << 8) + _bus - 1;
}

static void i_ret(void)
{
    --_pc->r_csp;
    _pc->r_pc = _pc->m_cs[_pc->r_csp] - 1;
}

static void i_jmp(void)
{
    _pc->r_pc = (_pc->r_rp << 8) + _bus - 1;
}

static void i_jmpp(void)
{
    if (_pc->r_acc != 0)
        _pc->r_pc = (_pc->r_rp << 8) + _bus - 1;
}

static void i_jmpz(void)
{
    if (_pc->r_acc == 0)
        _pc->r_pc = (_pc->r_rp << 8) + _bus - 1;
}

static void i_jmpn(void)
{
    if (_pc->r_acc & 0b1000000)
        _pc->r_pc = (_pc->r_rp << 8) + _bus - 1;
}

static void i_jmpc(void)
{
    if (_pc->r_flags & F_CARRY)
        _pc->r_pc = (_pc->r_rp << 8) + _bus - 1;
}

static void i_ccf(void)
{
    _pc->r_flags &= ~F_CARRY;
}

static void i_lrp(void)
{
    _pc->r_acc = *get_cache(_bus);
}

static void i_ccp(void)
{
    _pc->r_cp = _bus;
}

static void i_crp(void)
{
    _pc->r_rp = _bus;
}

static void i_push(void)
{
    _pc->m_us[_pc->r_usp] = _pc->r_acc;
    ++_pc->r_usp;
}

static void i_pop(void)
{
    --_pc->r_usp;
    _pc->r_acc = _pc->m_us[_pc->r_usp];
}

static void i_and(void)
{
    _pc->r_acc = _pc->r_acc & _bus;
}

static void i_or(void)
{
    _pc->r_acc = _pc->r_acc | _bus;
}

static void i_xor(void)
{
    _pc->r_acc = _pc->r_acc ^ _bus;
}

static void i_not(void)
{
    _pc->r_acc = ~_pc->r_acc;
}

static void i_lsc(void)
{
    u16 action;

    action = (_pc->r_acc << _bus) + ((_pc->r_flags & F_CARRY) >> F_CARRY_OFF);
    if (action & ~255)
        _pc->r_flags |= F_CARRY;
    _pc->r_acc = action & 255;
}

static void i_rsc(void)
{
    u8 t = _pc->r_acc;
    _pc->r_acc = (_pc->r_acc >> _bus) + (((_pc->r_flags & F_CARRY) >> F_CARRY_OFF) << 7);
    if (((t >> _bus) << _bus) != t)
        _pc->r_flags |= F_CARRY;
}

static void i_cmp(void)
{
    u8 negative = (_pc->r_acc & 0b10000000) & (_bus & 0b10000000);

    if (_pc->r_acc > _bus)
        _pc->r_acc = 1;
    else if (_pc->r_acc == _bus)
        _pc->r_acc = 0;
    else
        _pc->r_acc = 255;

    if (negative)
        _pc->r_acc = ~_pc->r_acc;
}

static void i_cmpu(void)
{
    if (_pc->r_acc > _bus)
        _pc->r_acc = 1;
    else if (_pc->r_acc == _bus)
        _pc->r_acc = 0;
    else
        _pc->r_acc = 255;
}

static void i_adc(void)
{
    u16 action = _pc->r_acc + _bus + ((_pc->r_flags & F_CARRY) >> F_CARRY_OFF);

    if (action & ~255)
        _pc->r_flags |= F_CARRY;
    _pc->r_acc = action & 255;
}

static void i_sbc(void)
{
    u16 action = _pc->r_acc - _bus - ((_pc->r_flags & F_CARRY) >> F_CARRY_OFF);

    if (action & ~255)
        _pc->r_flags |= F_CARRY;
    _pc->r_acc = action & 255;
}

static void i_inc(void)
{
    ++_pc->r_acc;
    if (_pc->r_acc == 0)
        _pc->r_flags |= F_CARRY;
}

static void i_dec(void)
{
    --_pc->r_acc;
    if (_pc->r_acc == 255)
        _pc->r_flags |= F_CARRY;
}

static void i_abs(void)
{
    if (_pc->r_acc & 0b10000000)
        _pc->r_acc = ~_pc->r_acc + 1;
}

static void i_mul(void)
{
    _pc->r_acc *= _bus;
}

static void i_div(void)
{
    _pc->r_acc /= _bus;
}

static void i_mod(void)
{
    _pc->r_acc %= _bus;
}

static void i_tse(void)
{
    _pc->r_acc = emulator_lut_sin(_bus);
}

static void i_tce(void)
{
    _pc->r_acc = emulator_lut_cos(_bus);
}

static void i_add(void)
{
    u16 action = _pc->r_acc + _bus;

    if (action & ~255)
        _pc->r_flags |= F_CARRY;
    _pc->r_acc = action & 255;
}

static void i_sub(void)
{
    u16 action = _pc->r_acc - _bus;

    if (action & ~255)
        _pc->r_flags |= F_CARRY;
    _pc->r_acc = action & 255;
}

static void i_rpl(void)
{
    _pc->r_acc = emulator_lut_rpl(_bus);
}

static void i_mulh(void)
{
    u16 action = _pc->r_acc * _bus;
    _pc->r_acc = (action & 0b1111111100000000) >> 8;
}

static void i_ui(void)
{
    s8 input[8] = {0};
    bool is_number = true;

    if (fgets(input, 8, stdin) == (s8 *)EOF)
        if (errno) error();
    if (input[0] == 0)
    {
        _pc->r_acc = 0;
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
        _pc->r_acc = atoi(input);
    else
        _pc->r_acc = input[0];
}

static void i_uo(void)
{
    printf("%d\n", _pc->r_acc);
}

static void i_uoc(void)
{
    putchar(_pc->r_acc);
}

static void i_uocr(void)
{
    putchar(_pc->r_acc);
    putchar('\n');
}

static void i_prw(void)
{
    _pc->m_ports[_bus] = _pc->r_acc;
}

static void i_prr(void)
{
    _pc->r_acc = _pc->m_ports[_bus];
}

static void i_halt(void)
{
    _pc->r_flags |= F_HALTED;
    emulator_finish();
}

static void i_int(void) {/* TODO: Make module system */}

static instruction select_instruction(u8 opcode)
{
    if (_pc->flags & EMULATOR_WARNING)
    {
        switch (opcode)
        {
        case 48:
        case 49:
        case 50:
        case 51:
            fprintf(
                stderr, "Deprecated %s at 0x%02X%02X\n",
                emulator_strinstruction(opcode), _pc->r_rp, _pc->r_pc
            );
        }

        switch (opcode)
        {
        case 126:
            fprintf(
                stderr, "Not implemented %s at 0x%02X%02X\n",
                emulator_strinstruction(opcode), _pc->r_rp, _pc->r_pc
            );
        }
    }
    switch (opcode)
    {
    case 0:   return i_nop;
    case 1:   return i_lra;
    case 2:   return i_sra;
    case 3:   return i_call;
    case 4:   return i_ret;
    case 5:   return i_jmp;
    case 6:   return i_jmpp;
    case 7:   return i_jmpz;
    case 8:   return i_jmpn;
    case 9:   return i_jmpc;
    case 10:  return i_ccf;
    case 11:  return i_lrp;
    case 12:  return i_ccp;
    case 13:  return i_crp;
    case 14:  return i_push;
    case 15:  return i_pop;
    case 16:  return i_and;
    case 17:  return i_or;
    case 18:  return i_xor;
    case 19:  return i_not;
    case 20:  return i_lsc;
    case 21:  return i_rsc;
    case 22:  return i_cmp;
    case 23:  return i_cmpu;
    /* ... */
    case 32:  return i_adc;
    case 33:  return i_sbc;
    case 34:  return i_inc;
    case 35:  return i_dec;
    case 36:  return i_abs;
    case 37:  return i_mul;
    case 38:  return i_div;
    case 39:  return i_mod;
    case 40:  return i_tse;
    case 41:  return i_tce;
    case 42:  return i_add;
    case 43:  return i_sub;
    case 44:  return i_rpl;
    case 45:  return i_mulh;
    /* ... */
    case 48:  return i_ui;
    case 49:  return i_uo;
    case 50:  return i_uoc;
    case 51:  return i_uocr;
    /* ... */
    case 112: return i_prw;
    case 113: return i_prr;
    /* ... */
    case 126: return i_int;
    case 127: return i_halt;
    default:  die(1, "Non-existent instruction at 0x%02X%02X\n", _pc->r_rp, _pc->r_pc);
    }
}

void emulator_do_tick(mq *pc, u8 opcode, u8 bus)
{
    _pc = pc;
    _bus = bus;

    select_instruction(opcode)();
}

#ifndef __COMMANDS_H__
#define __COMMANDS_H__

#include "types.h"

const s8 *emulator_strinstruction(u8 instruction);
u8 emulator_lut_sin(u8 n);
u8 emulator_lut_cos(u8 n);
u8 emulator_lut_rpl(u8 n);

#endif /* __COMMANDS_H__ */

#ifndef __INSTRUCTIONS_H__
#define __INSTRUCTIONS_H__

#include "emulator.h"
#include "types.h"
#include "mq.h"

void emulator_do_tick(mq *pc, u8 opcode, u8 bus);

#endif /* __INSTRUCTIONS_H__ */

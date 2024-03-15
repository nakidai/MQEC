#ifndef __EMULATOR_H__
#define __EMULATOR_H__

#include "types.h"

#define EMULATOR_VERBOSE 0b00000001

void emulator_init(s8 *filename, u8 flags);
void emulator_free(void);

void emulator_read_instructions(void);
void emulator_run(void);

#endif /* __EMULATOR_H__ */

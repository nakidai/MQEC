#ifndef __EMULATOR_H__
#define __EMULATOR_H__

#include "types.h"

#define EMULATOR_VERBOSE    0b00000001
#define EMULATOR_DEBUG      0b00000010
#define EMULATOR_WARNING    0b00000100
#define EMULATOR_PAUSE      0b00001000

void emulator_init(s8 *filename, u8 flags);
void emulator_free(void);

void emulator_read_instructions(void);
void emulator_run(void);


void emulator_finish(void);

u8 *get_cache(u8 offset);
u16 *get_rom(u8 offset);

#endif /* __EMULATOR_H__ */

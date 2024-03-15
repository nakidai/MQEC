#ifndef __DIE_H__
#define __DIE_H__

#include "types.h"

void die(i32 code, s8 *fmt, ...);
void error(void);

#endif /* __DIE_H__ */

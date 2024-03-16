#ifndef __MQ_H__
#define __MQ_H__

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

typedef void (*instruction)();

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

    /* Memory        Length Description */
    u16 *m_rom;   /* 2^16   ROM         */
    u8  *m_cache; /* 2^16   Cache       */
    u8  *m_us;    /* 2^8    User Stack  */
    u8  *m_cs;    /* 2^8    Call Stack  */
    u8  *m_ports; /* 2^8    I/O Ports   */

    /* Emulator variables    Description*/
    u8          flags;    /* Emulator flags      */
    s8         *filename; /* File being executed */
    u32         ticks;    /* Ticks executed      */
} mq;

#endif /* __MQ_H__ */

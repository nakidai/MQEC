#include "types.h"

static const u8 sins[256] =
{
    128, 131, 135, 139, 143, 147, 151, 155,
    159, 163, 167, 171, 174, 178, 182, 185,
    189, 192, 196, 199, 202, 206, 209, 212,
    215, 218, 220, 223, 226, 228, 231, 233,
    235, 237, 239, 241, 243, 245, 246, 248,
    249, 250, 251, 252, 253, 254, 254, 255,
    255, 255, 255, 255, 255, 255, 255, 254,
    253, 253, 252, 251, 250, 248, 247, 245,
    244, 242, 240, 238, 236, 234, 232, 230,
    227, 225, 222, 219, 216, 213, 210, 207,
    204, 201, 198, 194, 191, 187, 184, 180,
    176, 173, 169, 165, 161, 157, 153, 150,
    146, 142, 138, 134, 130, 126, 122, 118,
    114, 110, 106, 102,  98,  94,  90,  86,
     83,  79,  75,  72,  68,  64,  61,  58,
     54,  51,  48,  45,  42,  39,  36,  33,
     31,  28,  26,  23,  21,  19,  17,  15,
     13,  11,  10,   8,   7,   5,   4,   3,
      2,   2,   1,   0,   0,   0,   0,   0,
      0,   0,   0,   1,   1,   2,   3,   4,
      5,   6,   7,   9,  10,  12,  14,  16,
     18,  20,  22,  24,  27,  29,  32,  34,
     37,  40,  43,  46,  49,  52,  56,  59,
     62,  66,  69,  73,  77,  80,  84,  88,
     92,  96,  99, 103, 107, 111, 115, 119,
    123, 127, 131, 135, 139, 143, 147, 151,
    155, 159, 163, 167, 170, 174, 178, 182,
    185, 189, 192, 196, 199, 202, 205, 209,
    212, 215, 217, 220, 223, 226, 228, 231,
    233, 235, 237, 239, 241, 243, 245, 246,
    248, 249, 250, 251, 252, 253, 254, 254,
    255, 255, 255, 255, 255, 255, 255, 255
};

static const u8 coss[256] =
{
    255, 255, 255, 255, 255, 254, 253, 252,
    252, 250, 249, 248, 247, 245, 243, 242,
    240, 238, 236, 234, 231, 229, 226, 224,
    221, 218, 216, 213, 210, 206, 203, 200,
    197, 193, 190, 186, 183, 179, 175, 172,
    168, 164, 160, 156, 152, 148, 145, 141,
    137, 133, 129, 125, 121, 117, 113, 109,
    105, 101,  97,  93,  89,  85,  82,  78,
     74,  71,  67,  64,  60,  57,  53,  50,
     47,  44,  41,  38,  35,  33,  30,  27,
     25,  23,  20,  18,  16,  14,  12,  11,
      9,   8,   6,   5,   4,   3,   2,   1,
      1,   0,   0,   0,   0,   0,   0,   0,
      0,   1,   1,   2,   3,   4,   5,   6,
      8,   9,  11,  12,  14,  16,  18,  20,
     22,  25,  27,  30,  32,  35,  38,  41,
     44,  47,  50,  53,  57,  60,  63,  67,
     70,  74,  78,  81,  85,  89,  93,  97,
    101, 104, 108, 112, 116, 120, 124, 128,
    132, 136, 140, 144, 148, 152, 156, 160,
    164, 168, 171, 175, 179, 182, 186, 190,
    193, 196, 200, 203, 206, 209, 212, 215,
    218, 221, 224, 226, 229, 231, 233, 236,
    238, 240, 242, 243, 245, 247, 248, 249,
    250, 251, 252, 253, 254, 254, 255, 255,
    255, 255, 255, 255, 255, 255, 254, 253,
    253, 252, 251, 249, 248, 247, 245, 244,
    242, 240, 238, 236, 234, 231, 229, 227,
    224, 221, 219, 216, 213, 210, 207, 203,
    200, 197, 193, 190, 186, 183, 179, 176,
    172, 168, 164, 160, 157, 153, 149, 145,
    141, 137, 133, 129, 125, 121, 117, 113
};

static const u8 rpls[] =
{
    255,   1,   2,   3,   4,   5,   6,   7,
      8,   9,  10,  11,  12,  13,  14,  15,
     16,  17,  18,  19,  20,  21,  22,  23,
     24,  25,  26,  27,  28,  29,  30,  31,
     32,  33,  34,  35,  36,  37,  38,  39,
     40,  41,  42,  43,  44,  45,  46,  47,
     48,  49,  50,  51,  52,  53,  54,  55,
     56,  57,  58,  59,  60,  61,  62,  63,
     64,  65,  66,  67,  68,  69,  70,  71,
     72,  73,  74,  75,  76,  77,  78,  79,
     80,  81,  82,  83,  84,  85,  86,  87,
     88,  89,  90,  91,  92,  93,  94,  95,
     96,  97,  98,  99, 100, 101, 102, 103,
    104, 105, 106, 107, 108, 109, 110, 111,
    112, 113, 114, 115, 116, 117, 118, 119,
    120, 121, 122, 123, 124, 125, 126, 127,
    128, 129, 130, 131, 132, 133, 134, 135,
    136, 137, 138, 139, 140, 141, 142, 143,
    144, 145, 146, 147, 148, 149, 150, 151,
    152, 153, 154, 155, 156, 157, 158, 159,
    160, 161, 162, 163, 164, 165, 166, 167,
    168, 169, 170, 171, 172, 173, 174, 175,
    176, 177, 178, 179, 180, 181, 182, 183,
    184, 185, 186, 187, 188, 189, 190, 191,
    192, 193, 194, 195, 196, 197, 198, 199,
    200, 201, 202, 203, 204, 205, 206, 207,
    208, 209, 210, 211, 212, 213, 214, 215,
    216, 217, 218, 219, 220, 221, 222, 223,
    224, 225, 226, 227, 228, 229, 230, 231,
    232, 233, 234, 235, 236, 237, 238, 239,
    240, 241, 242, 243, 244, 245, 246, 247,
    248, 249, 250, 251, 252, 253, 254, 255
};

const s8 *emulator_strinstruction(u8 instruction)
{
    switch (instruction)
    {
    case 0:   return "NOP ";
    case 1:   return "LRA ";
    case 2:   return "SRA ";
    case 3:   return "CALL";
    case 4:   return "RET ";
    case 5:   return "JMP ";
    case 6:   return "JMPP";
    case 7:   return "JMPZ";
    case 8:   return "JMPN";
    case 9:   return "JMPC";
    case 10:  return "CCF ";
    case 11:  return "LRP ";
    case 12:  return "CCP ";
    case 13:  return "CRP ";
    case 14:  return "PUSH";
    case 15:  return "POP ";
    case 16:  return "AND ";
    case 17:  return "OR  ";
    case 18:  return "XOR ";
    case 19:  return "NOT ";
    case 20:  return "LSC ";
    case 21:  return "RSC ";
    case 22:  return "CMP ";
    case 23:  return "CMPU";
    /* ... */
    case 32:  return "ADC ";
    case 33:  return "SBC ";
    case 34:  return "INC ";
    case 35:  return "DEC ";
    case 36:  return "ABS ";
    case 37:  return "MUL ";
    case 38:  return "DIV ";
    case 39:  return "MOD ";
    case 40:  return "TSE ";
    case 41:  return "TCE ";
    case 42:  return "ADD ";
    case 43:  return "SUB ";
    case 44:  return "RPL ";
    /* ... */
    case 48:  return "UI  ";
    case 49:  return "UO  ";
    case 50:  return "UOC ";
    case 51:  return "UOCR";
    case 52:  return "RPL ";
    /* ... */
    case 112: return "PRW ";
    case 113: return "PRR ";
    case 126: return "INT ";
    case 127: return "HALT";
    }
    return "UNKN";
}

u8 emulator_lut_sin(u8 n)
{
    return sins[n];
}

u8 emulator_lut_cos(u8 n)
{
    return coss[n];
}

u8 emulator_lut_rpl(u8 n)
{
    return rpls[n];
}

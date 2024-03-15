#include "types.h"

const char *instructions[256] =
{
  /*    0       1       2       3       4       5       6       7 */
    "NOP ", "LRA ", "SRA ", "CALL", "RET ", "JMP ", "JMPP", "JMPZ",
  /*    8       9      10      11      12      13      14      15 */
    "JMPN", "JMPC", "CCF ", "LRP ", "CCP ", "CRP ", "PUSH", "POP ",
  /*   16      17      18      19      20      21      22      23 */
    "AND ", "OR  ", "XOR ", "NOT ", "LSC ", "RSC ", "CMP ", "CMPU",
  /*   24      25      26      27      28      29      30      31 */
    "UNKN", "UNKN", "UNKN", "UNKN", "UNKN", "UNKN", "UNKN", "UNKN",
  /*   32      33      34      35      36      37      38      39 */
    "ADC ", "SBC ", "INC ", "DEC ", "ABS ", "MUL ", "DIV ", "MOD ",
  /*   40      41      42      43      44      45      46      47 */
    "TSE ", "TCE ", "ADD ", "SUB ", "RPL ", "UNKN", "UNKN", "UNKN",
  /*   48      49      50      51      52      53      54      55 */
    "UI  ", "UO  ", "UOC ", "UOCR", "RPL ", "UNKN", "UNKN", "UNKN",
  /*   56      57      58      59      60      61      62      63 */
    "UNKN", "UNKN", "UNKN", "UNKN", "UNKN", "UNKN", "UNKN", "UNKN",
  /*   64      65      66      67      68      69      70      71 */
    "UNKN", "UNKN", "UNKN", "UNKN", "UNKN", "UNKN", "UNKN", "UNKN",
  /*   72      73      74      75      76      77      78      79 */
    "UNKN", "UNKN", "UNKN", "UNKN", "UNKN", "UNKN", "UNKN", "UNKN",
  /*   80      81      82      83      84      85      86      87 */
    "UNKN", "UNKN", "UNKN", "UNKN", "UNKN", "UNKN", "UNKN", "UNKN",
  /*   88      89      90      91      92      93      94      95 */
    "UNKN", "UNKN", "UNKN", "UNKN", "UNKN", "UNKN", "UNKN", "UNKN",
  /*   96      97      98      99     100     101     192     193 */
    "UNKN", "UNKN", "UNKN", "UNKN", "UNKN", "UNKN", "UNKN", "UNKN",
  /*  104     105     106     107     108     109     110     111 */
    "UNKN", "UNKN", "UNKN", "UNKN", "UNKN", "UNKN", "UNKN", "UNKN",
  /*  112     113     114     115     116     117     118     119 */
    "PRW ", "PRR ", "UNKN", "UNKN", "UNKN", "UNKN", "UNKN", "UNKN",
  /*  120     121     122     123     124     125     126     127 */
    "UNKN", "UNKN", "UNKN", "UNKN", "UNKN", "UNKN", "INT ", "HALT",
};

const char *strinstruction(u8 instruction)
{
    return instructions[instruction];
}

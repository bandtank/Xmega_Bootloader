/*
 * CCP_Write.c
 *
 * Created: 12/9/2011 10:59:46 PM
 *  Author: Tony
 *
 * CCP_CLK is a routine that allows the clock source to be changed.
 * The architecture requires that the CLK_CTRL register is updated
 * within 4 cycles of writing the unlock value (CCP_SIGN) into the
 * CCP register. After the value passed into the function in r24
 * is written into CLK_CTRL, c functions are capable of modifying 
 * the clock source for the core (e.g. from a cystal to a PLL).
 * 
 * CCP_RST is pretty much the same as the above comment. The core
 * can be reset via software by writing a 1 into the RST_CTRL
 * register. The same constraint applies to this register as the
 * CLK_CTRL register, though. It must be written within 4 cycles 
 * of writing the unlock value into the CCP register. After the sts
 * instruction executes, the stack pointer jumps to the reset
 * vector and the core and all peripherals are initialized the same
 * way as a power-on reset. 
 */ 

#include <avr/io.h> 

#define _SFR_ASM_COMPAT   1 
#define _SFR_OFFSET  0 
#define CCP_SIGN   0xD8 

.global CCP_CLK 
.global CCP_RST

.func CCP_CLK 
    CCP_CLK: 
    ldi  r20, CCP_SIGN 
    out  CCP, r20 
    sts  CLK_CTRL, r24 //1 uint8_t gets passed via r24
ret 

.endfunc 

.func CCP_RST
    CCP_RST: 
    ldi  r20, CCP_SIGN 
    out  CCP, r20 
    ldi  r20, 1
    sts  RST_CTRL, r20
ret 

.endfunc 
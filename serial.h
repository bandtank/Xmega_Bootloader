/*****************************************************************************
*
* Atmel Corporation
*
* File              : serial.h
* Compiler          : IAR C 3.10C Kickstart, AVR-GCC/avr-libc(>= 1.2.5)
* Revision          : $Revision: 1 $
* Date              : $Date: Wednesday, April 22, 2009 $
* Updated by        : $Author: raapeland $
*
* Support mail      : avr@atmel.com
*
* Target platform   : All AVRs with bootloader support
*
* AppNote           : AVR109 - Self-programming
*
* Description       : Header file for serial.c
****************************************************************************/

#include "Xmega_Bootloader.h"

#ifdef INTERFACE
  #if INTERFACE==UART

    /*! \brief Generate UART initialisation section.
     *
     *  \retval None
     */
    void initbootuart( void );
    /*! \brief UART Transmitting section.
     *
     *  \retval None
     */
    void sendchar( unsigned char );
    /*! \brief Generate UART initialisation section.
     *
     *  \retval 8-bit(unsigned char) Received Character
     */
    unsigned char recchar( void );

  #elif INTERFACE==W5500_TCP_RAW
  
    #include "w5500.h"

    #define initbootuart() w5500_init()
    #define sendchar(a) w5500_tcp_sendchar()
    #define recchar(a) w5500_tcp_recchar()

  #else
    #error unsupported INTERFACE
  #endif
#else
  #error INTERFACE not defined
#endif


#ifndef CONFIG_MACROS_H
#define CONFIG_MACROS_H


#define xCAT2(a,b) a##b
#define CAT2(a,b) xCAT2(a,b)

#define xCAT3(a,b,c) a##b##c
#define CAT3(a,b,c) xCAT3(a,b,c)


#define CAT_PORT(a,b,c) xCAT2(a,b)
#define Port(a) CAT_PORT(PORT,a)

#define CAT_PIN_CTRL4a(a,b,c,d) xCAT3(a,c,d)
#define Pin_control(a)  CAT_PIN_CTRL4a(PIN,a,CTRL)

#define xISOLATE_PIN_NUMBER(a, b) b
#define ISOLATE_PIN_NUMBER(a, b) xISOLATE_PIN_NUMBER(a,b)
#define Pin(a)  ISOLATE_PIN_NUMBER(a)

#define Shift_TX_PIN(a) (1 << (a*4 +3))
#define TX_Pin(my_uart)  Shift_TX_PIN(ISOLATE_PIN_NUMBER(my_uart))


#define Make_Uart_Name(uart)  CAT2(USART, uart)
#define Uart(my_uart)  Make_Uart_Name(CAT2(my_uart))



#if  (BAUD_RATE == 9600)
#define  BRREG_VALUE  12
#define  SCALE_VALUE   0
#endif

#if  (BAUD_RATE == 19200)
#define  BRREG_VALUE  11
#define  SCALE_VALUE  -1
#endif

#if (BAUD_RATE == 38400)
#define  BRREG_VALUE   9
#define  SCALE_VALUE  -2
#endif

#if (BAUD_RATE == 57600)
#define  BRREG_VALUE  75
#define  SCALE_VALUE  -6
#endif

#if (BAUD_RATE == 115200)
#define  BRREG_VALUE  11
#define  SCALE_VALUE  -7
#endif


//  This probably only works with GCC
//  It gets values from iox.....h

// SPM control
#define APP_END                   (APP_SECTION_START + APP_SECTION_SIZE)

#if(APP_SECTION_SIZE >= 0x10000)
   #define LARGE_MEMORY
#endif

// EEPROM_definitions
#define EEPROM_BYTES_IN_PAGE      EEPROM_PAGE_SIZE
#define EEPROM_BYTE_ADDRESS_MASK  (EEPROM_PAGE_SIZE -1)

/* definitions for device recognition */
#define  PARTCODE          0xFA          // I guess all Xmegas are the same.  It doesn't seem to matter what code is used.
#define  SIGNATURE_BYTE_1  SIGNATURE_0
#define  SIGNATURE_BYTE_2  SIGNATURE_1
#define  SIGNATURE_BYTE_3  SIGNATURE_2

#endif


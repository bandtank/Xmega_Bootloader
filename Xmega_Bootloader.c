#include "Xmega_Bootloader.h"

#ifdef LARGE_MEMORY
  #define ADDR_T unsigned long
#else  // !LARGE_MEMORY
  #define ADDR_T unsigned int
#endif // LARGE_MEMORY

#ifndef REMOVE_BLOCK_SUPPORT
unsigned char BlockLoad(unsigned int size, unsigned char mem, ADDR_T *address);
void BlockRead(unsigned int size, unsigned char mem, ADDR_T *address);

// BLOCKSIZE should be chosen so that the following holds: BLOCKSIZE*n = APP_PAGE_SIZE,  where n=1,2,3...
#define BLOCKSIZE APP_PAGE_SIZE

#endif // REMOVE_BLOCK_SUPPORT

#ifdef __ICCAVR__
  #define C_TASK __C_task
#else // ! __ICCAVR__
  #define C_TASK /**/
#endif // __ICCAVR__

int main(void)
{
    void (*funcptr)( void ) = 0x0000; // Set up function pointer to RESET vector.
    Port(ENTER_BOOTLOADER_PIN).Pin_control(ENTER_BOOTLOADER_PIN) = PORT_OPC_PULLUP_gc;

    //This delay allows the pull-up resistor sufficient time to pull high.
    //Realistically it only needs to be ~1uS, so waiting 5 cycles @ 2MHz
    //will be a 2.5uS delay.
    __builtin_avr_delay_cycles(5);

    /* Branch to bootloader or application code? */
#if (BOOTLOADER_PIN_EN == 0)
    //Active low pin
    if( !(Port(ENTER_BOOTLOADER_PIN).IN & (1<<Pin(ENTER_BOOTLOADER_PIN))) ) {
#elif (BOOTLOADER_PIN_EN == 1)
    //Active high pin
    if(  (Port(ENTER_BOOTLOADER_PIN).IN & (1<<Pin(ENTER_BOOTLOADER_PIN))) ) {
#else
    #error Invalid value for BOOTLOADER_PIN_EN
#endif
        // Check to see if the SPM lock bit is still set. Reset to clear the lock
        // bit while hopefully still hitting the bootloader entry condition.
        if(NVM.CTRLB & NVM_SPMLOCK_bm) {
            CCP_RST();
        }

        ADDR_T address = 0;
        unsigned int temp_int = 0;
        unsigned char val = 0;

        EEPROM_FlushBuffer();
        EEPROM_DisableMapping();

        Port(LED_PIN).DIRSET = (1 << Pin(LED_PIN));           // Set the pin direction
    #if (LED_ON == 1)
        Port(LED_PIN).OUTSET = (1 << Pin(LED_PIN));           // Turn on the LED
    #elif (LED_ON == 0)

    #else
        #error Invalid value for LED_ON
    #endif

        initbootuart();                                       // Initialize UART.

        for(;;) {
            val = recchar();                                  // Wait for command character.

            if(val == COMMAND_CHECK_AUTOINC)                  // Check autoincrement status.
                sendchar(RESPONSE_YES);                       // Yes, we do autoincrement.

            else if(val == COMMAND_SET_ADDRESS) {             // Set address (words, not bytes)
                address = (recchar() << 8) | recchar();
                sendchar(RESPONSE_OKAY);                      // Send OK back.
            }

            else if(val == COMMAND_CHIP_ERASE) {              // Chip erase.
                for(address = 0; address < APP_END; address += APP_PAGE_SIZE) {
                    SP_WaitForSPM();
#ifdef __ICCAVR__
#pragma diag_suppress=Pe1053 // Suppress warning for conversion from long-type address to flash ptr.
#endif
                    SP_EraseApplicationPage( address ); // Byte address, not word address
#ifdef __ICCAVR__
#pragma diag_default=Pe1053  // Back to default.
#endif
                }

                EEPROM_LoadPage(&val);                        // Write random values to the page buffer
                EEPROM_EraseAll();                            // Erasing all pages in the EEPROM
                sendchar(RESPONSE_OKAY);                      // Send OK
            }

#ifndef REMOVE_BLOCK_SUPPORT

            else if(val == COMMAND_CHECK_BLOCKLOAD) {         // Check block load support.
                sendchar(RESPONSE_YES);                       // Report block load supported.
                sendchar((BLOCKSIZE>>8) & 0xFF);              // MSB first.
                sendchar(BLOCKSIZE&0xFF);                     // Report BLOCKSIZE (bytes).
            }

            else if(val == COMMAND_START_BLOCKLOAD) {         // Start block load.
                temp_int = (recchar()<<8) | recchar();        // Get block size.
                val = recchar();                              // Get memtype
                sendchar( BlockLoad(temp_int,val,&address) ); // Block load.
            }

            else if(val == COMMAND_START_BLOCKREAD) {         // Start block read.
                temp_int = (recchar()<<8) | recchar();        // Get block size.
                val = recchar();                              // Get memtype
                BlockRead(temp_int, val, &address);           // Block read
            }
#endif // REMOVE_BLOCK_SUPPORT

#ifndef REMOVE_FLASH_BYTE_SUPPORT

            else if(val == COMMAND_READ_PROGMEM) {            // Read program memory
                SP_WaitForSPM();
#ifdef __ICCAVR__
#pragma diag_suppress=Pe1053 // Suppress warning for conversion from long-type address to flash ptr.
#endif
                sendchar( SP_ReadByte( (address << 1)+1) );   // Send high byte
                sendchar( SP_ReadByte( (address << 1)+0) );   // Send low byte
#ifdef __ICCAVR__
#pragma diag_default=Pe1053 // Back to default.
#endif
                address++;                                    // Auto-advance to next Flash word
            }

            else if(val == COMMAND_WRITE_PROGMEML) {          // Write program memory, low byte
                // NOTE: Always use this command before sending high byte.
                temp_int=recchar();                           // Get low byte for later SP_LoadFlashWord
                sendchar(RESPONSE_OKAY);                      // Send OK back.
            }

            else if(val == COMMAND_WRITE_PROGMEMH) {          // Write program memory, high byte.
                temp_int |= (recchar()<<8);                   // Get and insert high byte.
                SP_WaitForSPM();
#ifdef __ICCAVR__
#pragma diag_suppress=Pe1053 // Suppress warning for conversion from long-type address to flash ptr.
#endif
                SP_LoadFlashWord( (address << 1), temp_int );
#ifdef __ICCAVR__
#pragma diag_default=Pe1053 // Back to default.
#endif
                address++;                                    // Auto-advance to next Flash word.
                sendchar(RESPONSE_OKAY);                      // Send OK back.
            }

            else if(val== COMMAND_WRITE_PAGE) {               // Write page
                if( address >= (APP_END>>1) )                 // Protect bootloader area.
                    sendchar(RESPONSE_UNKNOWN);
                else {
                    SP_WaitForSPM();
#ifdef __ICCAVR__
#pragma diag_suppress=Pe1053 // Suppress warning for conversion from long-type address to flash ptr.
#endif
                    SP_WriteApplicationPage( address << 1);   // Convert byte address and write
#ifdef __ICCAVR__
#pragma diag_default=Pe1053 // Back to default.
#endif
                    sendchar(RESPONSE_OKAY);                  // Send OK back
                }
#endif // REMOVE_FLASH_BYTE_SUPPORT

#ifndef REMOVE_EEPROM_BYTE_SUPPORT
            }

            else if (val == COMMAND_WRITE_EEPROMBYTE){        // Write EEPROM memory
                EEPROM_WriteByte( (unsigned char)(address / EEPROM_BYTES_IN_PAGE) ,
                                  (unsigned char)(address & EEPROM_BYTE_ADDRESS_MASK), recchar() );
                address++;                                    // Select next EEPROM byte
            }

            else if (val == COMMAND_READ_EEPROMBYTE) {        // Read EEPROM memory
                sendchar( EEPROM_ReadByte( (unsigned char)(address / EEPROM_BYTES_IN_PAGE),
                                           (unsigned char)(address & EEPROM_BYTE_ADDRESS_MASK) ) );
                address++;                                    // Select next EEPROM byte
             }

#endif // REMOVE_EEPROM_BYTE_SUPPORT

#ifndef REMOVE_FUSE_AND_LOCK_BIT_SUPPORT

            else if(val == COMMAND_WRITE_LOCKBITS) {          // Write lockbits
                SP_WaitForSPM();                              // Wait for NVM to finish.
                SP_WriteLockBits( recchar() );                // Read and set lock bits.
                sendchar(RESPONSE_OKAY);                      // Send OK back.
            }
#if defined(_GET_LOCK_BITS)

            else if(val == COMMAND_READ_LOCKBITS) {           // Read lock bits
                SP_WaitForSPM();
                sendchar( SP_ReadLockBits() );                // Send lock bits
            }

            else if(val == COMMAND_READ_FUSEL) {              // Read low fuse bits
                SP_WaitForSPM();
                sendchar(SP_ReadFuseByte(0));                 // Send low fuse bits
            }

            else if(val == COMMAND_READ_FUSEH) {              // Read high fuse bits
                SP_WaitForSPM();
                sendchar(SP_ReadFuseByte(1));                 // Send high fuse bits
            }

            else if(val == COMMAND_READ_FUSEE) {              // Read extended fuse bits
                SP_WaitForSPM();
                sendchar(SP_ReadFuseByte(2));                 // Send extended fuse bits
            }
#endif // defined(_GET_LOCK_BITS)
#endif // REMOVE_FUSE_AND_LOCK_BIT_SUPPORT

#ifndef REMOVE_AVRPROG_SUPPORT

            else if((val == COMMAND_PROGMODE_ENTLV0) ||
			        (val == COMMAND_PROGMODE_ENTLV1) )        // Enter and leave programming mode
                sendchar(RESPONSE_OKAY);                      // Just answer OK


            else if(val == COMMAND_EXIT_BOOTLOADER) {         // Exit bootloader
                Uart(MY_UART).STATUS = (1 << USART_TXCIF_bp); // Clear flag
                sendchar(RESPONSE_OKAY);                      // Answer OK
                while (!(Uart(MY_UART).STATUS & (1 << USART_TXCIF_bp)));
                SP_WaitForSPM();
                CCP_RST();                                    // Reset
            }

            else if (val == COMMAND_READ_PROGTYPE)            // Get programmer type
                sendchar('S');                                // Answer SERIAL

            else if(val == COMMAND_READ_DEVCODES) {           // Return supported device codes
#if PARTCODE+0 > 0
                 sendchar( PARTCODE );                        // Supports only this device, of course.
#endif
                 sendchar( 0 );                               // Send list terminator.
            }

            else if(val == COMMAND_LED_ON) {                  // Turn on LED
            #if (LED_ON == LOW)
                Port(LED_PIN).OUTCLR = (1 << Pin(LED_PIN));
            #else
                Port(LED_PIN).OUTSET = (1 << Pin(LED_PIN));
            #endif
                sendchar(RESPONSE_OKAY);                      // Send OK back
            }

            else if(val == COMMAND_LED_OFF) {                 // Turn off LED
            #if (LED_ON == LOW)
                Port(LED_PIN).OUTSET = (1 << Pin(LED_PIN));
            #else
                Port(LED_PIN).OUTCLR = (1 << Pin(LED_PIN));
            #endif
                sendchar(RESPONSE_OKAY);                      // Send OK back.
            }

            else if(val == COMMAND_SET_DEVTYPE) {             // Set device type
                recchar();                                    // Unimplemented right now
                sendchar(RESPONSE_OKAY);                      // Send OK back.
            }

#endif // REMOVE_AVRPROG_SUPPORT

            else if(val == COMMAND_READ_BOOTLOADNAME) {       // Return programmer identifier
                sendchar('X');                                // Must be 7 characters
                sendchar('m');
                sendchar('e');
                sendchar('g');
                sendchar('a');
                sendchar('B');
                sendchar('l');
            }

            else if(val == COMMAND_READ_BOOTLOADVER) {        // Return software version
                sendchar('1');
                sendchar('1');
            }

            else if(val == COMMAND_READ_SIGBYTES) {           // Return signature bytes
                sendchar( SIGNATURE_BYTE_3 );
                sendchar( SIGNATURE_BYTE_2 );
                sendchar( SIGNATURE_BYTE_1 );
            }

            else if(val != COMMAND_ESCAPE)                    // If not ESC, then it's unrecognized
               sendchar(RESPONSE_UNKNOWN);
        } // end: for(;;)
    } else {
        SP_WaitForSPM();
        SP_LockSPM();                                         // Lock SPM
        EIND = 0x00;
        funcptr();                                            // Jump to application section.
    }
} // end: main

#ifndef REMOVE_BLOCK_SUPPORT

unsigned char BlockLoad(unsigned int size, unsigned char mem, ADDR_T *address) {
    unsigned int data;
    ADDR_T tempaddress;

    if(mem=='E') {                                            // EEPROM memory type
        unsigned char pageAddr, byteAddr, value;
        unsigned char buffer[BLOCKSIZE];

        EEPROM_FlushBuffer();
        // disable mapping of EEPROM into data space (enable IO mapped access)
        EEPROM_DisableMapping();

        // Fill buffer first, as EEPROM is too slow to copy with UART speed
        for(tempaddress=0;tempaddress<size;tempaddress++) {
            buffer[tempaddress] = recchar();
        }
        // Then program the EEPROM
        for( tempaddress=0; tempaddress < size; tempaddress++) {
            pageAddr = (unsigned char)( (*address) / EEPROM_BYTES_IN_PAGE);
            byteAddr = (unsigned char)( (*address) & EEPROM_BYTE_ADDRESS_MASK);
            value = buffer[tempaddress];

            EEPROM_WriteByte(pageAddr, byteAddr, value);

            (*address)++;                                     // Select next EEPROM byte
        }
        return '\r';                                          // Report programming OK
    }

    else if(mem=='F') {                                       // Flash memory type
        // NOTE: For flash programming, 'address' is given in words.
        (*address) <<= 1;                                     // Convert address to bytes temporarily.
        tempaddress = (*address);                             // Store address in page.

        do {
            data = recchar();
            data |= (recchar() << 8);
#ifdef __ICCAVR__
#pragma diag_suppress=Pe1053 // Suppress warning for conversion from long-type address to flash ptr.
#endif
            SP_LoadFlashWord(*address, data);
#ifdef __ICCAVR__
#pragma diag_default=Pe1053 // Back to default.
#endif
            (*address)+=2;                                    // Select next word in memory.
            size -= 2;                                        // Reduce number of bytes to write by two.
        } while(size);                                        // Loop until all bytes written.

#ifdef __ICCAVR__
#pragma diag_suppress=Pe1053 // Suppress warning for conversion from long-type address to flash ptr.
#endif
        SP_WriteApplicationPage(tempaddress);

#ifdef __ICCAVR__
#pragma diag_default=Pe1053 // Back to default.
#endif
        SP_WaitForSPM();
        (*address) >>= 1;                                     // Convert address back to Flash words again.
        return '\r';                                          // Report programming OK
    }

    else                                                      // Invalid memory type?
        return '?';
}

void BlockRead(unsigned int size, unsigned char mem, ADDR_T *address) {
    // EEPROM memory type.

    if (mem=='E') {                                           // Read EEPROM
        unsigned char byteAddr, pageAddr;
        EEPROM_DisableMapping();
        EEPROM_FlushBuffer();

        do {
            pageAddr = (unsigned char)(*address / EEPROM_BYTES_IN_PAGE);
            byteAddr = (unsigned char)(*address & EEPROM_BYTE_ADDRESS_MASK);

            sendchar( EEPROM_ReadByte( pageAddr, byteAddr ) );
            (*address)++;                                     // Select next EEPROM byte
            size--;                                           // Decrease number of bytes to read
        } while (size);                                       // Repeat until all block has been read
    }


    else if(mem=='F') {                                       // Flash memory type
        (*address) <<= 1;                                     // Convert address to bytes temporarily.

        do {
#ifdef __ICCAVR__
#pragma diag_suppress=Pe1053 // Suppress warning for conversion from long-type address to flash ptr.
#endif
          sendchar( SP_ReadByte( *address) );
          sendchar( SP_ReadByte( (*address)+1) );
#ifdef __ICCAVR__
#pragma diag_default=Pe1053                                   // Back to default.
#endif
            (*address) += 2;                                  // Select next word in memory.
            size -= 2;                                        // Subtract two bytes from number of bytes to read
        } while (size);                                       // Repeat until all block has been read

        (*address) >>= 1;                                     // Convert address back to Flash words again.
    }
}

#endif // REMOVE_BLOCK_SUPPORT

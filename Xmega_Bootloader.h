#ifndef XMEGA_BOOTLOADER_H
#define XMEGA_BOOTLOADER_H

////////////////////////////////
/*          COMMANDS          */
////////////////////////////////
// 'a'     = Check auto-increment status
// 'A'     = Set address, two parameters: <high byte>, <low byte>
// 'e'     = Erase Application Section and EEPROM
// 'b'     = Check block load support, returns BLOCKSIZE (2 bytes)
// 'B'     = Start block load, three parameters: block size (<high byte>,<low byte>),memtype
// 'g'     = Start block read, three parameters: block size (<high byte>,<low byte>),memtype
// 'R'     = Read program memory, returns high byte then low byte of flash word
// 'c'     = Write program memory, one parameter: low byte, returns '\r'
// 'C'     = Write program memory, one parameter: high byte, returns '\r'
// 'm'     = Write page, returns '?' if page is protected, returns '\r' if done
// 'D'     = Write EEPROM, one parameter: byte to write
// 'd'     = Read EEPROM, returns one byte
// 'l'     = Write lock bits, returns '\r'
// 'r'     = Read lock bits
// 'F'     = Read low fuse bits
// 'N'     = Read high fuse bits
// 'Q'     = Read extended fuse bits
// 'P'     = Enter and leave programming mode, returns '\r'
// 'L'     = Enter and leave programming mode, returns '\r'
// 'E'     = Exit bootloader, returns '\r', jumps to 0x0000
// 'p'     = Get programmer type, returns 'S'
// 't'     = Return supported device codes, returns PARTCODE and 0
// 'x'     = Turn on LED0, returns '\r'
// 'y'     = Turn off LED0, returns '\r'
// 'T'     = Set device type, one parameter: device byte, returns '\r'
// 'S'     = Returns Xmega_Bootloader
// 'V'     = Returns version number
// 's'     = Return signature bytes, returns 3 bytes (sig3, sig2, sig1)
// 0x1b    = ESC
// Unknown = '?'

#include <avr/io.h>
#include "config_macros.h"
#include "serial.h"
#include "eeprom_driver.h"
#include "sp_driver.h"

#define COMMAND_CHECK_AUTOINC      'a'
#define COMMAND_SET_ADDRESS        'A'
#define COMMAND_CHIP_ERASE         'e'
#define COMMAND_CHECK_BLOCKLOAD    'b'
#define COMMAND_START_BLOCKLOAD    'B'
#define COMMAND_START_BLOCKREAD    'g'
#define COMMAND_READ_PROGMEM       'R'
#define COMMAND_WRITE_PROGMEML     'c'
#define COMMAND_WRITE_PROGMEMH     'C'
#define COMMAND_WRITE_PAGE         'm'
#define COMMAND_WRITE_EEPROMBYTE   'D'
#define COMMAND_READ_EEPROMBYTE    'd'
#define COMMAND_WRITE_LOCKBITS     'l'
#define COMMAND_READ_LOCKBITS      'r'
#define COMMAND_READ_FUSEL         'F'
#define COMMAND_READ_FUSEH         'N'
#define COMMAND_READ_FUSEE         'Q'
#define COMMAND_PROGMODE_ENTLV0    'P'
#define COMMAND_PROGMODE_ENTLV1    'L'
#define COMMAND_EXIT_BOOTLOADER    'E'
#define COMMAND_READ_PROGTYPE      'p'
#define COMMAND_READ_DEVCODES      't'
#define COMMAND_LED_ON             'x'
#define COMMAND_LED_OFF            'y'
#define COMMAND_SET_DEVTYPE        'T'
#define COMMAND_READ_BOOTLOADNAME  'S'
#define COMMAND_READ_BOOTLOADVER   'V'
#define COMMAND_READ_SIGBYTES      's'
#define COMMAND_ESCAPE             0x1b
#define COMMAND_UNKNOWN            '?'

#define RESPONSE_OKAY              '\r'
#define RESPONSE_YES               'Y'
#define RESPONSE_NO                'N'
#define RESPONSE_UNKNOWN           '?'

extern void CCP_RST( void );

#endif
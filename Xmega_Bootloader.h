#ifndef XMEGA_BOOTLOADER_H
#define XMEGA_BOOTLOADER_H

#include <avr/io.h>
#include "config_macros.h"
#include "serial.h"
#include "eeprom_driver.h"
#include "sp_driver.h"

extern void CCP_RST( void );

#endif
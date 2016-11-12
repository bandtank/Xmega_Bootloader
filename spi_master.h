#ifndef SPI_MASTER_H_
#define SPI_MASTER_H_

#include <avr/io.h>

void spi_master_init(void);
uint8_t spi_master_byte(uint8_t data);

#define spi_master_deselect() Port_Out_Set(SPI_SS_PIN);
#define spi_master_select() Port_Out_Clr(SPI_SS_PIN);

//----------------------------------------------------------------------

#endif

//----------------------------------------------------------------------
// end of file
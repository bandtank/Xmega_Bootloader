#include <avr/io.h>

#include "config_macros.h"

void spi_master_init(void)
{
    // output pins
    Port_Out_Set(SPI_SS_PIN);               // SS initially high
    Port_Dir_Set(SPI_SS_PIN);               // SS is output
    CAT2(PORT,MY_SPI).DIRSET = (1 << 5);    // MOSI is output
    CAT2(PORT,MY_SPI).DIRSET = (1 << 7);    // SCLK is output

    // SPI enable master, mode 0, CLKper/2 (up to 16MHz)
    CAT2(SPI,MY_SPI).CTRL = SPI_CLK2X_bm | SPI_ENABLE_bm | SPI_MASTER_bm;
    
    return;
}

uint8_t spi_master_byte(uint8_t data)
{
    CAT2(SPI,MY_SPI).DATA = data;
    while (!(CAT2(SPI,MY_SPI).STATUS & SPI_IF_bm));
    return CAT2(SPI,MY_SPI).DATA;
}

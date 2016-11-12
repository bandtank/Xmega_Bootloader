// simple W5500 driver
// with support for encapsulation of serial traffic over TCP

#include <stdarg.h>

#include "config_macros.h"
#include "spi_master.h"
#include "w5500.h"

inline void w5500_rw_frame_start(uint8_t block, uint16_t addr, uint8_t w_r)
{
	spi_master_select();
	spi_master_byte(addr >> 8);
	spi_master_byte(addr & 0xFF);
	spi_master_byte(block << 3 | w_r << 2);
}

inline void w5500_rw_frame_end(void)
{
	spi_master_deselect();
}

void w5500_write_8(uint8_t block, uint16_t addr, uint8_t data)
{
	w5500_rw_frame_start(block, addr, W5500_WRITE);
	spi_master_byte(data);
	w5500_rw_frame_end();
}

void w5500_write_8_mult(uint8_t block, uint16_t addr, uint8_t count, ...)
{
	va_list args;
	uint8_t i;

	if (count) {
		w5500_rw_frame_start(block, addr, W5500_WRITE);
		for (i = 0; i < count; i++) {
			spi_master_byte((uint8_t)va_arg(args, int));
		}
		w5500_rw_frame_end();
		va_end(args);
	}
}

void w5500_write_16(uint8_t block, uint16_t addr, uint16_t data)
{
	w5500_rw_frame_start(block, addr, W5500_WRITE);
	spi_master_byte(data >> 8);
	spi_master_byte(data & 0xFf);
	w5500_rw_frame_end();
}

uint8_t w5500_read_8(uint8_t block, uint16_t addr)
{
	uint8_t r;
	
	w5500_rw_frame_start(block, addr, W5500_READ);
	r = spi_master_byte(0);
	w5500_rw_frame_end();
	return r;
}

uint16_t w5500_read_16(uint8_t block, uint16_t addr)
{
	uint16_t r;
	
	w5500_rw_frame_start(block, addr, W5500_READ);
	r = spi_master_byte(0) << 8;
	r |= spi_master_byte(0);
	w5500_rw_frame_end();
	return r;
}


uint16_t w5500_read_16_check(uint8_t block, uint16_t addr)
{
	uint16_t r1 = 0, r2 = 1;
	
	r2 = w5500_read_16(block, addr);
	r1 = ~r2;
	// repeat until result is stable
	while (r1 != r2) {
		r1 = r2;
		r2 = w5500_read_16(block, addr);
	}
	return r1;
}

uint8_t w5500_tcp_maintain(void)
{
	uint8_t s;

	while(1) {
		s = w5500_read_8(W5500_BLK_S0REG, W5500_SR_SR);
		if ((s != W5500_SR_SR_LISTEN) || (s != W5500_SR_SR_SYNRECV) || (s != W5500_SR_SR_ESTABLISHED))
			break;
		if (s == W5500_SR_SR_CLOSED) {
			w5500_write_8(W5500_BLK_S0REG, W5500_SR_CR, W5500_SR_CR_OPEN);				// command - open
			if (w5500_read_8(W5500_BLK_S0REG, W5500_SR_SR) == W5500_SR_SR_INIT) {
				w5500_write_8(W5500_BLK_S0REG, W5500_SR_CR, W5500_SR_CR_LISTEN);		// command - listen
				if (w5500_read_8(W5500_BLK_S0REG, W5500_SR_SR) == W5500_SR_SR_LISTEN)
					break;
			}
			w5500_write_8(W5500_BLK_S0REG, W5500_SR_CR, W5500_SR_CR_CLOSE);				// command - close
		}
	}
}

void w5500_tcp_sendchar(unsigned char c)
{
	static uint16_t tx_write_addr = 0;
	uint16_t fs;
	
	w5500_tcp_maintain();
	while(1) {
		fs =  w5500_read_16_check(W5500_BLK_S0REG, W5500_SR_TX_FSR);
		if (fs) {
			w5500_write_8(W5500_BLK_S0TXBUF, tx_write_addr++, c);					// write character
			w5500_write_16(W5500_BLK_S0REG, W5500_SR_TX_WR, tx_write_addr);			// update TX write pointer
			w5500_write_8(W5500_BLK_S0REG, W5500_SR_CR, W5500_SR_CR_SEND);	// send
			break;
		}
	}
}

// wait for received character
unsigned char w5500_tcp_recchar()
{
	unsigned char r;
	static uint16_t rx_read_addr = 0;
	static uint16_t len = 0;
	
	if (!len) { // no data already in buffer
		while(1) {	// wait until there is some new data in buffer
			len = w5500_read_16_check(W5500_BLK_S0REG, W5500_SR_RX_RSR);
			if (!len) // no data?
			w5500_tcp_maintain();
			else
			break;
		}
	}
	
	r = (unsigned char)w5500_read_8(W5500_BLK_S0RXBUF, rx_read_addr++);	// get next received byte
	if (!--len) { // all packet received?
		w5500_write_16(W5500_BLK_S0REG, W5500_SR_RX_RD, rx_read_addr);	// update RX read pointer
		w5500_write_8(W5500_BLK_S0REG, W5500_SR_CR, W5500_SR_CR_RECV);	// command - receive
	}
	
	return r;
}

uint8_t w5500_init(void)
{
	Port(W5500_INT_PIN).Pin_control(W5500_INT_PIN) = PORT_OPC_PULLUP_gc;
	spi_master_init();
	
	if (w5500_read_8(W5500_BLK_CREG, W5500_CR_VERSIONR) == 0x04) {
		
		// soft reset, wait for completion
		w5500_write_8(W5500_BLK_CREG, W5500_CR_MR, W5500_CR_MR_RST);
		while(w5500_read_8(W5500_BLK_CREG, W5500_CR_MR) & W5500_CR_MR_RST)
			;

		// set MAC address

#ifdef MAC_ADDRESS_LOCATION
  #if MAC_ADDRESS_LOCATION == FLASH
		// to do
  #elif MAC_ADDRESS_LOCATION == EEPROM
		// to do
  #elif MAC_ADDRESS_LOCATION == I2C
		// to do
  #else
    #error MAC_ADDRESS_LOCATION is unsupported 
  #endif
#else
  #error MAC_ADDRESS_LOCATION not specified
#endif
		
		w5500_write_8_mult(W5500_BLK_CREG, W5500_CR_SHAR, 6, 0x01, 0x02, 0xBE, 0xEF, 0xAB, 0xCD);	// set MAC address
		w5500_write_8_mult(W5500_BLK_CREG, W5500_CR_GAR, 4, 192, 168, 1, 1);						// set gateway address		
		w5500_write_8_mult(W5500_BLK_CREG, W5500_CR_SIPR, 4, 192, 168, 1, 6);						// set IP address
		w5500_write_8_mult(W5500_BLK_CREG, W5500_CR_SUBR, 4, 255, 255, 255, 0);						// set subnet mask
	
		// enable full autoconfig; reset PHY
		w5500_write_8(W5500_BLK_CREG, W5500_CR_PHYCFGR, W5500_CR_PHYCFGR_OPMD | W5500_CR_PHYCFGR_OPMDC7);
		w5500_write_8(W5500_BLK_CREG, W5500_CR_PHYCFGR, W5500_CR_PHYCFGR_NRST | W5500_CR_PHYCFGR_OPMD | W5500_CR_PHYCFGR_OPMDC7);

		// initialise for TCP comms on socket 0
		w5500_write_16(W5500_BLK_S0REG, W5500_SR_PORT, TCP_PORT);			// set port number
		w5500_write_8(W5500_BLK_S0REG, W5500_SR_MR, W5500_SR_MR_PROTO_TCP);	// set TCP mode

		// open TCP socket
		w5500_tcp_maintain();
	
		return 0;
	}
	else
		return 1; // fail
}


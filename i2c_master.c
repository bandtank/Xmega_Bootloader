#include <avr/io.h>

void int_i2c_init(void)
{   
	// baud rate = 400kHz @ 32MHz
	TWIC.MASTER.BAUD = 35;		

	// enable
	TWIC.MASTER.CTRLA |= TWI_MASTER_ENABLE_bm;
	
	// force idle
	TWIC.MASTER.STATUS = (TWIC.MASTER.STATUS & ~TWI_MASTER_BUSSTATE_gm) | TWI_MASTER_BUSSTATE_IDLE_gc;	
}

void int_i2c_write_subaddr(uint8_t addr, uint8_t subaddr, uint8_t data)
{
	TWIC.MASTER.ADDR = addr << 1;
	while (!(TWIC.MASTER.STATUS & TWI_MASTER_WIF_bm) || (TWIC.MASTER.STATUS & TWI_SLAVE_RXACK_bm));
	TWIC.MASTER.DATA = subaddr;
	while (!(TWIC.MASTER.STATUS & TWI_MASTER_WIF_bm) || (TWIC.MASTER.STATUS & TWI_SLAVE_RXACK_bm));
	TWIC.MASTER.DATA = data;
	while (!(TWIC.MASTER.STATUS & TWI_MASTER_WIF_bm) || (TWIC.MASTER.STATUS & TWI_SLAVE_RXACK_bm));
	TWIC.MASTER.CTRLC = TWI_MASTER_CMD_STOP_gc;
}

uint8_t int_i2c_read_subaddr(uint8_t addr, uint8_t subaddr)
{
	uint8_t r;
	
	TWIC.MASTER.ADDR = addr << 1;
	while (!(TWIC.MASTER.STATUS & TWI_MASTER_WIF_bm) || (TWIC.MASTER.STATUS & TWI_SLAVE_RXACK_bm));
	TWIC.MASTER.DATA = subaddr;
	while (!(TWIC.MASTER.STATUS & TWI_MASTER_WIF_bm) || (TWIC.MASTER.STATUS & TWI_SLAVE_RXACK_bm));
	TWIC.MASTER.ADDR = (addr << 1) | 0x01;
	while (!(TWIC.MASTER.STATUS & TWI_MASTER_RIF_bm));
	r = TWIC.MASTER.DATA;
	TWIC.MASTER.CTRLC = TWI_MASTER_ACKACT_bm | TWI_MASTER_CMD_STOP_gc;	
	return r;
}

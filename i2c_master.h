//----------------------------------------------------------------------
// i2c.h
//----------------------------------------------------------------------

#ifndef I2C_H_
#define I2C_H_

void int_i2c_init(void);
void int_i2c_write_subaddr(uint8_t addr, uint8_t subaddr, uint8_t data);
uint8_t int_i2c_read_subaddr(uint8_t addr, uint8_t subaddr);

void ext_i2c_init(void);
void ext_i2c_write_subaddr(uint8_t addr, uint8_t subaddr, uint8_t data);
uint8_t ext_i2c_read_subaddr(uint8_t addr, uint8_t subaddr);

#endif

//----------------------------------------------------------------------
// end of file
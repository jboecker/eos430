/*
 * i2c_master.h
 *
 *  Created on: 02.11.2012
 *      Author: Jan
 */

#ifndef I2C_MASTER_H_
#define I2C_MASTER_H_

void i2c_transmit(unsigned char slave_address, unsigned char* buffer, unsigned int num_bytes);
void i2c_receive(unsigned char slave_address, unsigned char* buffer, unsigned int num_bytes);
unsigned char i2c_get_nackflag();

void uart_tx(unsigned char byte);

#endif /* I2C_MASTER_H_ */

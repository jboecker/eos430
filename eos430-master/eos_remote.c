/*
 * eos_remote.c
 *
 *  Created on: 22.11.2012
 *      Author: Jan
 */

#include <msp430g2553.h>

#include "eos_remote.h"
#include "eos_dispatcher.h"
#include "comms.h"


unsigned char TI_USCI_I2C_slave_present(unsigned char slave_address){
  unsigned char ie2_bak, slaveadr_bak, ucb0i2cie, returnValue;
  ucb0i2cie = UCB0I2CIE;                      // restore old UCB0I2CIE
  ie2_bak = IE2;                              // store IE2 register
  slaveadr_bak = UCB0I2CSA;                   // store old slave address
  UCB0I2CIE &= ~ UCNACKIE;                    // no NACK interrupt
  UCB0I2CSA = slave_address;                  // set slave address
  IE2 &= ~(UCB0TXIE + UCB0RXIE);              // no RX or TX interrupts
  __disable_interrupt();
  UCB0CTL1 |= UCTR + UCTXSTT + UCTXSTP;       // I2C TX, start condition
  while (UCB0CTL1 & UCTXSTP);                 // wait for STOP condition

  returnValue = !(UCB0STAT & UCNACKIFG);
  __enable_interrupt();
  IE2 = ie2_bak;                              // restore IE2
  UCB0I2CSA = slaveadr_bak;                   // restore old slave address
  UCB0I2CIE = ucb0i2cie;                      // restore old UCB0CTL1
  return returnValue;                         // return whether or not
                                              // a NACK occured
}

void eos_remote_handle_message(unsigned char* message) {

	unsigned char slave_address = message[0];
	unsigned char msg_datalen = message[3];

	if (slave_address > 127)
		return;

	if (!TI_USCI_I2C_slave_present(slave_address)) {
		return;
	}

	static unsigned char leadin_bytes[] = {0xbb, 0x88};
	i2c_transmit(slave_address, &leadin_bytes[0], 2);

	i2c_transmit(slave_address, message, 5 + msg_datalen);

	unsigned char response[40];
	unsigned char response_length[1];

	unsigned char count = 0;
	while (++count) {
		i2c_receive(slave_address, &response_length[0], 1);
		if (response_length[0] != 0x01) {
			break;
			// 0x01 means the slave has an unprocessed packet
			// in its queue, so keep trying
		}
		if (count == 20) return; // cancel after "timeout"
	}


	if (response_length[0] > 0 && response_length[0] < 40) {
		i2c_receive(slave_address, &response[0], response_length[0]);
		eosprotocol_send_message(response, response_length[0]);
	}


}


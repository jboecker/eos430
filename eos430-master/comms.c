/*
 * i2c_master.c
 *
 *  Created on: 02.11.2012
 *      Author: Jan
 */

#include "comms.h"
#include "msp430g2553.h"

unsigned char *i2c_rxtx_ptr;
unsigned int i2c_byte_counter;
unsigned char i2c_state = 0;

#define I2C_STATE_IDLE 0
#define I2C_STATE_TRANSMITTING 1
#define I2C_STATE_RECEIVING 2
unsigned char i2c_nackflag;

unsigned char i2c_get_nackflag() {
	return i2c_nackflag;
}

void Setup_TX(unsigned char slave_address){
  _DINT();
  i2c_state = I2C_STATE_TRANSMITTING;
  i2c_nackflag = 0;


  IE2 &= ~UCB0RXIE;
  while (UCB0CTL1 & UCTXSTP);               // Ensure stop condition got sent// Disable RX interrupt
  UCB0CTL1 |= UCSWRST;                      // Enable SW reset
  UCB0I2CIE = UCNACKIE;
  UCB0CTL1 = UCSSEL_2 + UCSWRST;            // Use SMCLK, keep SW reset
  UCB0I2CSA = slave_address;                         // Slave Address is 048h
  UCB0CTL1 &= ~UCSWRST;                     // Clear SW reset, resume operation
  IE2 |= UCB0TXIE;                          // Enable TX interrupt
}
void Setup_RX(unsigned char slave_address){
  _DINT();
  i2c_state = I2C_STATE_RECEIVING;
  i2c_nackflag = 0;
  IE2 &= ~UCB0TXIE;
  UCB0CTL1 |= UCSWRST;                      // Enable SW reset
  UCB0I2CIE = UCNACKIE;
  UCB0CTL1 = UCSSEL_2 + UCSWRST;            // Use SMCLK, keep SW reset
  UCB0I2CSA = slave_address;                         // Slave Address is 048h
  UCB0CTL1 &= ~UCSWRST;                     // Clear SW reset, resume operation
  IE2 |= UCB0RXIE;                          // Enable RX interrupt
}

void i2c_transmit(unsigned char slave_address, unsigned char* buffer, unsigned int num_bytes) {
	i2c_state = I2C_STATE_TRANSMITTING;

	// set up the pointer to the data
	i2c_rxtx_ptr = buffer;
	i2c_byte_counter = num_bytes;

	Setup_TX(slave_address);

    while (UCB0CTL1 & UCTXSTP);             // Ensure stop condition got sent
    UCB0CTL1 |= UCTR + UCTXSTT;             // I2C TX, start condition
    __bis_SR_register(CPUOFF + GIE);        // Enter LPM0 w/ interrupts

    __no_operation();

    while (UCB0CTL1 & UCTXSTP);             // Ensure stop condition got sent

}

void i2c_receive(unsigned char slave_address, unsigned char* buffer, unsigned int num_bytes) {
	i2c_state = I2C_STATE_RECEIVING;

	// set up the pointer to the receive buffer
	i2c_rxtx_ptr = buffer;
	i2c_byte_counter = num_bytes;

	Setup_RX(slave_address);

	while (num_bytes--) {
		while (UCB0CTL1 & UCTXSTP);             // Ensure stop condition got sent
		UCB0CTL1 |= UCTXSTT;                    // I2C start condition
		while (UCB0CTL1 & UCTXSTT);             // Start condition sent?
		UCB0CTL1 |= UCTXSTP;                    // I2C stop condition
		__bis_SR_register(CPUOFF + GIE);        // Enter LPM0 w/ interrupts
		while (UCB0CTL1 & UCTXSTP);             // Ensure stop condition got sent
	}


}


void uart_tx(unsigned char byte) {
	while(!(IFG2 & UCA0TXIFG));
	UCA0TXBUF = byte;
}

void eosprotocol_send_message( unsigned char* message, unsigned char totallength) {
	unsigned char i;
	for (i=0; i<totallength; i++) {
		while(!(IFG2 & UCA0TXIFG));
		UCA0TXBUF = message[i];
	}
}

#pragma vector = USCIAB0RX_VECTOR
__interrupt void USCIAB0RX_ISR(void)
{
	// check for and handle I2C NACK
	if (UCB0STAT & UCNACKIFG){            // send STOP if slave sends NACK
	  	  i2c_nackflag = 1;
	  	  UCB0CTL1 |= UCTXSTP;
	  	  UCB0STAT &= ~UCNACKIFG;
	  	  __bic_SR_register_on_exit(CPUOFF);
	}

	// check for UART RX
	if (IFG2 & UCA0RXIFG) {
		unsigned char byte = UCA0RXBUF;
		eos_byte_received(byte);
	}
}

#pragma vector = USCIAB0TX_VECTOR
__interrupt void USCIAB0TX_ISR(void)
{

	// check for and handle I2C RX
	if ((IE2 & UCB0RXIE) && (IFG2 & UCB0RXIFG)) {
		unsigned char RxData;
		if(i2c_state == I2C_STATE_RECEIVING) {                              // Master Recieve?
			RxData = UCB0RXBUF;
			*i2c_rxtx_ptr++ = RxData;
			__bic_SR_register_on_exit(CPUOFF);      // Exit LPM0
		}
		IFG2 &= ~UCB0RXIFG;
	}

	// check for and handle I2C TX
	if ((IE2 & UCB0TXIE) & (IFG2 & UCB0TXIFG)) {
		if (i2c_state == I2C_STATE_TRANSMITTING) {                                     // Master Transmit
			if (i2c_byte_counter)                        // Check TX byte counter
			{
				UCB0TXBUF = *i2c_rxtx_ptr++;                   // Load TX buffer (automatically clears UCB0TXIFG)
				i2c_byte_counter--;                            // Decrement TX byte counter
			}
			else
			{
				UCB0CTL1 |= UCTXSTP;                    // I2C stop condition
				IFG2 &= ~UCB0TXIFG;                     // Clear USCI_B0 TX int flag
				__bic_SR_register_on_exit(CPUOFF);      // Exit LPM0
			}
		}
	}
}


//******************************************************************************
//  MSP430G2xx2 Demo - I2C Slave Receiver, single byte
//
//  Description: I2C Slave communicates with I2C Master using
//  the USI. Master data should increment from 0x00 with each transmitted byte
//  which is verified by the slave.
//  LED off for address or data Ack; LED on for address or data NAck.d by the slave.
//  ACLK = n/a, MCLK = SMCLK = Calibrated 1MHz
//
//  ***THIS IS THE SLAVE CODE***
//
//                  Slave                      Master
//                                      (MSP430G2xx2_usi_07.c)
//             MSP430G2xx2          MSP430G2xx2
//             -----------------          -----------------
//         /|\|              XIN|-    /|\|              XIN|-
//          | |                 |      | |                 |
//          --|RST          XOUT|-     --|RST          XOUT|-
//            |                 |        |                 |
//      LED <-|P1.0             |        |                 |
//            |                 |        |             P1.0|-> LED
//            |         SDA/P1.7|<-------|P1.7/SDA         |
//            |         SCL/P1.6|<-------|P1.6/SCL         |
//
//  Note: internal pull-ups are used in this example for SDA & SCL
//
//  D. Dang
//  Texas Instruments Inc.
//  December 2010
//  Built with CCS Version 4.2.0 and IAR Embedded Workbench Version: 5.10
//******************************************************************************

#include <msp430g2332.h>

#include "eos_local.h"
#include "eos_dispatcher.h"

int I2C_State = 0;                     // State variable
int I2C_Transmit = 0;
int I2C_TransmitBytesLeft = 0;


#define I2C_MSG_BUFFERSIZE 50
static volatile unsigned char i2c_msg[I2C_MSG_BUFFERSIZE];
static volatile unsigned char i2c_msg_pos = 0;

void main(void)
{

	//CSL_init();

  WDTCTL = WDTPW + WDTHOLD;            // Stop watchdog

    BCSCTL1 = CALBC1_16MHZ;               // Set DCO
    DCOCTL = CALDCO_16MHZ;


  P1OUT = 0xC0;                        // P1.6 & P1.7 Pullups
  P1REN |= 0xC0;                       // P1.6 & P1.7 Pullups
  P1DIR = 0xFF;                        // Unused pins as outputs
  P2OUT = 0;
  P2DIR = 0xFF;


  P1REN |= BIT3; // enable pullup for launchpad switch
  P1OUT |= BIT3;
  P1DIR &= ~BIT3;

  USICTL0 = USIPE6+USIPE7+USISWRST;    // Port & USI mode setup
  USICTL1 = USII2C+USIIE+USISTTIE;     // Enable I2C mode & USI interrupts
  USICKCTL = USICKPL;                  // Setup clock polarity
  USICNT |= USIIFGCC;                  // Disable automatic clear control
  USICTL0 &= ~USISWRST;                // Enable USI
  USICTL1 &= ~USIIFG;                  // Clear pending flag
  _EINT();

  i2c_msg[0] = 0;

  while(1)
  {
	  eosprotocol_process_message();
  }
}


void i2c_got_byte(unsigned char data) {
	eos_byte_received(data);
}
unsigned char i2c_wants_byte() {
	if (i2c_msg_pos >= I2C_MSG_BUFFERSIZE) {
		return 0x00; // failsafe
	}
	if (i2c_msg[0] == 0x01) {
		return 0x01; // without advancing i2c_msg_pos (0x01 = "busy")
	}
	return i2c_msg[i2c_msg_pos++];
}

void eos_message_complete_callback() {
	i2c_msg[0] = 0x01; // "I am busy."
}

void eosprotocol_send_message( unsigned char* message, unsigned char totallength) {
	unsigned char i;
	if (totallength < 50) {
		for (i=0; i<totallength; i++) {
			i2c_msg[i+1] = message[i];
		}
		i2c_msg[0] = totallength;
		i2c_msg_pos = 0;
	}
}

//******************************************************************************
// USI interrupt service routine
//******************************************************************************
#pragma vector = USI_VECTOR
__interrupt void USI_TXRX (void)
{
  if (USICTL1 & USISTTIFG)             // Start entry?
  {
    I2C_State = 2;                     // Enter 1st state on start
  }

  static unsigned char slave_address = 0x00;
  switch(I2C_State)
    {
      case 0: // Idle
              break;

      case 2: // RX Address
              USICNT = (USICNT & 0xE0) + 0x08; // Bit counter = 8, RX address
              USICTL1 &= ~USISTTIFG;   // Clear start flag
              I2C_State = 4;           // Go to next state: check address
              break;

      case 4: // Process Address and send (N)Ack
              if (USISRL & 0x01) {       // master reads from us
                slave_address = (EOS_OWNADDR << 1) | 0x01;            // Save R/W bit
                I2C_Transmit = 1;
              } else { // master writes to us
            	  slave_address = EOS_OWNADDR << 1;
            	  I2C_Transmit = 0;
              }

              unsigned char usisrl;
              usisrl = USISRL;
              if (usisrl == slave_address)  // are you talking to me?
              {
            	USICTL0 |= USIOE;        // SDA = output
                USISRL = 0x00;         // send ACK for address
                if (I2C_Transmit) {
                	I2C_State = 12; 	// Go to next state: TX data
                } else {
                	I2C_State = 8;      // Go to next state: RX data
                }
              }
              else
              {
            	  // the master is talking to someone else
            	  USICTL0 &= ~USIOE;	// so we shut up
            	  I2C_State = 6;         // Go to next state: prep for next Start
              }
              USICNT |= 0x01;          // Bit counter = 1, send (N)Ack bit
              break;

      case 6: // Prep for Start condition
              USICTL0 &= ~USIOE;       // SDA = input
              slave_address = EOS_OWNADDR << 1;         // Reset slave address
              I2C_State = 0;           // Reset state machine
              break;

      case 8: // Receive data byte
              USICTL0 &= ~USIOE;       // SDA = input
              USICNT |=  0x08;         // Bit counter = 8, RX data
              I2C_State = 10;          // Go to next state: Test data and (N)Ack
              break;

      case 10:// Check Data & TX (N)Ack
              USICTL0 |= USIOE;        // SDA = output
              volatile unsigned char data = 0x00;
              data = USISRL;
              i2c_got_byte(data);

              USISRL = 0x00;         // Send Ack

              USICNT |= 0x01;          // Bit counter = 1, send (N)Ack bit

            	  I2C_State = 8;           // Go to next state: receive next data byte

              break;

      case 12: // Transmit one data byte
          USICTL0 |= USIOE;       // SDA = input
          USISRL = i2c_wants_byte();
          USICNT |=  0x08;         // Bit counter = 8, RX data
          I2C_State = 14;          // Go to next state: receive data (n)ack
          break;

      case 14: // Receive (and ignore) data (n)ack
    	  USICTL0 &= ~USIOE;
    	  USICNT |= 0x01;
    	  I2C_State = 6; // after this, prepare for the next start condition

    }

  USICTL1 &= ~USIIFG;                  // Clear pending flags
}

/*
 * eos_local.c
 *
 *  Created on: 22.11.2012
 *      Author: Jan
 */

#include <msp430g2332.h>
#include "eos_local.h"
#include "eos_dispatcher.h"
#include "eos_config.h"

static unsigned char eos_inputstate[EOS_INPUTSTATE_LENGTH];

inline void setRow(int rowNumber) {
	// all rows to L (pull-down resistor)
	P1DIR &= ~(BIT1|BIT2|BIT3|BIT4);
	P1OUT &= ~(BIT1|BIT2|BIT3|BIT4);
	P1REN |= (BIT1|BIT2|BIT3|BIT4);

	// selected row to 1
	switch(rowNumber) {
	case 0:
		P1REN &= ~BIT1;
		P1OUT |= BIT1;
		P1DIR |= BIT1;
		break;
	case 1:
		P1REN &= ~BIT2;
		P1OUT |= BIT2;
		P1DIR |= BIT2;
		break;
	case 2:
		P1REN &= ~BIT3;
		P1OUT |= BIT3;
		P1DIR |= BIT3;
		break;
	case 3:
		P1REN &= ~BIT4;
		P1OUT |= BIT4;
		P1DIR |= BIT4;
		break;
	}
}
inline int getColumnNumber() {

	int ret = 0;

	volatile unsigned char p2;

	P2DIR = 0x00;
	P2REN = 0xff;
	P2OUT = 0x00;

	_NOP();_NOP();_NOP();_NOP();

	P1OUT |= BIT0;
	p2 = P2IN;
	P1OUT &= ~BIT0;

	_NOP();
	if (P1IN & BIT5)
		ret = 1;
	if (p2 & BIT0)
		ret = 2;
	if (p2 & BIT1)
		ret = 3;
	if (p2 & BIT2)
		ret = 4;
	if (p2 & BIT3)
		ret = 5;
	if (p2 & BIT4)
		ret = 6;
	if (p2 & BIT5)
		ret = 7;
	if (p2 & BIT7)
		ret = 8;
	if (p2 & BIT6)
		ret = 9;


	return ret;
}
void eos_update_input_state() {
	eos_inputstate[0] = 0;
	eos_inputstate[1] = 0;
	eos_inputstate[2] = 0;
	eos_inputstate[3] = 0;
	eos_inputstate[4] = 0;

	unsigned char rowNumber;
	for (rowNumber = 0; rowNumber < 4; rowNumber++) {
	//for (rowNumber = 0; rowNumber < 1; rowNumber++) {
		setRow(rowNumber);
		int col = getColumnNumber();
		if (col > 0) {
			int buttonNumber = (rowNumber*9) + col - 1;
			char index =buttonNumber / 8;
			eos_inputstate[index] = 1 << (buttonNumber - (index*8));
			return;
		}
	}
}


void eos_local_handle_message(unsigned char* message) {

	unsigned char msg_cmd = message[2];
	unsigned char msg_datalen = message[3];
	unsigned char msg_checksum = message[4 + msg_datalen];


	// verify checksum
	unsigned char check = 0;
	unsigned char i;
	/*
	for (i=0; i<4+msg_datalen; i++)
		if (i != 3)
			check += message[i];
	if (check != msg_checksum)
		return;
*/

	static unsigned char response_msg[50];
	response_msg[0] = EOS_PACKET_START_BYTE;
	response_msg[1] = EOS_PACKET_LEADIN_BYTE;
	unsigned char response_msg_len = 2;

	if (msg_cmd == 130) { // INFO cmd

		unsigned char responsedata[19] = {
				EOS_BOARD_NAME,
				EOS_BOARD_VERSION,
				EOS_NUMBER_OF_DIGITAL_INPUTS, // digital input
				0, // analog inputs
				0, // rotaries
				0, // led outputs
				0, // stepper motor outputs
				0, // servo motor outputs
				0, // number of displays
		};

		unsigned char checksum = 0;

		checksum += 0;
		response_msg[response_msg_len++] = 0x00; // toaddr

		checksum += EOS_ADDRESS;
		response_msg[response_msg_len++] = EOS_ADDRESS; // fromaddr

		checksum += 66;
		response_msg[response_msg_len++] = 66; // command, response flag set, expects_response flag cleared

		response_msg[response_msg_len++] = 19; // data length, does not go into the checksum

		for (i =0; i<19; i++) {
			checksum += responsedata[i];
			response_msg[response_msg_len++] = responsedata[i];
		}
		response_msg[response_msg_len++] = checksum;

	} else if (msg_cmd == 191) { // POLL STATUS cmd
		// response:
		// zero length / no packet data -> no change
		// 1 bit per digital input in 8bit banks
		// 2 bytes per analog input
		// 1 byte per rotary encoder (signed byte indicating number of steps per poll)



		unsigned char checksum = 0;

		checksum += 0;
		response_msg[response_msg_len++] = 0; // toaddr

		checksum += EOS_ADDRESS;
		response_msg[response_msg_len++] = EOS_ADDRESS; // fromaddr

		checksum += 127;
		response_msg[response_msg_len++] = 127; // command, response flag set, expects_response flag cleared

		response_msg[response_msg_len++] = EOS_INPUTSTATE_LENGTH; // data length, does not go into the checksum


		for (i=0; i<EOS_INPUTSTATE_LENGTH; i++) {
			checksum += eos_inputstate[i];
			response_msg[response_msg_len++] = eos_inputstate[i];
		}

		response_msg[response_msg_len++] = checksum;
	}

	if (response_msg_len == 2) {
		// no response was sent
		response_msg_len = 0;
	}

	eosprotocol_send_message(response_msg, response_msg_len);

}

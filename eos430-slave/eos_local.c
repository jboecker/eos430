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
	//P1REN &= ~(BIT3|BIT4|BIT5|BIT6);
	P1DIR &= ~(BIT3|BIT4|BIT5|BIT2);
	P1OUT &= ~(BIT3|BIT4|BIT5|BIT2);
	switch(rowNumber) {
	case 0:
		P1OUT |= BIT3;
		P1DIR |= BIT3;
		break;
	case 1:
		P1OUT |= BIT4;
		P1DIR |= BIT4;
		break;
	case 2:
		P1OUT |= BIT5;
		P1DIR |= BIT5;
		break;
	case 3:
		P1OUT |= BIT2;
		P1DIR |= BIT2;
		break;
	}
}
inline int getColumnNumber() {
	if (P2IN & BIT0)
		return 1;
	if (P2IN & BIT1)
		return 2;
	if (P2IN & BIT2)
		return 3;
	if (P2IN & BIT3)
		return 4;
	if (P2IN & BIT4)
		return 5;
	if (P2IN & BIT5)
		return 6;
	if (P2IN & BIT6)
		return 7;
	if (P2IN & BIT7)
		return 8;
	if (P1IN & BIT1)
		return 9;
	return 0;
}
void eos_update_input_state(unsigned char eos_inputstate[]) {
	eos_inputstate[0] = 0;
	eos_inputstate[1] = 0;
	eos_inputstate[2] = 0;
	eos_inputstate[3] = 0;
	eos_inputstate[4] = 0;

	return;

	int rowNumber; int colNumber; int buttonNumber = 0;
	for (rowNumber = 0; rowNumber < 4; rowNumber++) {
		setRow(rowNumber);
		_NOP();_NOP();_NOP();_NOP();
		int col = getColumnNumber();
		if (col > 0) {
			int buttonNumber = (rowNumber*9) + col;
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

		eos_update_input_state(eos_inputstate);

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

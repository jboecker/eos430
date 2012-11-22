/*
 * eos_local.c
 *
 *  Created on: 22.11.2012
 *      Author: Jan
 */

#include <msp430g2332.h>
#include "eos_local.h"
#include "eos_dispatcher.h"

void eos_update_input_state(unsigned char* state) {
	if (P1IN & BIT3) {
		state[0] = 0x01;
	} else {
		state[0] = 0x00;
	}
}

void eos_local_handle_message(unsigned char* ringbuffer, unsigned char start_index, unsigned char end_index) {


	P1OUT |= BIT4;


	unsigned char msg_cmd = 0;
	unsigned char msg_datalen = 0;
	unsigned char msg_data[30];
	unsigned char msg_checksum = 0;
	unsigned char msg_fromaddr = 0;
	unsigned char msg_toaddr = 0;

	msg_toaddr = ringbuffer[start_index++ % EOS_MESSAGE_BUFFER_SIZE];
	msg_fromaddr = ringbuffer[start_index++ % EOS_MESSAGE_BUFFER_SIZE];
	msg_cmd = ringbuffer[start_index++ % EOS_MESSAGE_BUFFER_SIZE];
	msg_datalen = ringbuffer[start_index++ % EOS_MESSAGE_BUFFER_SIZE];

	unsigned char i;
	for (i=0; i<msg_datalen; i++)
		msg_data[i] = ringbuffer[start_index++ % EOS_MESSAGE_BUFFER_SIZE];
	msg_checksum = ringbuffer[start_index++ % EOS_MESSAGE_BUFFER_SIZE];



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

		unsigned char eos_inputstate[EOS_INPUTSTATE_LENGTH];
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

	P1OUT &= ~BIT4;



}

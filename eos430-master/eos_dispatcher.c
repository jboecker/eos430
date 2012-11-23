/*
 * eos_dispatcher.c
 *
 *  Created on: 22.11.2012
 *      Author: Jan
 */

#include "eos_dispatcher.h"
#include "eos_local.h"
#include "eos_remote.h"
#include <msp430.h>

// ringbuffer to hold incoming EOS messages
static unsigned char eos_message_buffer[EOS_MESSAGE_BUFFER_SIZE];
static volatile unsigned char eos_message_complete = 0;

// states are multiples of 2 to allow
// the compiler to generate a jump table
#define EOS_RXSTATE_START 0
#define EOS_RXSTATE_LEADIN 2
#define EOS_RXSTATE_TOADDR 4
#define EOS_RXSTATE_FROMADDR 6
#define EOS_RXSTATE_COMMAND 8
#define EOS_RXSTATE_DATALENGTH 10
#define EOS_RXSTATE_DATA 12
#define EOS_RXSTATE_CHECKSUM 14

// the eos_rx_state variable will contain the state for
// the next item that is expected
static unsigned char eos_rx_state = EOS_RXSTATE_START;

void eos_byte_received(unsigned char byte) {

	// rx_datalength stores the data length of the EOS packet
	// that is currently being received
	static unsigned char rx_datalength = 0;
	static unsigned char msgbuf_writepos = 0;
	static unsigned char recording_message = 0;

	if (msgbuf_writepos >= EOS_MESSAGE_BUFFER_SIZE) {
		// buffer is full, this should not normally happen
		// reset state machine
		eos_rx_state = EOS_RXSTATE_START;
		msgbuf_writepos = 0;
	}

	if (eos_message_complete) {
		// we still have a complete message (that is currently being processed)
		// in the buffer. Ignore received byte and reset state machine.
		eos_rx_state = EOS_RXSTATE_START;
		msgbuf_writepos = 0;
		return;
	}

	switch (eos_rx_state) {
	case EOS_RXSTATE_START:
		// Waiting for a START byte.
		// If we receive one, go to the next state.
		if (byte == EOS_PACKET_START_BYTE)
			eos_rx_state = EOS_RXSTATE_LEADIN;
		else
			eos_rx_state = EOS_RXSTATE_START;
		break;
	case EOS_RXSTATE_LEADIN:
		// Expecting a LEADIN byte.
		// If we do not get one, wait for the next START byte.
		if (byte == EOS_PACKET_LEADIN_BYTE)
			eos_rx_state = EOS_RXSTATE_TOADDR;
		else
			eos_rx_state = EOS_RXSTATE_START;
		break;
	case EOS_RXSTATE_TOADDR:
		recording_message = 1;
		eos_rx_state = EOS_RXSTATE_FROMADDR;
		break;
	case EOS_RXSTATE_FROMADDR:
		eos_rx_state = EOS_RXSTATE_COMMAND;
		break;
	case EOS_RXSTATE_COMMAND:
		eos_rx_state = EOS_RXSTATE_DATALENGTH;
		break;
	case EOS_RXSTATE_DATALENGTH:
		rx_datalength = byte;
		if (rx_datalength > 0)
			eos_rx_state = EOS_RXSTATE_DATA;
		else
			eos_rx_state = EOS_RXSTATE_CHECKSUM;
		break;
	case EOS_RXSTATE_DATA:
		rx_datalength--;
		if (rx_datalength == 0) {
			eos_rx_state = EOS_RXSTATE_CHECKSUM;
		}
		break;
	case EOS_RXSTATE_CHECKSUM:
		eos_message_buffer[msgbuf_writepos++] = byte;
		recording_message = 0;
		eos_message_complete = 1;
		msgbuf_writepos = 0;
		eos_message_complete_callback();

		eos_rx_state = EOS_RXSTATE_START;

		break;
	}

	if (recording_message) {
		eos_message_buffer[msgbuf_writepos++] = byte;
	}
}

void eosprotocol_process_message() {

	if (eos_message_complete) {

		unsigned char msg_toaddr = eos_message_buffer[0];

		if (EOS_OWNADDR != 0 && msg_toaddr == EOS_OWNADDR) {
			eos_local_handle_message(eos_message_buffer);
		} else {
			eos_remote_handle_message(eos_message_buffer);
		}

		// we have processed the message
		// so the buffer can be overwritten by the next one
		eos_message_complete = 0;
	}

}

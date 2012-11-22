/*
 * eos_dispatcher.c
 *
 *  Created on: 22.11.2012
 *      Author: Jan
 */

#include "eos_dispatcher.h"
#include "eos_local.h"
#include "eos_remote.h"
#include "msp430g2553.h"
#include "comms.h"

// ringbuffer to hold incoming EOS messages
static unsigned char eos_message_buffer[EOS_MESSAGE_BUFFER_SIZE];
static volatile unsigned char eos_message_complete_pos = 0;


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
	static unsigned int msgbuf_writepos = 0;
	static unsigned char recording_message;

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
		eos_message_buffer[msgbuf_writepos++ % EOS_MESSAGE_BUFFER_SIZE] = byte;
		recording_message = 0;
		eos_message_complete_pos = (msgbuf_writepos % EOS_MESSAGE_BUFFER_SIZE);

		eos_rx_state = EOS_RXSTATE_START;

		break;
	}

	if (recording_message) {
		eos_message_buffer[msgbuf_writepos++ % EOS_MESSAGE_BUFFER_SIZE] = byte;
	}
}

void eosprotocol_process_message() {

	unsigned char msg_toaddr = 0;
	unsigned char msg_fromaddr = 0;
	unsigned char msg_cmd = 0;
	unsigned char msg_datalen = 0;

	static unsigned char msgbuf_readpos = 0;

	unsigned char complete_pos = eos_message_complete_pos;
	if ((complete_pos % EOS_MESSAGE_BUFFER_SIZE) != (msgbuf_readpos % EOS_MESSAGE_BUFFER_SIZE)) {

		unsigned char pos = msgbuf_readpos;

		msg_toaddr = eos_message_buffer[pos++ % EOS_MESSAGE_BUFFER_SIZE];
		msg_fromaddr = eos_message_buffer[pos++ % EOS_MESSAGE_BUFFER_SIZE];
		msg_cmd = eos_message_buffer[pos++ % EOS_MESSAGE_BUFFER_SIZE];
		msg_datalen = eos_message_buffer[pos++ % EOS_MESSAGE_BUFFER_SIZE];

		if (msg_datalen > 50) {
			// we do not expect to handle a message this large,
			// something has obviously gone wrong.
			// so we discard all messages in the buffer.
			msgbuf_readpos = eos_message_complete_pos;
			return;
		} else {
			if (EOS_OWNADDR != 0 && msg_toaddr == EOS_OWNADDR) {
				eos_local_handle_message(eos_message_buffer, msgbuf_readpos, complete_pos);
			} else {
				eos_remote_handle_message(eos_message_buffer, msgbuf_readpos, complete_pos);
			}
		}

		msgbuf_readpos = complete_pos;

		unsigned char* check = &eos_message_buffer[msgbuf_readpos];

		_NOP();
	}

}

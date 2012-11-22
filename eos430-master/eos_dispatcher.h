/*
 * eos_dispatcher.h
 *
 *  Created on: 22.11.2012
 *      Author: Jan
 */

/*
 * The EOS dispatcher code implements the state machine
 * to parse the incoming data into separate EOS messages.
 */

#ifndef EOS_DISPATCHER_H_
#define EOS_DISPATCHER_H_

// size of the EOS message ringbuffer
#define EOS_MESSAGE_BUFFER_SIZE 250

// address of this device on the EOS bus
// messages to this device will be dispatched to the local handler
// (if this device is a dedicated EOS master without its own input elements)
#define EOS_OWNADDR 0

// call this when a new byte arrives on the EOS bus
void eos_byte_received(unsigned char data);

// call this periodically from the main loop
// to process any completed messages
void eosprotocol_process_message();

// this function has to send the given data to the PC
// totallength is the length of the message buffer
// message is the pointer to the message buffer
// the data in the buffer will be ready to send to the EOS bus,
// i.e. it will include the LEADIN and START bytes and a correct checksum
extern void eosprotocol_send_message( unsigned char* message, unsigned char totallength);

#define EOS_PACKET_START_BYTE 0xbb		// the first byte of any EOS packet
#define EOS_PACKET_LEADIN_BYTE 0x88     // the second byte of any EOS packet

#endif /* EOS_DISPATCHER_H_ */

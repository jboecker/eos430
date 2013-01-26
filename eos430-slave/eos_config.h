/*
 * eos_config.h
 *
 *  Created on: 23.11.2012
 *      Author: Jan
 */

#ifndef EOS_CONFIG_H_
#define EOS_CONFIG_H_


// address of this device on the EOS bus
// messages to this device will be dispatched to the local handler
// (if this device is a dedicated EOS master without its own input elements)
#define EOS_ADDRESS 0x07

#define EOS_NUMBER_OF_DIGITAL_INPUTS 35
#define EOS_INPUTSTATE_LENGTH 5 // must match the other NUMBER_OF_* defines

#define EOS_BOARD_NAME 'A','1','0',' ','U','F','C',' '
#define EOS_BOARD_VERSION '0','0','0','1'


// size of the EOS message ringbuffer
#define EOS_MESSAGE_BUFFER_SIZE 50


#endif /* EOS_CONFIG_H_ */

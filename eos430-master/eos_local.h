/*
 * eos_local.h
 *
 *  Created on: 22.11.2012
 *      Author: Jan
 */

#ifndef EOS_LOCAL_H_
#define EOS_LOCAL_H_

#define EOS_ADDRESS 2
#define EOS_NUMBER_OF_DIGITAL_INPUTS 8
#define EOS_INPUTSTATE_LENGTH 1

#define EOS_BOARD_NAME 'e','o','s','l','o','c','a','l'
#define EOS_BOARD_VERSION '0','0','0','1'


void eos_local_handle_message(unsigned char* message);

void eos_update_input_state(unsigned char* state);

#endif /* EOS_LOCAL_H_ */

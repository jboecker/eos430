/*
 * eos_remote.h
 *
 *  Created on: 22.11.2012
 *      Author: Jan
 */

#ifndef EOS_REMOTE_H_
#define EOS_REMOTE_H_

/*
 * Define this to 1 if you do not use any slave controller.
 *
 * Normally, the master firmware will look for other controllers on the I2C bus.
 * However, this will hang if the I2C pull-up resistors are not connected, i.e.
 * there "is no I2C bus".
 *
 * Setting EOS_DISABLE_REMOTE to 1 will cause the dispatcher to
 * ignore the I2C bus altogether.
 */
#define EOS_DISABLE_REMOTE 0
//#define EOS_DISABLE_REMOTE 1

void eos_remote_handle_message(unsigned char* message);


#endif /* EOS_REMOTE_H_ */

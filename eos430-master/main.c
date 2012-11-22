/*
 * ======== Standard MSP430 includes ========
 */
#include <msp430.h>
#include <msp430g2553.h>
#include "eos_dispatcher.h"

/*
 * ======== Grace related includes ========
 */
#include <ti/mcu/msp430/csl/CSL.h>

#include "comms.h"


void main(void)
{
	CSL_init();


	  P1DIR |= BIT0;
	  P1OUT &= ~BIT0;

	  IE2 |= UCA0RXIE;

	while(1){

		eosprotocol_process_message();
		P1OUT ^= BIT0;
	}
}

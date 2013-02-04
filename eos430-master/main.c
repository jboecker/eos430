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

	  P1DIR &= ~BIT3; // make P1.3 an input
	  P1REN |= BIT3;  // enable the pull-up resistor for P1.3
	  P1OUT |= BIT3;  // we want a pull-up, not a pull-down, so the corresponding P1OUT bit needs to be 1

	  IE2 |= UCA0RXIE;

	while(1){
		eos_update_input_state();
		eosprotocol_process_message();
		P1OUT ^= BIT0;
	}
}

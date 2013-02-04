---
layout: page
title: Why MSP430?
section: documentation
sortkey: "010"
---

# Why MSP430?

Gadrocs own firmware is available for the Arduino platform. So why
re-invent the wheel for the MSP430 controllers?

Here is a brief comparison of the MSP430 and Arduino platforms
regarding building custom hardware for Helios (so this will ignore
aspects like power consumption, which is mostly irrelevant for
non-battery powered applications).

Please note that while I have experience with Atmel AVR
microcontrollers, I have only read about the Arduino platform -- I
have not used the Arduino IDE myself yet.

## Advantages of the MSP430

* The MSP430 Value Line controllers are cheaper than the
  higher-end ATMega controllers used in the Arduino platform.

* You don't need an external crystal, because the MSP430 controllers
   can run at the full 16 Mhz from their internal clock source.

* With the MSP430 Launchpad, TI offers a development board that
includes everything you need for about a quarter of the cost of an
Arduino board:

  - A programmer for the MSP430 controllers

  - comes with two microcontrollers (a MSP430G2553 and a MSP430G2452)

  - in addition to connecting the programmer, the USB connection also
    allows you to talk to the microcontroller via a serial connection
    at 9600 baud, so you don't need a separate "FTDI cable".


## Advantages of the Arduino

* The Arduino works with 5V logic levels, so you can directly connect
  it with other 5V logic such as some LCD character displays. The
  MSP430 requires a supply voltage of 1.8V to 3.6V, so if you want to
  interface with 5V logic (such as some LCD character displays), you
  will need a level shifter. The MSP430 Launchpad will supply 3.6V to
  the controller, so if you want to connect to 3.3V logic, you will
  need to use another power source.

* The Arduino has a larger community focused on beginners. Given the
  amount and quality of documentation on the web, both platforms
  should be suitable for absolute beginners, though.

* Atmels AVR controllers have been around for a long time. Combined
  with the popularity of the Arduino platform, this means that they
  are available from a huge number of sources. MSP430 controllers are
  hard to find outside of TI resellers, so you have fewer options (see
  also: [Buying MSP430 controllers](/sourcing-msp430)).

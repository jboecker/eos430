---
layout: page
title: Overview
section: documentation
sortkey: "001"
---

# Overview

The eos430 project implements the EOS protocol on TI MSP430 microcontrollers. If you refer to the [EOS protocol specification](http://gadrocsworkshop.com/eos/protocol), ignore all the stuff about dynamic addressing, configuration and group addresses for now. As far as I know, most of this is not yet implemented in HELIOS, anyway.

Here is what you need to know to understand what eos430 does:

* Helios can talk to an **EOS bus** via a serial connection. On this
  EOS bus there exist up to 126 different **devices**, each addressed
  by its own **device address**.

* An **EOS device** has an address from 1 to 127 (address zero is reserved for the bus master, which is the PC running Helios). It also has an 8-character board name and a 4-character firmware version.

* Each EOS device has a certain number of:
  - digital inputs
  - analog inputs
  - rotary encoders
  - LED outputs
  - stepper motor outputs
  - servo motor outputs
  - alphanumeric displays

  Currently, only digital inputs are supported by eos430.

* Helios communicates with the devices on the EOS bus using certain
  commands.  The important two (and the only ones implemented by
  eos430 at the moment) are command 130 (Info), which tells Helios the
  name, firmware version, and number of buttons of your device, and
  command 191 (Poll Status), which tells Helios which buttons are
  pressed.

* For simplicity, in eos430 one microcontroller corresponds to exactly
  one EOS device.

* You will need one **master controller** and zero or more **slave
  controllers**.  These run the firmwares in the `eos430-master` and
  `eos430-slave` subfolders, respectively.

## Master Controller

The **master controller** talks to the PC over a serial port (most
likely emulated via USB) and optionally to several **slave
controllers** using I2C.  The remaining GPIO pins can be used
to connect controls.

The master firmware requires a MSP430 controller with an USCI
peripheral. The MSP430G2553 included with the MSP430 Launchpad
development board meets this requirement.

## Slave Controller

A **slave controller** allows you to add an additional EOS device. It
is connected to the master controller using I2C.

The slave firmware requires a MSP430 controller with an USI
peripheral. The MSP430G2452 included with the MSP430 Launchpad
development board meets this requirement.

The USI peripheral is less sophisticated than the USCI one, so those
controllers are cheaper.

## Program Flow

The eos430 firmware will wait for incoming EOS commands addressed to
its configured device address. In case of command 130 (Info), it will
answer with the board name, firmware version and other information
configured in `eos_local.h`.

In case of command 191 (Poll Status), it will call the
`eos_update_input_state` function. You will have to adapt this
function to your particular hardware. It reads the state of all your
controls and stores it in a byte array in the format required by the
EOS protocol.

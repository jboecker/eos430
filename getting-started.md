---
layout: page
title: Getting Started
section: tutorials
sortkey: "002"
---

# Getting Started

With the "MSP430 Launchpad", TI offers a very inexpensive development
board for their Value Line microcontrollers that includes everything
you need to get started for $4.30.

This tutorial will show you how to connect the Launchpad's built-in
push button to Helios.

## Prerequisites

This tutorial assumes you are using the "MSP430 Launchpad" development board.

It is assumed that you are using Code Composer Studio and know how to
compile code and run it on your microcontroller. If you don't, check out the excellent [Getting
Started with the MSP430 Launchpad
Workshop](http://processors.wiki.ti.com/index.php/Getting_Started_with_the_MSP430_LaunchPad_Workshop),
especially [Lab
2](http://focus.ti.com/general/docs/video/Portal.tsp?lang=en&entryid=0_yrd0la9n)
starting at 10:25, or read [Blink your first
LED](http://processors.wiki.ti.com/index.php/Blink_your_first_LED) on
the TI wiki.

If you cannot upload code to the Launchpad and are using TrackIR, try
closing the TrackIR software and reconnecting the Launchpad.

## Get the Code

The recommended way is to install `git` on your computer and clone the
repository. For this tutorial, use the `getting-started` branch. That
way you can develop your own changes to the firmware under version
control (you will most likely have to adapt the firmware to reflect
your specific panel).

If you don't want to bother with that right now, just [download the
current `getting-started` branch as a ZIP file.](https://github.com/jboecker/eos430/archive/getting-started.zip)

## Get the Firmware onto your Microcontroller

Make sure you have plugged the MSP430G2553 into your Launchpad (it
should already be plugged in out of the box).

Open the master firmware project, which is located in the
`eos430-master` subdirectory, in Code Composer Studio.

Compile and run the code on your Launchpad.

## Set up a Helios Profile

If you have not already done so, download and install
[Helios](http://gadrocsworkshop.com/helios) now.

* Open the Helios Profile Editor and create a new profile.

* Click **"Profile" -> "Add Interface"**, select **EOSBus (Direct
Serial)** and click **Add**.

* At the top of the screen, select the (virtual) COM port that your
  Launchpad is connected to.

* The text "Scanning Bus..." appears in red. After it disappears
  again, you should see your EOS device showing up under the name `1
  Button` in the list on the left.

* Under "Bindings" (at the bottom of the screen), select the "Output"
  tab. In the left tree, navigate to "Interfaces" -> "Keyboard". On
  the right side, open the `1 Button` node. 
  
* Drag the "release key" item onto the "Digital Input 0 Off" entry in the
  list on the right. Under "Properties", enter "k" into the text
  field.

* Save your profile.

## Running your Helios Profile

* Close the profile editor and start the Helios Control Center

* Select your profile using the arrow buttons

* Click "Start".

* Open a text editor such as notepad and verify that every time you
  press the left button on your Launchpad, the letter "k" is typed.

Congratulations, you have built your first custom hardware for Helios.

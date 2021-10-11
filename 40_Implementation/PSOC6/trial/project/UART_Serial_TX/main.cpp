/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mbed.h"
#include <iostream>
#include <string>

// Blinking rate in milliseconds
#define BLINKING_RATE_MS     100
#define DEVICE_SERIAL           1
using namespace std;

int main()
{
    // Initialise the digital pin LED1 as an output
	DigitalOut led(LED1);
	UARTSerial pc(P1_1,P1_0,9600); // tx, rx

	    while (true)
	    {
	        led = !led;
	        //pc.printf("Hello World!\n\r");
	        pc.write("Hello World!\n\r",20);
	        ThisThread::sleep_for(BLINKING_RATE_MS);
	    }

}

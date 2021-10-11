/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mbed.h"


// Blinking rate in milliseconds
#define BLINKING_RATE_MS     1000


int main()
{
    // Initialise the digital pin LED1 as an output
    DigitalOut led(LED1);
    Serial pc(USBTX, USBRX); // tx, rx
     
    while (true) 
    {
    	
        led = !led;
        pc.printf("Hello World!\n\r");
        ThisThread::sleep_for(BLINKING_RATE_MS);
    }

}

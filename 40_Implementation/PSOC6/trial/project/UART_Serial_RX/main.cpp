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
    UARTSerial device(P1_1,P1_0,9600);// tx, rx
    Serial pc(USBTX, USBRX);
    char c[20];
    while (true)
    {
        led = !led;
        //pc.printf("Hello World!\n\r");
        device.read(&c,20);
        pc.printf(c);
        ThisThread::sleep_for(BLINKING_RATE_MS);
    }
}

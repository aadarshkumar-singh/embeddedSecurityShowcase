#include "mbed.h"

#define BLINKING_RATE_MS     100
//Green Led
DigitalOut led1(LED1);

//Red Led
DigitalOut led2(LED2);

//Serial Object
Serial pc(USBTX, USBRX,9600);

// Serial Rx ISR
void callback()
{
    // Note: you need to actually read from the serial to clear the RX interrupt
    pc.getc();
    // Toggle green Led on reception
    led2 = !led2;
}

int main()
{
	// Attach ISR to the serial object
    pc.attach(&callback);

    while (1)
    {
    	// Blink red led
        led1 = !led1;
        ThisThread::sleep_for(BLINKING_RATE_MS);
    }
}

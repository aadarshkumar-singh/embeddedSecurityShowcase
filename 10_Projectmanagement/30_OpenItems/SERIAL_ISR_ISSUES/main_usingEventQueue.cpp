// Rx
// Execution of certain function in an ISR context is not safe.
// For example, printf() in an interrupt context causes a Mutex error.
// Using Mbed's EventQueue, you can defer execution of code from an interrupt context to a user context.
// More about EventQueue: https://os.mbed.com/docs/mbed-os/v5.15/tutorials/the-eventqueue-api.html

#include "mbed.h"

EventQueue *queue = mbed_event_queue(); // event queue
RawSerial car(USBTX,USBRX); //mbed os serial library

// On receiving data , This ISR is called
void onDataReceived();

void myCallback()
{
	if (car.readable())
	{
		printf("received: %c\r\n", car.getc());
	}

    car.attach(&onDataReceived, Serial::RxIrq);  // reattach interrupt
}

// On receiving data , This ISR is called . Interrupt are detached, User callback is called
void onDataReceived() {
    car.attach(NULL, Serial::RxIrq); // detach interrupt
    queue->call(myCallback);        // process in a different context
}

int main()
{
    car.attach(&onDataReceived, Serial::RxIrq);
    //queue->dispatch_forever(); --> Not needed
}

/****************************************************************************
* Filename        : main.cpp
* Author          :
* Description     : Main application for the embedded security showcase on
* 					securing the communication between remote control and
* 					the car running PSoC64 MCU.
*
* Note            : Main decides to run either the remote control application
* 					or car application.
*
****************************************************************************/


#include "appRemoteStateMachine.h"
#include "appCarStateMachine.h"

/***************************************************************************
 * Macro Definitions
 ***************************************************************************/

/**
 * \brief Macro definition to decide to run either remote or car application.
 */
#define REMOTE

/***************************************************************************
 * Global Variables
 ***************************************************************************/

/**
 * \brief Event Queue Object which execute callback to receive data
 * 		  every time a UART RX interrupt occurs
 */
EventQueue *queue = mbed_event_queue();

/**
 * \brief Ringbuffer to store the data received via UART.
 * 		  Since the data is received at a faster rate it is stored in a ring
 * 		  buffer.
 *
 * \note Although ringbuffer is used it has to be ensured by the sender that
 *       rx buffer is not filled up faster than it should
 *
 *       The size of the ringbuffer is UART_RB_SIZE i.e 500
 */
CRingBuffer uartRxBuf;

/***************************************************************************
 * Application Specifics
 ***************************************************************************/

/***************************************************************************
 * Car Application specific definitions
 ***************************************************************************/

#ifdef CAR

/**
 *  \brief Raw Serial Object to send/receive data via UART for the car application.
 *  @param Tx Pin
 *  @param Rx Pin
 */
RawSerial car(P13_1,P13_0);

/**
 * \brief ISR to receive serial protocol data whenever UART Rx interrupt occurs.
 * @return void
 */
void onDataReceived();

/**
 * \brief Callback to receive data via UART for the car application
 * @return void
 *
 * \note The callback is called via the event queues every time UART Rx Interrupt
 * 		 occurs
 */
void myCallback()
{
	// Check if character is available to read
	if (!car.readable())
	{
		printf("This should not happen\n");
	}

	// Loop till UART rx buffer is empty
	while (car.readable())
	{
		//This line is blocking the ISR too long
		if (uartRxBuf.write(car.getc()) == RC_SUCCESS)
		{
			//printf("W\n"); // Debug log
		}
		else
		{
			printf(" Buffer Overflow \n ");
		}
	}

	// re-attach(enable) interrupt
    car.attach(&onDataReceived, Serial::RxIrq);
}


/**
 * \brief ISR to receive serial protocol data whenever UART Rx interrupt occurs.
 * @return void
 */
void onDataReceived()
{
	// detach(disable) interrupt
    car.attach(NULL, Serial::RxIrq);

    // process the data received in a user (different) context
    queue->call(myCallback);
}

#endif


/***************************************************************************
 * Remote application specific definitions
 ***************************************************************************/

/**
 *  \brief Raw Serial Object to send/receive data via UART for the remote application.
 *  @param Tx Pin
 *  @param Rx Pin
 */
#ifdef REMOTE

RawSerial remote(P13_1,P13_0);

/**
 * \brief ISR to receive serial protocol data whenever UART Rx interrupt occurs.
 * @return void
 */
void onDataReceived();

/**
 * \brief Callback to receive data via UART for the car application
 * @return void
 *
 * \note The callback is called via the event queues every time UART Rx Interrupt
 * 		 occurs
 */
void myCallback()
{
	// Check if character is available to read
	if (!remote.readable())
	{
		printf("This should not happen\n");
	}

	// Loop till UART rx buffer is empty
	while (remote.readable())
	{

		//This line is blocking the ISR too long
		if (uartRxBuf.write(remote.getc()) == RC_SUCCESS)
		{
			//printf("W\n"); // Debug log
		}
		else
		{
			printf(" Buffer Overflow \n ");
		}

	}
	// re-attach(enable) interrupt
    remote.attach(&onDataReceived, Serial::RxIrq);  // reattach interrupt
}

/**
 * \brief ISR to receive serial protocol data whenever UART Rx interrupt occurs.
 * @return void
 */
void onDataReceived()
{
	// detach(disable) interrupt
    remote.attach(NULL, Serial::RxIrq);

    // process the data received in a user (different) context
    queue->call(myCallback);
}

#endif

int main()
{
#ifdef CAR
    car.attach(&onDataReceived, Serial::RxIrq);
    //The car application call
    car_application(car,uartRxBuf);
#endif

#ifdef REMOTE
    remote.attach(&onDataReceived, Serial::RxIrq);
    //The remote application call
    remote_application(remote,uartRxBuf);
#endif
}

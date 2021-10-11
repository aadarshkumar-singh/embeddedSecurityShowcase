/***************************************************************************
*============= Copyright by Darmstadt University of Applied Sciences =======
****************************************************************************
* Filename        : CRINGBUFFER.H
* Author          : 
* Description     : Header file that declares API to create Ringbuffer of
* 					given size.
*
****************************************************************************/

#ifndef ASW_SERVICES_RINGBUFFER_CRINGBUFFER_H_
#define ASW_SERVICES_RINGBUFFER_CRINGBUFFER_H_

/***************************************************************************
 * Includes
 ***************************************************************************/
#include "mbed.h"

/***************************************************************************
 * Macro Definitions
 ***************************************************************************/
#define UART_RB_SIZE 500

/***************************************************************************
 * Enumerations and Type definitions
 ***************************************************************************/
 /**
 * \brief Error types for Ring Buffer
 */
enum RC_RB_t{
	RC_SUCCESS,           //!< RC_SUCCESS
	RC_ERROR,             //!< RC_ERROR
	RC_WRONGPARAM,        //!< RC_WRONGPARAM
	RC_BUFFEROVERFLOW,    //!< RC_BUFFEROVERFLOW
	RC_BUFFERUNDERFLOW,   //!< RC_BUFFERUNDERFLOW
	RC_PERIPHERALOCCUPIED,//!< RC_PERIPHERALOCCUPIED
	RC_NODATA             //!< RC_NODATA
};


/**
 * \brief Class that Defines APIs to create Ring buffer of given size.
 */
class CRingBuffer {
private:

	/**
	 * \brief Pointer that stores the address of buffer of given size
	 */
	char m_pBuffer[UART_RB_SIZE];

	/**
	 * \brief Index that indicates the buffer index untill which
	 * 		  Buffer is read.
	 */
	uint16_t m_readIdx;

	/**
	 * \brief Index that indicates the buffer index untill which
	 * 		  Buffer is written.
	 */
	uint16_t m_writeIdx;

	/**
	 * \brief Index that indicates the buffer index untill which
	 * 		  Buffer is filled.
	 */
	uint16_t m_fillLevel;
public:


	/**
	 * \brief Creates a ring buffer of given size
	 * @param size size of the ringbuffer
	 */
    CRingBuffer();

    /**
     * \brief API to read the data from the ringbuffer
     * @param data Data which is read from the buffer is copied into
     * 			   the parameter
     * @return RC_SUCCESS : When the data is read successfully.
     * 		   RC_BUFFERUNDERFLOW : when the data read has failed.
     */
    RC_RB_t read(char& data);

    /**
     * \brief To know the index of the buffer untill which the ringbuffer is
     * 		  Filled.
     * @return Fill Level of the buffer
     */
    uint16_t getFillLevelOfBuffer();

    /**
     * \brief API to write the data in the ringbuffer.
     * @param data Data to be written in the ringbuffer
     * @return ErrorCode : RC_SUCCESS : When Data written successfully.
     * 					   RC_BUFFERUNDERFLOW : when the data written is failed.
     */
    RC_RB_t write(char data);

    /**
     * \brief API to clear the Ringbuffer
     * @return Error Code : RC_SUCCESS : When Cleared the Buffer successfully.
     */
    RC_RB_t clear();

};

/********************
**  CLASS END
*********************/
#endif /* CRINGBUFFER_H */

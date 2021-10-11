/***************************************************************************
*============= Copyright by Darmstadt University of Applied Sciences =======
****************************************************************************
* Filename        : CRINGBUFFER.CPP
* Author          : Aadarsh Kumar Singh
* Description     : Source file that defines API to create Ringbuffer of
* 					given size.
****************************************************************************/

/***************************************************************************
 * Includes
 ***************************************************************************/

#include "CRingBuffer.h"

/**
 * \brief Creates a ring buffer of UART_RB_SIZE size
 */
CRingBuffer::CRingBuffer()
{

	m_fillLevel = 0;
	m_readIdx = 0;
	m_writeIdx = 0;
	
	/*Filling buffer with NULL values*/
	for(int i =0;i<UART_RB_SIZE;i++)
		m_pBuffer[i]='\0';

}


/**
 * \brief API to read the data from the ringbuffer
 * @param data Data which is read from the buffer is copied into
 * 			   the parameter
 * @return RC_SUCCESS : When the data is read successfully.
 * 		   RC_BUFFERUNDERFLOW : when the data read has failed.
 */
RC_RB_t CRingBuffer::read(char& data)
{
	if (m_fillLevel > 0)
	{
		/*poping data from ring buffer */
		data = m_pBuffer[m_readIdx++];

		m_readIdx %= UART_RB_SIZE;
		m_fillLevel--;

		return RC_SUCCESS;
	}
	else
	{
		/*if the m_filllevel is less than or equal to 0,the buffer is empty*/
		return RC_BUFFERUNDERFLOW;
	}

}

/**
 * \brief API to write the data in the ringbuffer.
 * @param data Data to be written in the ringbuffer
 * @return ErrorCode : RC_SUCCESS : When Data written successfully.
 * 					   RC_BUFFERUNDERFLOW : when the data written is failed.
 */
RC_RB_t CRingBuffer::write(char data)
{
	if (m_fillLevel < UART_RB_SIZE)
	{
		m_pBuffer[m_writeIdx++] = data;

		m_writeIdx %= UART_RB_SIZE;
		m_fillLevel++;

		return RC_SUCCESS;
	}
	else
	{
		/*the size of m_fillLevel is reached its maximum (UART_RB_SIZE),so a write operation is not allowed*/
		return RC_BUFFERUNDERFLOW;
	}
}


/**
 * \brief API to clear the Ringbuffer
 * @return Error Code : RC_SUCCESS : When Cleared the Buffer successfully.
 */
RC_RB_t CRingBuffer::clear()
{
	m_fillLevel = 0;
	m_readIdx = 0;
	m_writeIdx = 0;

	/*Clearing buffer data to NULL*/
	for(int i =0;i<UART_RB_SIZE;i++)
		m_pBuffer[i]='\0';

	return RC_SUCCESS;
}

/**
 * \brief To know the index of the buffer untill which the ringbuffer is
 * 		  Filled.
 * @return Fill Level of the buffer
 */
uint16_t CRingBuffer::getFillLevelOfBuffer()
{
	return m_fillLevel ;
}


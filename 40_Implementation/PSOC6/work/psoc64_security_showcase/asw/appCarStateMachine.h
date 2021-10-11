/****************************************************************************
* Filename        : appCarStateMachine.h
* Author          : Hari Krishna Yelchuri
* Description     : Header file that declares functions for the car
* 					application
*
****************************************************************************/

#ifndef ASW_SERVICES_CARAPPLICATION_H_
#define ASW_SERVICES_CARAPPLICATION_H_

/***************************************************************************
 * Includes
 ***************************************************************************/
#include "mbed.h"
#include "CRingBuffer.h"
#include "encryptMgr.h"
#include "signMgr.h"
#include "signVerifyMgr.h"
#include "dhKeyExchangeMgr.h"
#include "encryptMgrTest.h"
#include "decryptMgrTest.h"

/***************************************************************************
 * Macro Definitions
 ***************************************************************************/

/************************ < Macros for UART protocol ***********************/

/**< Start of frame for the UART Protocol  */
#define CAR_PROTOCOL_SOF '$'

/**< End of packet 1 for the UART Protocol */
#define CAR_PROTOCOL_EOP1 '@'

/**< End of packet 2 for the UART Protocol */
#define CAR_PROTOCOL_EOP2 '#'

/**< Length of protocol excluding data packets */
#define CAR_PROT_ID_LENGTH 3

/**< First data byte of car's positive acknowledgment frame */
#define CAR_POS_ACK_BYTE_1 '*'

/**< Second data byte of car's positive acknowledgment frame */
#define CAR_POS_ACK_BYTE_2 '%'

/************************ < Macros for Signature ***********************/

/**< Signature length of car */
#define CAR_SIGNATURE_LENGTH 64

/**< length of public key for generating signature */
#define CAR_PUB_KEY_LENGTH 65

/**< Length of Buffer in which Signature Public key is stored */
#define CAR_SIGNATURE_PUB_KEY_BUF_LEN (CAR_PUB_KEY_LENGTH + CAR_PROT_ID_LENGTH)

/**< Length of Buffer in which signature of car is stored */
#define CAR_SIGNATURE_BUFFER_LENGTH (CAR_SIGNATURE_LENGTH + CAR_PROT_ID_LENGTH)

/************************ < Macros for Diffie Hellman ******************/

/**< Length of Diffie Hellman Public key */
#define CAR_DH_PUB_KEY_SIZE 32

/**< Length of Diffie Hellman shared secret key used for encryption */
#define CAR_ENC_KEY_SIZE 16

/**< Length of Buffer in which Diffie Hellman Public key is stored */
#define CAR_DH_PUB_KEY_BUF_LEN (CAR_DH_PUB_KEY_SIZE + CAR_PROT_ID_LENGTH)

/************************ < Macros for Encryption/Decryption ************/

/**< Length of Buffer in which Initialization vector used for encryption is stored */
#define CAR_IV_BUFFER_LEN (AES_BLOCK_SIZE_SUPPORTED_BY_HW + CAR_PROT_ID_LENGTH)

/**< Length of Buffer in which length of encrypted text is stored */
#define CAR_CIPHER_TEXT_BUFFER_LEN (1 + CAR_PROT_ID_LENGTH)

/***************************************************************************
 * Function Declarations
 ***************************************************************************/

/**
 * \brief API which contains statemachine logic for car application
 * @param car    : UART port used by car for tx/rx message
 * @param uartRxBuf : UART Messages that are received are stored in this ringbuffer
 *
 * \note size of ringbuffer is 500 bytes
 */
void car_application(RawSerial &car,CRingBuffer &uartRxBuf);

/**
 * \brief API to receive UART protocol one byte at a time, It parses required cryptographic
 *        messages like signature,keys etc from the protocol received. It uses SOF and EOP1,
 *        EOP2 identifiers present in the protocol for parsing.
 *
 * @param rxdData    : Data Byte received from Ringbuffer
 * @param dataLength : Length of protocol that has to be parsed from the received UART data.
 * @param uartRxBuf  : Ringbuffer in which UART data is stored
 * \note size of ringbuffer is 500 bytes
 *
 * @return true  : When End of Packet 2 is received i.e. on indication of end of message
 * 		   false : Untill End of packet 2 is received.
 */
bool car_rxData(unsigned char* myData, unsigned int dataLength, CRingBuffer &uartRxBuf);

/**
 * \brief The cryptographic data like signature, keys, cipher text etc. are translated into
 * 		  UART Protocol in the below format :
 * 		  <SOF><Data Byte 1><Data Byte 2>...<EOP1><EOP2>
 *
 * @param inputBuffer     : Input Buffer containing the protocol
 * @param outputBuffer[o] : Output buffer that has to be transmitted
 * @param length	      : Length of protocol
 */
void car_createtxPacket(uint8_t* inputBuffer, char * outputBuffer, int length);

/**
 *
 * \brief Sends data byte by byte from the created UART protocol packet
 *
 * @param buffer : Buffer that has to be transmitted via UART
 * @param length : Length of buffer that has to be transmitted
 * @param car : UART port used by car for tx message
 *
 * @return true : When entire buffer is sent via UART
 * 		   false : Untill entire buffer is sent via UART
 */
bool car_SendDataByte( char * buffer , uint8_t length, RawSerial &car);

/**
 * \brief  API to check if positive acknowledgment is sent by receiver. It checks if the
 * 		   receiver has received the message properly. The receiver sends a positive
 * 		   acknowledgment message <$,*,*> if messages are received properly.
 *
 * @param uartRxBuf : Ringbuffer in which UART data is stored
 * @return true : When Positive Acknowledgment is received (Received buffer contains <$,*,*>)
 * 		   false : When negative Acknowledgment is received
 */
bool car_CheckAckRxd(CRingBuffer &uartRxBuf);

/**
 * \brief API to send positive acknowledgment <$,*,*> when messages are received correctly.
 * @param car : UART port used by car for tx message
 */
void car_SendAck(RawSerial &car);

/**
 * \brief Generates signature and public key for the car.
 * 		  The generated signature and public key is packetized into UART protocol
 * 		  and returned.
 * @param signBuf [o]: returns buffer containing signature
 * @param pubKeyBuf[o] : returns Buffer containing Public key
 */
void car_generateSignAndPubKey(char *signBuf, char *pubKeyBuf);

/**
 * \brief Generates Diffie Hellmann Context to generate Diffie Hellmann public key
 * 		  that has to be exchanged with the receiver.
 *
 * @param carDhCtx         : Diffie Hellmann Context
 * @param carDhPubKey[o]   : Generated Diffie Hellmann public key
 * @param DhPubKeyBuffer[o]: Buffer containing UART protocol of DH public key to be transmitted
 */
void car_GenerateDhPubKey(dhKeyExchangeMgr &carDhCtx,uint8_t *carDhPubKey, char *DhPubKeyBuffer);

/**
 * \brief Computes the shared secret key using Diffie Hellmann Context and the the DH public key that is
 * 		  generated.
 *
 * @param carDhCtx            : Diffie Hellmann Context
 * @param carRxdRemoteDhPubKey   : Generated Diffie Hellmann public key
 * @param carSharedEncKey [o] : DH Secret Key that is computed
 * @return : true  : If shared secret key is computed successfully.
 * 		   : false : If computation of shared secret key fails.
 */
void car_computeSharedSecretKey(dhKeyExchangeMgr &carDhCtx , uint8_t* carRxdRemoteDhPubKey , uint8_t* carSharedEncKey );

#endif

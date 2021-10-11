/****************************************************************************
* Filename        : appCarStateMachine.cpp
* Author          : Hari Krishna Yelchuri
* Description     : Source file that defines APIs for the car
* 					application
*
****************************************************************************/

#include "appCarStateMachine.h"

/***************************************************************************
 * Type definitions
 ***************************************************************************/

/**
 * \brief Describes the states of the car state machine
 */
typedef enum carStates_e
{
	CAR_RX_SIGNATURE,
	CAR_RX_PUBLICKEY,
	CAR_VERIFY_AUTHENTICITY,
	CAR_SEND_SIGNED_CREDENTIAL,
	CAR_CHECK_SIGN_RXD_ACK,
	CAR_SEND_PUB_KEY,
	CAR_PUB_KEY_RXD_ACK,
	CAR_CRED_VERIFIED_BY_REMOTE,
	CAR_RX_REMOTE_DH_KEY,
	CAR_DH_KEY_SEND,
	CAR_RX_DH_KEY_ACK,
	CAR_RX_IV,
	CAR_RX_LEN,
	CAR_RX_ENCDATA,
	CAR_TEST,
	CAR_ENCRYPT_MESSAGE,
	CAR_SEND_ENCRYPT_IV,
	CAR_RX_IV_ACK,
	CAR_SEND_ENCRYPT_LEN,
	CAR_RX_ENCRYPT_LEN_ACK,
	CAR_SEND_CIPHER_TEXT,
	CAR_RX_ACK_CIPHER_TEXT,
	CAR_DEFAULT,
}carStates_t;


/**
 * \brief Describes states of car to receive the UART protocol messages
 */
typedef enum car_rxStates_e
{
	CAR_RX_SOP,
	CAR_RX_DATA,
	CAR_RX_EOP1,
	CAR_RX_EOP2,
}car_rxStates_t;

/**
 * \brief Describe states of car to check if acknowledgment is sent by the receiver.
 */
typedef enum car_rxAckStates_e
{
	CAR_RX_ACK_SOP,
	CAR_RX_ACK_BYTE1,
	CAR_RX_ACK_BYTE2,
}car_rxAckStates_t;

/***************************************************************************
 * API definitions
 ***************************************************************************/

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
bool car_rxData(unsigned char* myData, unsigned int dataLength, CRingBuffer &uartRxBuf)
{
	// Statemachine that receives data byte by byte,
	// data between the <SOF> byte and <EOP1,EOP2> bytes are stored in the receive buffer
	// Discards the message if expected protocol is not received and returns false
	static unsigned int dataIdx = 0;
	static char ch = '\0';
	static car_rxStates_t state = CAR_RX_SOP;

	if (uartRxBuf.read(ch) == RC_SUCCESS)
	{
		switch (state)
		{
		// Checks the start of payload
		case CAR_RX_SOP:
		{
			if(ch == CAR_PROTOCOL_SOF)
				state = CAR_RX_DATA;
			break;
		}

		// Received UART data between <SOF> and <EOP1,EOP2> is stored
		case CAR_RX_DATA:
		{
			myData[dataIdx++] = ch;
			if (dataIdx == dataLength)
			{
				dataIdx = 0;
				state = CAR_RX_EOP1;
			}
			break;
		}

		// Checks if received char is first byte of end of payload
		case CAR_RX_EOP1:
		{
			if(ch == CAR_PROTOCOL_EOP1)
			{
				state = CAR_RX_EOP2;
			}
			else
			{
				state = CAR_RX_SOP;
			}
			break;
		}

		// Checks if received char is second byte of end of payload
		case CAR_RX_EOP2:
		{
			if(ch == CAR_PROTOCOL_EOP2)
			{
				state = CAR_RX_SOP;
				return 1;
			}
			else
			{
				state = CAR_RX_SOP;
			}
			break;
		}
		// Debug print in case expected protocol is not received.
		default:
		{
			printf("Something is wrong \n");
		}

		}
	}
	return 0;
}


/**
 * \brief The cryptographic data like signature, keys, cipher text etc. are translated into
 * 		  UART Protocol in the below format :
 * 		  <SOF><Data Byte 1><Data Byte 2>...<EOP1><EOP2>
 *
 * @param inputBuffer     : Input Buffer containing the protocol
 * @param outputBuffer[o] : Output buffer that has to be transmitted
 * @param length	      : Length of protocol
 */
void car_createtxPacket(uint8_t* inputBuffer, char * outputBuffer, int length)
{
	int outputIdx = 0;
	int inputIdx = 0;

	// First byte is Start of frame
	outputBuffer[outputIdx] = CAR_PROTOCOL_SOF ;

	// Data payload bytes
	for (outputIdx = 1; outputIdx <= length ; outputIdx++)
	{
		outputBuffer[outputIdx] = (char)inputBuffer[inputIdx++] ;
	}

	// Symbols for end of frame
	outputBuffer[outputIdx] = CAR_PROTOCOL_EOP1;
	outputBuffer[++outputIdx] = CAR_PROTOCOL_EOP2;
}

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
bool car_SendDataByte( char * buffer , uint8_t length, RawSerial &car)
{
	static int bufIdx = 0;

	// checks if write buffer of UART is free to send data
	if(car.writeable() != 0)
	{
		// sends one byte of data at a time until entire payload is sent
		car.putc(buffer[bufIdx++]);
		if (bufIdx == length)
		{
			bufIdx = 0 ;
			return true ;
		}
	}
	else
	{
		// returns false and error log when UART write buffer is busy
		printf("Error sending \n");
		return false ;
	}

	// return false until entire payload is sent byte by bytes
	return false ;

}



/**
 * \brief  API to check if positive acknowledgment is sent by receiver. It checks if the
 * 		   receiver has received the message properly. The receiver sends a positive
 * 		   acknowledgment message <$,*,*> if messages are received properly.
 *
 * @param uartRxBuf : Ringbuffer in which UART data is stored
 * @return true : When Positive Acknowledgment is received (Received buffer contains <$,*,*>)
 * 		   false : When negative Acknowledgment is received
 */
bool car_CheckAckRxd(CRingBuffer &uartRxBuf)
{
	static char ch = '\0';
	static car_rxAckStates_t state = CAR_RX_ACK_SOP;


	// Check if UART data is read successfully from receive ringbuffer
	// Discards the message if expected protocol is not received and
	// returns false
	if (uartRxBuf.read(ch) == RC_SUCCESS)
	{
		// state machine that checks received data byte by byte
		// only when complete acknowledgment protocol is obtained it returns true
		switch (state)
		{

		// Checks if first Frame is Start Of Payload ($)
		case CAR_RX_ACK_SOP:
		{

			if(ch == CAR_PROTOCOL_SOF)
				state = CAR_RX_ACK_BYTE1;
			break;
		}
		// check if first byte of acknowledgment payload (*) is received
		case CAR_RX_ACK_BYTE1:
		{
			if (ch == CAR_POS_ACK_BYTE_1)
			{
				state = CAR_RX_ACK_BYTE2;
			}
			else
			{
				state = CAR_RX_ACK_SOP;
			}
			break;
		}
		// check if second byte of acknowledgment payload (*) is received
		case CAR_RX_ACK_BYTE2:
		{
			if(ch == CAR_POS_ACK_BYTE_2)
			{
				state = CAR_RX_ACK_SOP;
				return true;
			}
			else
			{
				state = CAR_RX_ACK_SOP;
			}
			break;
		}
		// In case invalid data is received instead of acknowledgment frames
		default:
		{
			printf("Something is wrong \n");
		}

		}
	}
	return false;
}

/**
 * \brief API to send positive acknowledgment <$,*,*> when messages are received correctly.
 * @param car : UART port used by car for tx message
 */
void car_SendAck(RawSerial &car)
{
	char remoteAck[3] = {CAR_PROTOCOL_SOF,CAR_POS_ACK_BYTE_1, CAR_POS_ACK_BYTE_2};

	for (int i = 0 ; i< 3 ; i++)
	{
		car.putc(remoteAck[i]);
	}

	printf("    Ack Sent. \n ");
}



/**
 * \brief Generates signature and public key for the car.
 * 		  The generated signature and public key is packetized into UART protocol
 * 		  and returned.
 * @param signBuf [o]: returns buffer containing signature
 * @param pubKeyBuf[o] : returns Buffer containing Public key
 */
void car_generateSignAndPubKey(char *signBuf, char *pubKeyBuf)
{
	// Unique ID
	uint8_t remoteId[] = {"I am 01234567891"};

	uartData_t remoteDataToTx ;
	signMgr remoteSignDataToTxd(remoteId);
	remoteSignDataToTxd.signhash();
	remoteDataToTx = remoteSignDataToTxd.export_PublicKey();

	car_createtxPacket(remoteDataToTx.signature,signBuf,CAR_SIGNATURE_LENGTH);
	car_createtxPacket(remoteDataToTx.publicKey,pubKeyBuf,CAR_PUB_KEY_LENGTH);
}

/**
 * \brief Generates Diffie Hellmann Context to generate Diffie Hellmann public key
 * 		  that has to be exchanged with the receiver.
 *
 * @param carDhCtx         : Diffie Hellmann Context
 * @param carDhPubKey[o]   : Generated Diffie Hellmann public key
 * @param DhPubKeyBuffer[o]: Buffer containing UART protocol of DH public key to be transmitted
 */
void car_GenerateDhPubKey(dhKeyExchangeMgr &carDhCtx,uint8_t *carDhPubKey, char *DhPubKeyBuffer)
{
	carDhCtx.generateDhKey(carDhPubKey);
	car_createtxPacket(carDhPubKey,DhPubKeyBuffer,CAR_DH_PUB_KEY_SIZE);
}

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
// make it bool later based on return type
void car_computeSharedSecretKey(dhKeyExchangeMgr &carDhCtx , uint8_t* carRxdRemoteDhPubKey , uint8_t* carSharedEncKey )
{
	carDhCtx.computeSharedSecretDhKey(carRxdRemoteDhPubKey, carSharedEncKey);
}

/**
 * \brief API which contains statemachine logic for car application
 * @param car    : UART port used by car for tx/rx message
 * @param uartRxBuf : UART Messages that are received are stored in this ringbuffer
 *
 * \note size of ringbuffer is 500 bytes
 */
void car_application(RawSerial &car,CRingBuffer &uartRxBuf)
{
	/* remote application states */
	carStates_t state = CAR_RX_SIGNATURE;

	/* variables used for signature generation and verification */
	uartData carVerifyRemoteSign;
	char signatureBuffer[CAR_SIGNATURE_BUFFER_LENGTH];
	char signPubKeyBuffer[CAR_SIGNATURE_PUB_KEY_BUF_LEN];

	/* variables used for Diffie Hellmann shared secret key exchange */
	unsigned char dhBufRemotetoCar[32];
	char carDhPubKeyBuffer[CAR_DH_PUB_KEY_BUF_LEN];
	static dhKeyExchangeMgr carDhCtx;
	static uint8_t carDhPubKey[CAR_DH_PUB_KEY_SIZE];
	static uint8_t carRxdRemoteDhPubKey[CAR_DH_PUB_KEY_SIZE];
	static uint8_t carSharedEncKey[CAR_ENC_KEY_SIZE];

	// Generate signature of remote controller
	car_generateSignAndPubKey(signatureBuffer, signPubKeyBuffer);

	// Generate diffie Hellmann public key for shared secret computation
	car_GenerateDhPubKey(carDhCtx,carDhPubKey,carDhPubKeyBuffer);

	/* variables used for decryption */
	char car_IV[16];
	unsigned char lengthbuffer;
	char* decryptBuffer = 0;

	/* variables used for encryption */
	uint8_t plainText[]= {"I am plaintext.1234"};
	encryptMgr encryptObj (plainText,sizeof(plainText),ENCRYPT_CBC_WITH_PADDING);
	char *cipherTextBuffer;
	char remoteIvTxBuf[CAR_IV_BUFFER_LEN];
	char lenCipherTextBuf[CAR_CIPHER_TEXT_BUFFER_LEN];
	uint8_t cipherTextlen =0;
	uint8_t cipherTextBuflen =0;

	while(true)
	{
		/* Waiting for sync between tx side and rx side, tx side should send slowly
				   so that rx buffer is not filled up faster than it should. */
		wait_us(1000);

		//State machine car application
		switch(state)
		{
			//For authenticating remote, car receives a signature
			case CAR_RX_SIGNATURE:
			{
				if(car_rxData((unsigned char *)carVerifyRemoteSign.signature, carVerifyRemoteSign.signature_length, uartRxBuf))
				{
					printf(" \n Car : received  remote signature : ");
					for (int i = 0 ; i< 64 ; i++)
							printf("%c",carVerifyRemoteSign.signature[i]);
					printf("\n");

					car_SendAck(car);
					state = CAR_RX_PUBLICKEY;
				}
				break;
			}
			//Receive the public key to verify the signature
			case CAR_RX_PUBLICKEY:
			{
				if(car_rxData((unsigned char *)carVerifyRemoteSign.publicKey, carVerifyRemoteSign.publiKey_length, uartRxBuf))
				{
					printf (" \n Car : received remote public key  : ");
					for (int i = 0 ; i< 65 ; i++)
							printf("%c",carVerifyRemoteSign.publicKey[i]);

					printf("\n");

					car_SendAck(car);
					state = CAR_VERIFY_AUTHENTICITY;
				}
				break;
			}
			//Verify the authenticity using signature and public key
			case CAR_VERIFY_AUTHENTICITY :
			{
				signVerifyMgr testObj(carVerifyRemoteSign);
				if(testObj.verifySignature())
				{
					printf(" \n Car : verified remote signature \n ");
					car_SendAck(car);
					state = CAR_SEND_SIGNED_CREDENTIAL;
				}
				break;
			}

			//Send signature to get authenticated
			case CAR_SEND_SIGNED_CREDENTIAL :
			{
				if (car_SendDataByte(signatureBuffer,CAR_SIGNATURE_BUFFER_LENGTH, car))
				{
					printf(" \n Car : Sent signature to remote \n ");
					state = CAR_CHECK_SIGN_RXD_ACK;
				}

				break;
			}

			//Receive ack, if remote receives data successfully
			case CAR_CHECK_SIGN_RXD_ACK:
			{
				if (car_CheckAckRxd(uartRxBuf))
				{
					printf (" \n Car : got signature received acknowledgment from remote \n");
					printf (" \n Car : sending  public Key for verifying signature \n");
	
					state = CAR_SEND_PUB_KEY;
				}
				break;
			}
			//Send public key for verification of sent signature
			case CAR_SEND_PUB_KEY :
			{

				if (car_SendDataByte(signPubKeyBuffer,CAR_SIGNATURE_PUB_KEY_BUF_LEN, car))
				{
					printf(" \n Car : Sent public key to remote  \n ");
					state = CAR_PUB_KEY_RXD_ACK;
				}
				break;
			}
			//Receive ack, if remote receives data successfully
			case CAR_PUB_KEY_RXD_ACK :
			{
				if (car_CheckAckRxd(uartRxBuf))
				{
					printf (" \n Car : got public key received acknowledgment from remote \n");
					state = CAR_CRED_VERIFIED_BY_REMOTE;
				}

				break;
			}
			//Receive ack, if remote authenticates car successfully
			case CAR_CRED_VERIFIED_BY_REMOTE :
			{
				if (car_CheckAckRxd(uartRxBuf))
				{
					printf (" \n Car : signature verified by remote \n");
					state = CAR_RX_REMOTE_DH_KEY;
				}
				break;
			}
			//Receive Remote DH Key and compute shared secret key
			case CAR_RX_REMOTE_DH_KEY :
			{
				/* receives UART protocol containing DH public key of car byte by byte (used for shared secret key computation)
				   remote remains in this state until entire protocol with DH public key is received (blocking call)
				   When complete protocol is received it sends positive acknowledgment frame to car. */
				if( car_rxData( dhBufRemotetoCar, 32, uartRxBuf))
				{
					printf (" \n Car : received deffie helmann public key of remote : ");
					for (int i = 0 ; i< 32 ; i++)
					{
						printf("%c", dhBufRemotetoCar[i]);
						carRxdRemoteDhPubKey[i] = (uint8_t)dhBufRemotetoCar[i];
					}
					printf("\n");

					car_SendAck(car);
					car_computeSharedSecretKey(carDhCtx , carRxdRemoteDhPubKey , carSharedEncKey );

					printf(" \n Car : computed shared secret key successfully :  ");
					for (int i = 0 ; i< 16 ; i++)
					{
						printf("%d", carSharedEncKey[i]);
					}
					printf("\n");

					state = CAR_DH_KEY_SEND;
				}
				break;
			}
			//car sends diffie Hellmann public key for shared secret key computation
			case CAR_DH_KEY_SEND :
			{
				if (car_SendDataByte(carDhPubKeyBuffer, CAR_DH_PUB_KEY_BUF_LEN, car))
				{
					printf("\n Car : deffie helmann public key sent \n");
					state = CAR_RX_DH_KEY_ACK;
				}
				break;
			}
			//Receive ack for DH public key
			case CAR_RX_DH_KEY_ACK :
			{
				if (car_CheckAckRxd(uartRxBuf))
				{
					printf (" \n Car : got deffie helmann public key received ack from remote \n");
					state = CAR_RX_IV;
				}
				break;
			}

			//Receive the initialisation vector
			case CAR_RX_IV :
			{
				if(car_rxData((unsigned char *)car_IV, 16, uartRxBuf))
				{
					printf (" \n Car : received encryption IV of remote : ");
					for (int i = 0 ; i< 16 ; i++)
					{
						printf("%c",car_IV[i]);
					}
					printf("\n");

					car_SendAck(car);
					state = CAR_RX_LEN;
				}
				break;
			}

			//Receive the length of the vector
			case CAR_RX_LEN :
			{
				if(car_rxData(&lengthbuffer, 1, uartRxBuf))
				{
					printf (" \n Car : received encrypted message length of remote : ");
					printf("%c",lengthbuffer);
					printf("\n");

					car_SendAck(car);
					decryptBuffer = new char[(uint8_t)lengthbuffer];
					state = CAR_RX_ENCDATA;
				}
				break;
			}

			//Here car receives the encrypted data from the remote
			case CAR_RX_ENCDATA :
			{
				if(car_rxData((unsigned char *)decryptBuffer, (uint8_t)lengthbuffer, uartRxBuf))
				{
					printf (" \n Car : received encrypted message of remote : ");
					for (int i = 0 ; i< (uint8_t)lengthbuffer ; i++)
							printf("%c",decryptBuffer[i]);

					printf("\n");

					car_SendAck(car);
					state = CAR_TEST ;
				}

				break;
			}

			/* car decrypts the encrypted message using the DH shared secret key, IV and length of cipher text
						   upon decryption sends positive acknowledgment frame.*/
			case CAR_TEST :
			{
				printf(" \n Car : Decrypting with the computed shared secret key : \n");
				decryptMgrTest_TestCase_EncryptedData((uint8_t*)decryptBuffer,
						(uint8_t)lengthbuffer, (uint8_t*)car_IV,
						carSharedEncKey);
				car_SendAck(car);
				printf(" \n Car : message received from remote decrypted successfully \n ");
				state = CAR_ENCRYPT_MESSAGE;
				break;
			}

			// car encrypts the message to be transmitted using the diffie Hellmann shared secret key
			case CAR_ENCRYPT_MESSAGE :
			{
				if (psa_crypto_init() != PSA_SUCCESS)
				{
				  printf("Failed to initialize PSA Crypto\n");
				}

				if (encryptObj.encryptionDhKeyHandle(carSharedEncKey))
				{
					//printf("\n Key Handle Encrypt DH Success \n ");

					if ( ENCRYPT_SUCCESS == encryptObj.encryptUsingSymmetricCiphers())
					{
							printf(" \n Car : message encrypted successfully \n");

							cipherTextlen = encryptObj.sizeofEncryption() ;
							cipherTextBuflen = cipherTextlen + CAR_PROT_ID_LENGTH ;
							cipherTextBuffer = new char[cipherTextBuflen];

							car_createtxPacket(encryptObj.GetEncryptedData(),cipherTextBuffer, encryptObj.sizeofEncryption());
							car_createtxPacket(encryptObj.GetIvData(), remoteIvTxBuf,AES_BLOCK_SIZE_SUPPORTED_BY_HW);
							car_createtxPacket(&cipherTextlen,lenCipherTextBuf,1);
							state = CAR_SEND_ENCRYPT_IV;
					}
					else
					{
						printf("Encryption Failure\n");
					}
				}
				break;
			}

			/* Computed IV of remote is sent to remote . The IV is generated randomly every time encryption is performed.
					   In this example we encrypt a simple plain text message only once.
					   Also at a time maximum of 255 bytes can be encrypted. */
			case CAR_SEND_ENCRYPT_IV:
			{
				if (car_SendDataByte(remoteIvTxBuf,CAR_IV_BUFFER_LEN, car))
				{
					printf("\n Car :  Encryption IV Sent \n");
					state = CAR_RX_IV_ACK;
				}
				break;
			}

			// remote checks if remote sent positive acknowledgment after receiving its IV
			case CAR_RX_IV_ACK :
			{
				if (car_CheckAckRxd(uartRxBuf))
				{
					printf (" \n Car : got encryption IV received ack from remote \n");
					state = CAR_SEND_ENCRYPT_LEN;
				}
				break;
			}

			// car sends length of cipher text
			case CAR_SEND_ENCRYPT_LEN :
			{
				if (car_SendDataByte(lenCipherTextBuf,CAR_CIPHER_TEXT_BUFFER_LEN, car))
				{
					printf("\n Car: Encrypted message length sent \n");
					state = CAR_RX_ENCRYPT_LEN_ACK;
				}
				break;
			}

			case CAR_RX_ENCRYPT_LEN_ACK :
			{
				if (car_CheckAckRxd(uartRxBuf))
				{
					printf (" \n Car : got cipher text length received ack from remote \n");
					state = CAR_SEND_CIPHER_TEXT;
				}
				break;
			}

			//Car sends the encrypted data
			case CAR_SEND_CIPHER_TEXT :
			{
				if (car_SendDataByte(cipherTextBuffer,cipherTextBuflen, car))
				{
					printf("\n Car:  Encrypted message sent \n");
					state = CAR_RX_ACK_CIPHER_TEXT;
				}
				break;
			}

			//Car receives ack for the sent encrypted data
			case CAR_RX_ACK_CIPHER_TEXT :
			{
				if (car_CheckAckRxd(uartRxBuf))
				{
					printf(" \n Car : message decrypted successfully by remote \n");
					state = CAR_DEFAULT;
				}

				break;
			}

			case CAR_DEFAULT :
			{

				break;
			}

		} // end of switch
	} // end of while
} // end of main

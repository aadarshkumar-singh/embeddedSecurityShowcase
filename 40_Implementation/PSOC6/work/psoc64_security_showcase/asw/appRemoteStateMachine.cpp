/****************************************************************************
* Filename        : appRemoteStateMachine.cpp
* Author          : Aadarsh Kumar Singh
* Description     : Source file that defines APIs for the remote
* 					application
*
****************************************************************************/

#include "appRemoteStateMachine.h"

/***************************************************************************
 * Type definitions
 ***************************************************************************/

/**
 * \brief Describes the states of the remote state machine
 */
typedef enum remoteStates_e
{
	REMOTE_STATE_SEND_SIGNED_CREDENTIAL,//!< REMOTE_STATE_SEND_SIGNED_CREDENTIAL
	REMOTE_CHECK_SIGN_RXD_ACK,          //!< REMOTE_CHECK_SIGN_RXD_ACK
	REMOTE_SEND_PUB_KEY,                //!< REMOTE_SEND_PUB_KEY
	REMOTE_PUB_KEY_RXD_ACK,             //!< REMOTE_PUB_KEY_RXD_ACK
	REMOTE_CRED_VERIFIED_BY_CAR,        //!< REMOTE_CRED_VERIFIED_BY_CAR
	REMOTE_RX_CAR_CRED,                 //!< REMOTE_RX_CAR_CRED
	REMOTE_RX_CAR_PUB_KEY,              //!< REMOTE_RX_CAR_PUB_KEY
	REMOTE_VERIFY_CAR_AUTHENTICITY,     //!< REMOTE_VERIFY_CAR_AUTHENTICITY
	REMOTE_DH_KEY_SEND,                 //!< REMOTE_DH_KEY_SEND
	REMOTE_RX_DH_KEY_ACK,               //!< REMOTE_RX_DH_KEY_ACK
	REMOTE_RX_CAR_DH_KEY,               //!< REMOTE_RX_CAR_DH_KEY
	REMOTE_ENCRYPT_MESSAGE,             //!< REMOTE_ENCRYPT_MESSAGE
	REMOTE_ENCRYPT_FAIL,                //!< REMOTE_ENCRYPT_FAIL
	REMOTE_SEND_ENCRYPT_IV, 			//!< REMOTE_SEND_ENCRYPT_IV
	REMOTE_RX_IV_ACK,                   //!< REMOTE_RX_IV_ACK
	REMOTE_SEND_ENCRYPT_LEN,            //!< REMOTE_SEND_ENCRYPT_LEN
	REMOTE_RX_ENCRYPT_LEN_ACK,          //!< REMOTE_RX_ENCRYPT_LEN_ACK
	REMOTE_SEND_CIPHER_TEXT,            //!< REMOTE_SEND_CIPHER_TEXT
	REMOTE_RX_ACK_CIPHER_TEXT,          //!< REMOTE_RX_ACK_CIPHER_TEXT
	REMOTE_RX_CAR_IV,                   //!< REMOTE_RX_CAR_IV
	REMOTE_RX_CAR_CIPHER_LEN,           //!< REMOTE_RX_CAR_CIPHER_LEN
	REMOTE_RX_CAR_CIPHER_DATA,          //!< REMOTE_RX_CAR_CIPHER_DATA
	REMOTE_DECRYPT_CAR_CIPHER_TEXT,     //!< REMOTE_DECRYPT_CAR_CIPHER_TEXT
	REMOTE_CHECK_CAR_DECRYPTED_CIPHER,  //!< REMOTE_CHECK_CAR_DECRYPTED_CIPHER
	REMOTE_DEFAULT,                     //!< REMOTE_DEFAULT
}remoteStates_t;


/**
 * \brief Describes states of remote controller to receive the UART protocol messages
 */
typedef enum remote_rxStates_e
{
	REMOTE_RX_SOP, //!< REMOTE_RX_SOP
	REMOTE_RX_DATA,//!< REMOTE_RX_DATA
	REMOTE_RX_EOP1,//!< REMOTE_RX_EOP1
	REMOTE_RX_EOP2,//!< REMOTE_RX_EOP2
}remote_rxStates_t;

/**
 * \brief Describe states of remote controller to check if acknowledgment is sent by the receiver.
 */
typedef enum remote_rxAckStates_e
{
	REMOTE_RX_ACK_SOP,  //!< REMOTE_RX_ACK_SOP
	REMOTE_RX_ACK_BYTE1,//!< REMOTE_RX_ACK_BYTE1
	REMOTE_RX_ACK_BYTE2,//!< REMOTE_RX_ACK_BYTE2
}remote_rxAckStates_t;



/***************************************************************************
 * API definitions
 ***************************************************************************/

/**
 * \brief The cryptographic data like signature, keys, cipher text etc. are translated into
 * 		  UART Protocol in the below format :
 * 		  <SOF><Data Byte 1><Data Byte 2>...<EOP1><EOP2>
 *
 * @param inputBuffer     : Input Buffer containing the protocol
 * @param outputBuffer[o] : Output buffer that has to be transmitted
 * @param length	      : Length of protocol
 */

void remote_createtxPacket(uint8_t* inputBuffer, char * outputBuffer, int length)
{
	int outputIdx = 0;
	int inputIdx = 0;

	// First byte is Start of frame
	outputBuffer[outputIdx] = REMOTE_PROTOCOL_SOF ;

	// Data payload bytes
	for (outputIdx = 1; outputIdx <= length ; outputIdx++)
	{
		outputBuffer[outputIdx] = (char)inputBuffer[inputIdx++] ;
	}

	// Symbols for end of frame
	outputBuffer[outputIdx] = REMOTE_PROTOCOL_EOP1;
	outputBuffer[++outputIdx] = REMOTE_PROTOCOL_EOP2;
}

/**
 *
 * \brief Sends data byte by byte from the created UART protocol packet
 *
 * @param buffer : Buffer that has to be transmitted via UART
 * @param length : Length of buffer that has to be transmitted
 * @param remote : UART port used by remote for tx message
 *
 * @return true : When entire buffer is sent via UART
 * 		   false : Untill entire buffer is sent via UART
 */
bool remote_sendDataByte( char * buffer , uint8_t length, RawSerial &remote)
{
	static int bufIdx = 0;

	// checks if write buffer of UART is free to send data
	if(remote.writeable() != 0)
	{
		// sends one byte of data at a time until entire payload is sent
		remote.putc(buffer[bufIdx++]);
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
 * 		   acknowledgment message <$,*,*>, the sender checks if messages are received properly.

 * \note   Each transaction happens byte by byte.
 *
 * @param uartRxBuf : Ringbuffer in which UART data is stored
 * @return true : When Positive Acknowledgment is received (Received buffer contains <$,*,*>)
 * 		   false : When negative Acknowledgment is received
 */
bool remote_checkAckRxd(CRingBuffer &uartRxBuf)
{
	static char ch = '\0';
	static remote_rxAckStates_t state = REMOTE_RX_ACK_SOP;

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
		case REMOTE_RX_ACK_SOP:
		{
			if(ch == REMOTE_PROTOCOL_SOF)
				state = REMOTE_RX_ACK_BYTE1;
			break;
		}
		// check if first byte of acknowledgment payload (*) is received
		case REMOTE_RX_ACK_BYTE1:
		{
			if (ch == REMOTE_POS_ACK_BYTE_1)
			{
				state = REMOTE_RX_ACK_BYTE2;
			}
			else
			{
				state = REMOTE_RX_ACK_SOP;
			}
			break;
		}

		// check if second byte of acknowledgment payload (*) is received
		case REMOTE_RX_ACK_BYTE2:
		{
			if(ch == REMOTE_POS_ACK_BYTE_2)
			{
				state = REMOTE_RX_ACK_SOP;
				return true;
			}
			else
			{
				state = REMOTE_RX_ACK_SOP;
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
bool remote_rxDataByte(unsigned char* rxdData, unsigned int dataLength, CRingBuffer &uartRxBuf)
{
	static unsigned int dataIdx = 0;
	static char ch = '\0';
	static remote_rxStates_t state = REMOTE_RX_SOP;

	// Statemachine that receives data byte by byte,
	// data between the <SOF> byte and <EOP1,EOP2> bytes are stored in the receive buffer
	// Discards the message if expected protocol is not received and returns false
	if (uartRxBuf.read(ch) == RC_SUCCESS)
	{
		switch (state)
		{
			// Checks the start of payload
			case REMOTE_RX_SOP:
			{
				if(ch == REMOTE_PROTOCOL_SOF)
				{
					state = REMOTE_RX_DATA;
				}

				break;
			}

			// Received UART data between <SOF> and <EOP1,EOP2> is stored
			case REMOTE_RX_DATA:
			{
				rxdData[dataIdx++] = ch;

				if (dataIdx == dataLength)
				{
					dataIdx = 0;
					state = REMOTE_RX_EOP1;
				}

				break;
			}

			// Checks if received character is first byte of end of payload
			case REMOTE_RX_EOP1:
			{
				if(ch == REMOTE_PROTOCOL_EOP1)
				{
					state = REMOTE_RX_EOP2;
				}
				else
				{
					state = REMOTE_RX_SOP;
				}
				break;
			}

			// Checks if received char is second byte of end of payload
			case REMOTE_RX_EOP2:
			{
				if(ch == REMOTE_PROTOCOL_EOP2)
				{
					state = REMOTE_RX_SOP;
					return true;
				}
				else
				{
					state = REMOTE_RX_SOP;
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

	return false;
}


/**
 * \brief API to send positive acknowledgment <$,*,*> when messages are received correctly.
 * @param remote : UART port used by remote for tx message
 */
void remote_sendAck(RawSerial &remote)
{
	// Positive acknowledgment payload
	char remoteAck[3] = {REMOTE_PROTOCOL_SOF , REMOTE_POS_ACK_BYTE_1 , REMOTE_POS_ACK_BYTE_2};

	for (int i = 0 ; i< 3 ; i++)
	{
		remote.putc(remoteAck[i]);
	}

	printf("     Ack Sent.\n  ");
}

/**
 * \brief Generates signature and public key for the remote.
 * 		  The generated signature and public key is packetized into UART protocol
 * 		  and returned.
 * @param signBuf [o]: returns buffer containing signature
 * @param pubKeyBuf[o] : returns Buffer containing Public key
 */
void remote_generateSignAndPubKey(char *signBuf, char *pubKeyBuf)
{
	// Unique ID - Credential of the remote controller
	uint8_t remoteId[] = {"I am 01234567891"};

	// structure to store public key and signature
	uartData_t remoteDataToTx ;

	// Initialization of signature Manager to sign remote credentials
	signMgr remoteSignDataToTxd(remoteId);

	// Generate hash
	remoteSignDataToTxd.signhash();

	// Export the generated public key
	remoteDataToTx = remoteSignDataToTxd.export_PublicKey();

	// Create UART protocol for transmission of signature and public key
	remote_createtxPacket(remoteDataToTx.signature,signBuf,SIGNATURE_LENGTH);
	remote_createtxPacket(remoteDataToTx.publicKey,pubKeyBuf,PUB_KEY_LENGTH);
}


/**
 * \brief Generates Diffie Hellmann Context to generate Diffie Hellmann public key
 * 		  that has to be exchanged with the receiver.
 *
 * @param remoteDhCtx         : Diffie Hellmann Context
 * @param remoteDhPubKey[o]   : Generated Diffie Hellmann public key
 * @param DhPubKeyBuffer[o]   : Buffer containing UART protocol of DH public key to be transmitted
 */
void remote_generateDhPubKey(dhKeyExchangeMgr &remoteDhCtx,uint8_t *remoteDhPubKey, char *DhPubKeyBuffer)
{
	// Generate Diffie Hellmann Public key for exchange
	remoteDhCtx.generateDhKey(remoteDhPubKey);

	// Create UART protocol for transmission of public key
	remote_createtxPacket(remoteDhPubKey,DhPubKeyBuffer,REMOTE_DH_PUB_KEY_SIZE);
}

/**
 * \brief Computes the shared secret key using Diffie Hellmann Context and the the DH public key that is
 * 		  generated.
 *
 * @param remoteDhCtx            : Diffie Hellmann Context
 * @param remoteRxdCarDhPubKey   : Generated Diffie Hellmann public key
 * @param remoteSharedEncKey [o] : DH Secret Key that is computed
 * @return : true  : If shared secret key is computed successfully.
 * 		   : false : If computation of shared secret key fails.
 */
bool remote_computeSharedSecretKey(dhKeyExchangeMgr &remoteDhCtx , uint8_t* remoteRxdCarDhPubKey , uint8_t* remoteSharedEncKey)
{
	// Computes the shared diffie hellmann secret key for encryption and decryption
	if ( KEYEXCHANGE_INIT_SUCCESS == remoteDhCtx.computeSharedSecretDhKey(remoteRxdCarDhPubKey, remoteSharedEncKey))
	{
		return true ;
	}
	else
	{
		return false ;
	}
}


/**
 * \brief API which contains statemachine logic for remote controller application
 * @param remote    : UART port used by remote for tx/rx message
 * @param uartRxBuf : UART Messages that are received are stored in this ringbuffer
 *
 * \note - size of ringbuffer is 500 bytes
 * 		 - Maximum Length of plain text that can be encrypted at once is 255.
 * 		 - Hardware crypto initialization must be done before initialization of
 * 		   application service manager(encryption,signature,DH)
 */
void remote_application(RawSerial &remote,CRingBuffer &uartRxBuf)
{
	/* remote application states */
	remoteStates_t state = REMOTE_STATE_SEND_SIGNED_CREDENTIAL;

	/* variables used for signature generation and verification */
	uartData remoteVerifyCarSign; 					  // struct to verify the received car signature
	char signatureBuffer[SIGNATURE_BUFFER_LENGTH];    // stores UART protocol containing signature of remote
	char signPubKeyBuffer[SIGNATURE_PUB_KEY_BUF_LEN]; // stores UART protocol containing public key(signature) of remote

	/* variables used for Diffie Hellmann shared secret key exchange */
	char remoteDhPubKeyBuffer[REMOTE_DH_PUB_KEY_BUF_LEN]; 		// stores UART protocol containing DH public key of remote
	unsigned char remoteRxdDhPubKeyBuf[REMOTE_DH_PUB_KEY_SIZE]; // stores UART protocol containing DH public key of car.
	static dhKeyExchangeMgr remoteDhCtx; 					    // Diffie Hellmann context for remote controller
	static uint8_t remoteDhPubKey[REMOTE_DH_PUB_KEY_SIZE];		// remote's DH public key
	static uint8_t remoteRxdCarDhPubKey[REMOTE_DH_PUB_KEY_SIZE];// received DH public key of car
	static uint8_t remoteSharedEncKey[REMOTE_ENC_KEY_SIZE];		// remote shared secret key

	/* Initialization of Hardware crypto module */
	if (psa_crypto_init() != PSA_SUCCESS)
	{
		printf("Failed to initialize PSA Crypto\n");

	}

	/* variables used for encryption */
	uint8_t plainText[]= {"I am plaintext.1234"};		  // Example plain text to be encrypted by remote and sent to car
	encryptMgr encryptObj (plainText,sizeof(plainText),ENCRYPT_CBC_WITH_PADDING);	// Initialization of Encryption Manager
	char *cipherTextBuffer; 							  //stores text encrypted by remote
	char remoteIvTxBuf[REMOTE_IV_BUFFER_LEN]; 			  // stores UART protocol containing remote's Initialization vector
	char lenCipherTextBuf[REMOTE_CIPHER_TEXT_BUFFER_LEN]; // stores UART protocol containing length of remote's cipher text
	uint8_t cipherTextlen =0; 							  // Maximum Length of plain text that can be encrypted at once is 255.
	uint8_t cipherTextBuflen =0; 						  // Length of UART protocol buffer containing length of cipher text

	/* variables used for decryption */
	char remoteRxdCarIvBuf[16]; 			 			  // stores received car's Initialization vector
	unsigned char remoteRxdCarCipherLenBuf;				  // stores received length of car's cipher text
	char* remoteRxdCipherTextOfCar = 0;					  // stores received cipher text of car.

	// Generate signature of remote controller
	remote_generateSignAndPubKey(signatureBuffer,signPubKeyBuffer);

	// Generate diffie Hellmann public key for shared secret computation
	remote_generateDhPubKey(remoteDhCtx,remoteDhPubKey,remoteDhPubKeyBuffer);

	while(true)
	{
		/* Waiting for sync between tx side and rx side, tx side should send slowly
		   so that rx buffer is not filled up faster than it should. */
		wait_us(1000);

		// Statemachine of remote application
		switch(state)
		{

		// For car to authenticate the remote , remote sends it signed credentials.
		case REMOTE_STATE_SEND_SIGNED_CREDENTIAL:
		{
			/* sends UART protocol containing signature of remote byte by byte
			   remote remains in this state until entire protocol with signed
			   credentials is sent (blocking call) */

			if (remote_sendDataByte(signatureBuffer,SIGNATURE_BUFFER_LENGTH, remote))
			{
				printf(" \n Remote : Sent signature to car \n ");
				state = REMOTE_CHECK_SIGN_RXD_ACK;
			}
			break;
		}

		// remote checks if car sent positive acknowledgment after receiving its signed credentials
		case REMOTE_CHECK_SIGN_RXD_ACK:
		{
			// remote remains in this state until positive acknowledgment protocol is received.
			if (remote_checkAckRxd(uartRxBuf))
			{
				printf (" \n Remote : got signature received acknowledgment from car \n");
				printf (" \n Remote : sending  public Key for verifying signature \n");
				state = REMOTE_SEND_PUB_KEY;
			}
			break;
		}

		// For car to verify the signed remote credentials , remote sends its public key.
		case REMOTE_SEND_PUB_KEY :
		{

			/* sends UART protocol containing public key of remote byte by byte (used for verifying sign)
			   remote remains in this state until entire protocol with public key is sent (blocking call) */

			if (remote_sendDataByte(signPubKeyBuffer,SIGNATURE_PUB_KEY_BUF_LEN, remote))
			{
				printf(" \n Remote : Sent public key to car  \n ");
				state = REMOTE_PUB_KEY_RXD_ACK;
			}
			break;
		}

		// remote checks if car sent positive acknowledgment after receiving its public key
		case REMOTE_PUB_KEY_RXD_ACK :
		{
			// remote remains in this state until positive acknowledgment protocol is received.
			if (remote_checkAckRxd(uartRxBuf))
			{
				printf (" \n Remote : got public key received acknowledgment from car \n");
				state = REMOTE_CRED_VERIFIED_BY_CAR;
			}

			break;
		}

		// Remote checks if its credentials are authenticated by car
		case REMOTE_CRED_VERIFIED_BY_CAR :
		{
			/* On authentication of remote's signed credential ,car sends positive acknowledgment
			   remote checks for this positive acknowledgment frame , remains in this state
			   until authenticated by car (blocking call) */

			if (remote_checkAckRxd(uartRxBuf))
			{
				printf (" \n Remote : signature verified by car \n");
				state = REMOTE_RX_CAR_CRED;
			}
			break;
		}

		// For remote to authenticate car , remote receives car's signed credentials.
		case REMOTE_RX_CAR_CRED:
		{
			/* receives UART protocol containing signature of car byte by byte
			   remote remains in this state until entire protocol with signed credentials is received (blocking call)
			   When complete protocol is received it sends positive acknowledgment frame to car. */

			if(remote_rxDataByte((unsigned char *)remoteVerifyCarSign.signature, remoteVerifyCarSign.signature_length, uartRxBuf))
			{
				printf(" \n Remote : received  car signature : ");
				for (int i = 0 ; i< ECDSA_SIGN_LENGTH ; i++)
					printf("%c",remoteVerifyCarSign.signature[i]);

				printf("\n");

				remote_sendAck(remote);
				state = REMOTE_RX_CAR_PUB_KEY;
			}
			break;
		}

		// For remote to verify the authenticity of car , remote receives car's public key.
		case REMOTE_RX_CAR_PUB_KEY:
		{
			/* receives UART protocol containing public key of car byte by byte (used for verifying sign)
			   remote remains in this state until entire protocol with public key is received (blocking call)
			   When complete protocol is received it sends positive acknowledgment frame to car. */

			if(remote_rxDataByte((unsigned char *)remoteVerifyCarSign.publicKey, remoteVerifyCarSign.publiKey_length, uartRxBuf))
			{
				printf (" \n Remote : received car public key  : ");

				for (int i = 0 ; i< ECDSA_PUB_KEY_LENGTH ; i++)
					printf("%c",remoteVerifyCarSign.publicKey[i]);

				printf("\n");

				remote_sendAck(remote);
				state = REMOTE_VERIFY_CAR_AUTHENTICITY;
			}
			break;
		}

		// remote verifies the authenticity of car using the received public key and signature
		case REMOTE_VERIFY_CAR_AUTHENTICITY :
		{
			/* Initialization of signature verification manager using the received public key and signature
			   Until the car credentials are authenticated it remains in this state(blocking call)
			   sends positive acknowledgment frame after successful authentication of car. */

			signVerifyMgr signVerifyObj(remoteVerifyCarSign);
			if(signVerifyObj.verifySignature())
			{
				printf(" \n Remote : verified car signature \n ");
				remote_sendAck(remote);
				state = REMOTE_DH_KEY_SEND;
			}
			break;
		}

		// remote sends its diffie Hellmann public key for shared secret key computation
		case REMOTE_DH_KEY_SEND:
		{
			/* sends UART protocol containing DH public key of remote byte by byte (used for shared secret key computation)
			   remote remains in this state until entire protocol with DH public key is sent (blocking call) */

			if (remote_sendDataByte(remoteDhPubKeyBuffer,REMOTE_DH_PUB_KEY_BUF_LEN, remote))
			{
				printf("\n Remote : deffie helmann public Key sent \n");
				state = REMOTE_RX_DH_KEY_ACK;
			}
			break;
		}

		// remote checks if car sent positive acknowledgment after receiving its DH public key
		case REMOTE_RX_DH_KEY_ACK:
		{
			// remote remains in this state until positive acknowledgment protocol is received.
			if (remote_checkAckRxd(uartRxBuf))
			{
				printf (" \n Remote : got deffie helmann public key received ack from car \n");
				state = REMOTE_RX_CAR_DH_KEY;
			}
			break;
		}

		// remote receives car's diffie Hellmann public key for shared secret key computation
		case REMOTE_RX_CAR_DH_KEY:
		{
			/* receives UART protocol containing DH public key of car byte by byte (used for shared secret key computation)
			   remote remains in this state until entire protocol with DH public key is received (blocking call)
			   When complete protocol is received it sends positive acknowledgment frame to car. */

			if( remote_rxDataByte( remoteRxdDhPubKeyBuf, REMOTE_DH_PUB_KEY_SIZE, uartRxBuf))
			{
				printf (" \n Remote : received deffie helmann public key of car : ");
				for (int i = 0 ; i< 32 ; i++)
				{
					printf("%c", remoteRxdDhPubKeyBuf[i]);
					remoteRxdCarDhPubKey[i] = (uint8_t)remoteRxdDhPubKeyBuf[i];
				}
				printf("\n");

				remote_sendAck(remote);
				state = REMOTE_ENCRYPT_MESSAGE;
			}
			break;
		}

		// remote encrypts the message to be transmitted using the diffie Hellmann shared secret key
		case REMOTE_ENCRYPT_MESSAGE:
		{
			/* remote computes DH shared secret key using its DH context (remote's DH public key for
			   exchange was generated using this) and received DH public key of car
			   On successfull computation of secret key , it starts encrypting the message */

			if (remote_computeSharedSecretKey(remoteDhCtx , remoteRxdCarDhPubKey , remoteSharedEncKey ))
			{
				printf(" \n Remote : computed shared secret key successfully :  ");
				for (int i = 0 ; i< 16 ; i++)
				{
					printf("%d", remoteSharedEncKey[i]);
				}
				printf("\n");

				// Computed shared secret key is used for encryption
				if (encryptObj.encryptionDhKeyHandle(remoteSharedEncKey))
				{
					//printf("\n Key Handle Encrypt DH Success \n ");

					// Encryption is performed , cipher text, IV and length is stored on success
					if ( ENCRYPT_SUCCESS == encryptObj.encryptUsingSymmetricCiphers())
					{
						printf(" \n Remote : message encrypted successfully \n");

						// Length of cipher text is computed
						cipherTextlen = encryptObj.sizeofEncryption() ;

						// UART protocol buffer length is calculated
						cipherTextBuflen = cipherTextlen + PROT_ID_LENGTH ;

						// Buffer to store cipher text is created
						cipherTextBuffer = new char[cipherTextBuflen];

						// UART protocol frame for cipher text, IV and its length is created
						remote_createtxPacket(encryptObj.GetEncryptedData(),cipherTextBuffer, encryptObj.sizeofEncryption());
						remote_createtxPacket(encryptObj.GetIvData(), remoteIvTxBuf,AES_BLOCK_SIZE_SUPPORTED_BY_HW);
						remote_createtxPacket(&cipherTextlen,lenCipherTextBuf,1);
						state = REMOTE_SEND_ENCRYPT_IV;
					}
					// On failure debug print is provided to the user.
					else
					{
						printf("Encryption Failure\n");
						state = REMOTE_ENCRYPT_FAIL;
					}

				}
			}
			break;
		}

		// If Encryption fails it remains in this state.
		case REMOTE_ENCRYPT_FAIL :
		{
			break;
		}

		/* Computed IV of remote is sent to car . The IV is generated randomly every time encryption is performed.
		   In this example we encrypt a simple plain text message only once.
		   Also at a time maximum of 255 bytes can be encrypted. */
		case REMOTE_SEND_ENCRYPT_IV:
		{
			/* sends UART protocol containing IV of remote byte by byte (used for encryption)
			   remote remains in this state until entire protocol with IV is sent (blocking call) */

			if (remote_sendDataByte(remoteIvTxBuf,REMOTE_IV_BUFFER_LEN, remote))
			{
				printf("\n Remote :  Encryption IV Sent \n");
				state = REMOTE_RX_IV_ACK;
			}
			break;
		}
		// remote checks if car sent positive acknowledgment after receiving its IV
		case REMOTE_RX_IV_ACK :
		{
			// remote remains in this state until positive acknowledgment protocol is received.
			if (remote_checkAckRxd(uartRxBuf))
			{
				printf (" \n Remote : got encryption IV received ack from car \n");
				state = REMOTE_SEND_ENCRYPT_LEN;
			}
			break;
		}

		// remote sends length of cipher text
		case REMOTE_SEND_ENCRYPT_LEN :
		{
			/* sends UART protocol containing remote's cipher text length byte by byte
			   remote remains in this state until entire protocol with length is sent (blocking call) */

			if (remote_sendDataByte(lenCipherTextBuf,REMOTE_CIPHER_TEXT_BUFFER_LEN, remote))
			{
				printf("\n Remote: Encrypted message length sent \n");
				state = REMOTE_RX_ENCRYPT_LEN_ACK;
			}
			break;
		}

		// remote checks if car sent positive acknowledgment after receiving the length of cipher text
		case REMOTE_RX_ENCRYPT_LEN_ACK :
		{
			// remote remains in this state until positive acknowledgment protocol is received.
			if (remote_checkAckRxd(uartRxBuf))
			{
				printf (" \n Remote : got cipher text length received ack from car \n");
				state = REMOTE_SEND_CIPHER_TEXT;
			}
			break;
		}

		// remote sends cipher text
		case REMOTE_SEND_CIPHER_TEXT :
		{
			/* sends UART protocol containing remote's cipher text byte by byte
			   remote remains in this state until entire protocol with cipher text is sent (blocking call) */

			if (remote_sendDataByte(cipherTextBuffer,cipherTextBuflen, remote))
			{
				printf("\n Remote:  Encrypted message sent \n");
				state = REMOTE_RX_ACK_CIPHER_TEXT;
			}
			break;
		}

		// remote checks if car sent positive acknowledgment after receiving cipher text
		case REMOTE_RX_ACK_CIPHER_TEXT:
		{
			// remote remains in this state until positive acknowledgment protocol is received.
			if (remote_checkAckRxd(uartRxBuf))
			{
				printf (" \n Remote : got encryption text received ack from car \n");
				state = REMOTE_CHECK_CAR_DECRYPTED_CIPHER;
			}

			break;
		}

		// remote checks if the car has decrypted its data successfully
		case REMOTE_CHECK_CAR_DECRYPTED_CIPHER :
		{
			// remote checks if it received positive acknowledgment frame from car upon successful decryption.
			if (remote_checkAckRxd(uartRxBuf))
			{
				printf(" \n Remote : message decrypted successfully by car \n");
				state = REMOTE_RX_CAR_IV;
			}

			break;
		}

		// remote receives car's IV for performing decryption of car's encrypted message
		case REMOTE_RX_CAR_IV :
		{
			/* receives UART protocol containing IV of car byte by byte (used for decryption of car's message )
			   remote remains in this state until entire protocol with IV is received (blocking call)
			   When complete protocol is received it sends positive acknowledgment frame to car. */

			if(remote_rxDataByte((unsigned char *)remoteRxdCarIvBuf, 16, uartRxBuf))
			{
				printf (" \n Remote : received encryption IV of car : ");
				for (int i = 0 ; i< 16 ; i++)
				{
					printf("%c",remoteRxdCarIvBuf[i]);
				}
				printf("\n");

				remote_sendAck(remote);
				state = REMOTE_RX_CAR_CIPHER_LEN;
			}
			break;
		}

		// remote receives car's cipher text length for performing decryption of car's encrypted message
		case REMOTE_RX_CAR_CIPHER_LEN :
		{
			/* receives UART protocol containing car's cipher text length byte by byte (used for decryption of car's message )
			   remote remains in this state until entire protocol with cipher text length is received (blocking call)
			   When complete protocol is received it sends positive acknowledgment frame to car. */

			if(remote_rxDataByte(&remoteRxdCarCipherLenBuf, 1, uartRxBuf))
			{
				printf (" \n Remote : received encrypted message length of car : ");
				printf("%c",remoteRxdCarCipherLenBuf);
				printf("\n");

				remote_sendAck(remote);
				remoteRxdCipherTextOfCar = new char[(uint8_t)remoteRxdCarCipherLenBuf];
				state = REMOTE_RX_CAR_CIPHER_DATA;
			}
			break;
		}

		// remote receives car's cipher text for decryption
		case REMOTE_RX_CAR_CIPHER_DATA :
		{
			/* receives UART protocol containing car's cipher text byte by byte (for decryption)
			   remote remains in this state until entire protocol with cipher text is received (blocking call)
			   When complete protocol is received it sends positive acknowledgment frame to car. */

			if(remote_rxDataByte((unsigned char *)remoteRxdCipherTextOfCar, (uint8_t)remoteRxdCarCipherLenBuf, uartRxBuf))
			{
				printf (" \n Remote : received encrypted message of car : ");
				for (int i = 0 ; i< (uint8_t)remoteRxdCarCipherLenBuf ; i++)
					printf("%c",remoteRxdCipherTextOfCar[i]);
				printf("\n");

				remote_sendAck(remote);
				state = REMOTE_DECRYPT_CAR_CIPHER_TEXT ;
			}

			break;
		}

		// remote decrypts the encrypted message using the DH shared secret key, IV and length of cipher text
		case REMOTE_DECRYPT_CAR_CIPHER_TEXT :
		{
			if (psa_crypto_init() != PSA_SUCCESS)
			{
				printf("Failed to initialize PSA Crypto\n");
			}

			printf(" \n Remote : Decrypting with the computed shared secret key : \n");


			/* remote decrypts the encrypted message using the DH shared secret key, IV and length of cipher text
			   upon decryption sends positive acknowledgment frame.*/

			decryptMgrTest_TestCase_EncryptedData((uint8_t*)remoteRxdCipherTextOfCar,
					(uint8_t)remoteRxdCarCipherLenBuf, (uint8_t*)remoteRxdCarIvBuf,
					remoteSharedEncKey);
			remote_sendAck(remote);
			state = REMOTE_DEFAULT;
			break;
		}

		// After Decryption is performed it remains in this default state
		case REMOTE_DEFAULT:
		{
			static uint8_t flag = 1 ;
			if (flag)
			{
				printf(" \n Remote : message received from car decrypted successfully \n ");
				flag = 0;
			}

			break;
		}

		} // end of switch

	} // end of while

} // end of remote application

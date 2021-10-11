/****************************************************************************
* Filename        : decryptMgr.h
* Author          : Vaishnavi Sankaranarayanan
* Description     : Header file that declares APIs supported for AES block mode
* 				    of Decryption.
* Created on	  : Jun 28, 2020
****************************************************************************/

#ifndef ASW_SERVICES_DECRYPTION_MANAGER_DECRYPTMGR_H_
#define ASW_SERVICES_DECRYPTION_MANAGER_DECRYPTMGR_H_


/***************************************************************************
 * Includes
 ***************************************************************************/
#include "psa/crypto.h"
#include "mbedtls/version.h"
#include "mbed.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "decryptServicesCfg.h"


/**
 * \brief Decryption Manager for providing services for Decrypting using
 * 		  AES Decryption
 */
class decryptMgr {


		/**
		 * \brief Buffer to store encrypted message
		 */
		uint8_t* m_cipherEncryptedData ;
		/**
		 * \brief Size of Decrypted text
		 */
		size_t m_sizeOfDecryptedText;

		/**
		 * \brief Size of encrypted Message
		 */
		volatile uint16_t m_sizeOfEncryptedData;
		/*
		 * \brief Size of IV
		 */


		/**
		 * \brief Size of one encryption block
		 */
		uint8_t  m_blockSize;

		/**
		 * \brief Initialisation Vector of AES block size
		 */
		uint8_t  m_iv[AES_BLOCK_SIZE_SUPPORTED_BY_HW];

		/**
		 * #brief Key used for decryption
		 */
		uint8_t m_decryptionKey[AES_DECRYPTION_KEY_SIZE];


		 /* \brief Buffer to store message that has been decrypted
		 */
		uint8_t *m_DecryptedText ;

		/**
		 * \brief Type of AES Algorithm supported by user application
		 */
		volatile typeOfDecryptAlgo_t m_algoType;
	
		/**
		 * \brief Error status of Decryption operation
		 */
		errorTypeDecrypt_t m_errorStatus;
		
		/**
		* \brief AES-Algorithm supported by PSA architecture
		*/
		psa_algorithm_t m_algSupportedByPSA ;
		
		/**
		* \brief initialization attributes for performing decryption.
		* 		  Used for creating key handle
		*/
	    psa_key_attributes_t m_attributes ;

		/**
		* \brief Key Handle for decryption
		*/
	    psa_key_handle_t m_handle;
		
		/**
		* \brief structure that stores multipart cipher operations
		*/
	    psa_cipher_operation_t m_operation ;

		/**
		 * \brief  Initialization of Decryption algorithm
		 * @return Error status:
		 * 		   ENCRYPT_VALID_ALGO : Algorithm supported by user application
		 * 		   ENCRYPT_INVALID_ALGO : Algorithm not supported by user application
		 * \note   - Length of the Encrypted text is always padded ,
		 *			 This is handled by the encryption manager and always a padded text
		 * 			 is receieved and hence the size of the encrypted text will be divisible
		 *			 by the block size.
		 * 		   - Padding algorithm supported is PKCS7
		 */
	    errorTypeDecrypt_t setDecryptionAlgo();
		
		/**
		* \brief  Calculates size of a decryption block
		* @return Error status:
		* 		   DECRYPT_VALID_BLOCK_SIZE : Valid Block size for AES Decryption
		* 		   DECYPT_INVALID_BLOCK_SIZE : Block size not supported by h/w for AES Decryption
		*/
	    errorTypeDecrypt_t calculateBlockSize();
		
		/**
		* \brief  Calculates size of a decrypted data buffer
		* @return Error status:
		* 		   DECRYPT_PADDING_SUCCESS : when buffer size to store Decrypted values is proper
		* 		   DECRYPT_PADDING_FAIL : when buffer size to store Decrypted values is not proper
		*/
	    errorTypeDecrypt_t calculateSizeOfDecryptedText();

		/**
		* \brief  Creates key handle for Decryption
		* @return Error status:
		* 		   DECRYPT_HANDLE_SUCCESS : Valid key handle for AES Decryption
		* 		   DECRYPT_HANDLE_FAIL : Invalid key handle for AES Decryption
		*/
		
	    errorTypeDecrypt_t createKeyHandleForDecryption(uint8_t * decryptionKey);
		
		/**
		 * \brief  Initializes all the data structure essential for
		 * 		   performing decryption
		 * \note  The key handle is created with default AES decryption key
		 * 		  so that the manager supports AES- decryption for application which
		 * 		  does not use Diffie Hellmann key exchange algorithm for deriving
		 * 		  the shared secret key between the sender and receiver
		 * @return void
		 */
	   	void initializeDecryptionServices();

public:
		/**
		* \brief Constructor for the Decrypt Manager
		*
		* @param m_cipherEncryptedData : Encrypted data buffer
		* @param m_Iv : IV used for Encryption
		* @param sizeOfEncryptedData : Size of the Encrypted data
		* @param algoType:AES- Algorithim used for encryption.
		*/
	   	decryptMgr(uint8_t* m_cipherEncryptedData ,uint8_t* m_Iv, uint16_t sizeOfEncryptedData, typeOfDecryptAlgo_t algoType);
		
		/**
		* \brief Performs symmetric decryption
		* @return DECRYPT_SUCCESS : When decryption is successful
		* 		   DECRYPT_CIPHER_SETUP_FAIL : When decryption setup fails
		* 		   DECRYPT_IV_GENERATE_FAIL : When Initialisation vector is not generated
		* 		   DECRYPT_CIPHER_UPDATE_FAIL : when updation of decryption buffer fails
		* 		   DECRYPT_CIPHER_NOT_FINISHED : When multipart decryption is not finished
		*/
	   	errorTypeDecrypt_t DecryptUsingSymmetricCiphers();
		/**
		 * \brief Used to create Key handle for diffie hellman keys
		 * @param dhkey : DH key
		 * @return true : if key handle is successfully created
		 * 		   false : On failure of creating key handle
		 */
		bool decryptionDhKeyHandle(uint8_t *dhKey);

		/**
		 *  \brief Displays the decrypted message in UART
		 *  \note  used for debug logs
		 */	
	   	void displayDecryptedMessage();
		
		/**
		* \brief Destructor
		* 		  - Frees the Decrypted data buffer created in runtime
		* 		  - Destroy the key handle
		* 		  - Frees the data structures used by mbed tls crypto library
		*/
	   	virtual ~decryptMgr();

};

#endif /* ASW_SERVICES_DECRYPTION_MANAGER_DECRYPTMGR_H_ */

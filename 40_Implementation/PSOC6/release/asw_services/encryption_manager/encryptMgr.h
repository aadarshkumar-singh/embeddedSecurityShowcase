/****************************************************************************
* Filename        : encryptMgr.h
* Author          : Aadarsh Kumar Singh
* Description     : Header file that declares APIs supported for AES block mode
* 				    of encryption.
* Created on	  : Jun 6, 2020
****************************************************************************/

#ifndef ASW_SERVICES_ENCRYPTION_MANAGER_ENCRYPTMGR_H_
#define ASW_SERVICES_ENCRYPTION_MANAGER_ENCRYPTMGR_H_

/***************************************************************************
 * Includes
 ***************************************************************************/
#include "psa/crypto.h"
#include "mbedtls/version.h"
#include "mbed.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "encryptServicesCfg.h"

/**
 * \brief Encryption Manager for providing services for Encrypting using
 * 		  AES Encryption
 *
 * \note  - It supports AES block cipher mode of encryption. Encryption is performed
 * 	        block by block not as stream.
 */
class encryptMgr {

	/**
	 * \brief Buffer to store message that has to be encrypted
	 */
	uint8_t* m_plainText ;

	/**
	 * \brief Buffer to store encrypted message
	 */
	uint8_t* m_cipherText ;

	/**
	 * \brief Size of plain text
	 */
	volatile uint16_t m_sizeOfPlainText;

	/**
	 * \brief Size of Cipher text
	 */
	volatile uint16_t m_sizeOfCipherText;

	/**
	 * \brief Size of a encryption block
	 *
	 * \note Always 16 bytes irrespective of type of AES-Algo
	 */
	uint8_t  m_blockSize;

	/**
	 * \brief Initialisation Vector for Encryption
	 * \note Size must be same as block size,
	 * 		 We use PSoC6 board it supports AES_BLOCK_SIZE_SUPPORTED_BY_HW
	 * 		 size
	 *
	 */
	uint8_t  m_iv[AES_BLOCK_SIZE_SUPPORTED_BY_HW];

	/**
	 * #brief Key used for encryption
	 */
	uint8_t m_encryptionKey[AES_ENCRYPTION_KEY_SIZE];

	/**
	 * \brief Type of AES Algorithm supported by user application
	 */
	volatile typeOfEncryptAlgo_t m_algoType;

	/**
	 * \brief Error status of Encryption operation
	 */
	errorTypeEncrypt_t m_errorStatus;

	/**
	 * \brief AES-Algorithm supported by PSA architecture
	 */
	psa_algorithm_t m_algSupportedByPSA ;

	/**
	 * \brief initialization attributes for performing encryption.
	 * 		  Used for creating key handle
	 */
    psa_key_attributes_t m_attributes ;

    /**
     * \brief Key Handle for encryption
     */
    psa_key_handle_t m_handle;

    /**
     * \brief structure that stores multipart cipher operations
     */
    psa_cipher_operation_t m_operation ;

    /**
     * \brief  Initialization of Encryption algorithm
     * @return Error status:
     * 		   ENCRYPT_VALID_ALGO : Algorithm supported by user application
     * 		   ENCRYPT_INVALID_ALGO : Algorithm not supported by user application
     */
    errorTypeEncrypt_t setEncryptionAlgo();

    /**
     * \brief  Calculates size of a encryption block
     * @return Error status:
     * 		   ENCRYPT_VALID_BLOCK_SIZE : Valid Block size for AES Encryption
     * 		   ENCYPT_INVALID_BLOCK_SIZE : Block size not supported by h/w for AES Encryption
     */
    errorTypeEncrypt_t calculateBlockSize();

    /**
     * \brief  Calculates size of a encrypted data buffer
     * @return Error status:
     * 		   ENCRYPT_PADDING_SUCCESS : when buffer size to store encrypted values is proper
     * 		   ENCRYPT_PADDING_FAIL : when buffer size to store encrypted values is not proper
     */
    errorTypeEncrypt_t calculateSizeOfCipherText();

    /**
     * \brief  Creates key handle for Encryption
     * @return Error status:
     * 		   ENCRYPT_HANDLE_SUCCESS : Valid key handle for AES Encryption
     * 		   ENCRYPT_HANDLE_FAIL : Invalid key handle for AES Encryption
     */
    errorTypeEncrypt_t createKeyHandleForEncryption(uint8_t * encryptionKey);

    /**
     * \brief  Initializes all the data structure essential for
     * 		   performing encryption
     * @return void
     */
	void initializeEncryptionServices();

public:
	/**
	 * \brief Constructor for the encryption Manager
	 *
	 * @param plainText : address of the Input plain text buffer
	 * @param sizeOfPlainText : size of the input plain text buffer
	 * @param algoType : AES- Algorithim used for encryption
	 */
	encryptMgr(uint8_t* plainText , uint16_t sizeOfPlainText, typeOfEncryptAlgo_t algoType);

	/**
	 * \brief Performs symmetric encryption
	 * @return ENCRYPT_SUCCESS : When encryption is successful
	 * 		   ENCRYPT_CIPHER_SETUP_FAIL : When encryption setup fails
	 * 		   ENCRYPT_IV_GENERATE_FAIL : When Initialisation vector is not generated
	 * 		   ENCRYPT_CIPHER_UPDATE_FAIL : when updation of encrypted buffer fails
	 * 		   ENCRYPT_CIPHER_NOT_FINISHED : When multipart encryption is not finished
	 */
	errorTypeEncrypt_t encryptUsingSymmetricCiphers();

	/**
	 *  \brief Displays the encrypted message in UART
	 */
	void displayEncryptedMessage();

	/**
	 *
	 * \brief  Used to create Key handle for diffie hellman keys
	 *
	 * @param dhKey : Deffie Hellmann shared secret key
	 * @return true : if key handle is successfully created
	 * 		   false : On failure of creating key handle
	 *
	 * \note   The key used for encryption will be derived from DH algorithm.
	 *         This API has to be used to assign the diffie hellmann key to the
	 *         key handle created during initialization/object creation.hence
	 *         this has to be called before performing encryption if Diffie
	 *         hellmann key exchange algo is used by application.
	 */
	bool encryptionDhKeyHandle(uint8_t *dhKey);

	/**
	 * \brief API to get the encrypted data
	 * @return Pointer to the encrypted Data
	 */
	uint8_t* GetEncryptedData();

	/**
	 * \brief API to get the initialization vector
	 * @return Pointer to the initialization vector
	 */
	uint8_t* GetIvData();

	/**
	 * \brief API to get the size of encryption data
	 * @return size of encryption data
	 */
	uint8_t  sizeofEncryption();

	/**
	 * \brief Destructor
	 * 		  - Frees the encrypted data buffer created in runtime
	 * 		  - Destroy the key handle
	 * 		  - Frees the data structures used by mbed tls crypto library
	 */
	virtual ~encryptMgr();
};

#endif /* ASW_SERVICES_ENCRYPTION_MANAGER_ENCRYPTMGR_H_ */

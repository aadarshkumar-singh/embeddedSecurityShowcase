/****************************************************************************
* Filename        : decryptMgr.cpp
* Author          : Vaishnavi Sankaranarayanan
* Description     : Source file that defines APIs supported for AES block mode
* 					of Decryption.
* Created on	  : Jun 28, 2020
****************************************************************************/


/***************************************************************************
 * Includes
 ***************************************************************************/
#include "decryptMgr.h"


/***************************************************************************
 * Global variables
 ***************************************************************************/

/**
 * \brief UART Object to print on Hterm
 */
Serial pc2(USBTX, USBRX); // tx, rx


/***************************************************************************
 * API definitions
 ***************************************************************************/

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
errorTypeDecrypt_t decryptMgr::setDecryptionAlgo() {
	errorTypeDecrypt_t errorCode  = DECRYPT_VALID_ALGO ;

	switch (m_algoType)
	{
	/* Macro used by PSA layer for no padding*/
	case DECRYPT_CBC_NO_PADDING :
		m_algSupportedByPSA = PSA_ALG_CBC_NO_PADDING ;
		break;

	/* Macro used by PSA layer for padding with PKCS7 algorithm.*/
	case DECRYPT_CBC_WITH_PADDING:
		m_algSupportedByPSA = PSA_ALG_CBC_PKCS7;
		break;

	default :
		errorCode  =DECRYPT_INVALID_ALGO ;
	}

	return errorCode ;

}

/**
 * \brief  Calculates size of a decryption block
 * @return Error status:
 * 		   DECRYPT_VALID_BLOCK_SIZE : Valid Block size for AES Decryption
 * 		   DECRYPT_INVALID_BLOCK_SIZE : Block size not supported by h/w for AES Decryption
 */
errorTypeDecrypt_t decryptMgr::calculateBlockSize()
{
	errorTypeDecrypt_t errorCode  = DECRYPT_VALID_BLOCK_SIZE ;

	/* PSA API to find the block size supported by the PSoC hardware crypto
	   for AES block mode of encryption. */
	m_blockSize = PSA_BLOCK_CIPHER_BLOCK_SIZE(PSA_KEY_TYPE_AES);

	/* Verifying the block size returns matches with block size specified in the documentation.*/
	if (m_blockSize != AES_BLOCK_SIZE_SUPPORTED_BY_HW)
	{
		errorCode = DECRYPT_INVALID_BLOCK_SIZE ;
	}

	return errorCode ;
}


/**
 * \brief  Calculates size of a decrypted data buffer
 * @return Error status:
 * 		   DECRYPT_PADDING_SUCCESS : when buffer size to store decrypted values is proper
 * 		   DECRYPT_PADDING_FAIL : when buffer size to store decrypted values is not proper
 */

errorTypeDecrypt_t decryptMgr::calculateSizeOfDecryptedText()
{
	errorTypeDecrypt_t errorCode  = DECRYPT_PADDING_SUCCESS ;

    /* Checks if no padding is used the size of plain text is divisible
	   by block size , if not error code is returned. */
	if (m_algoType == DECRYPT_CBC_NO_PADDING)
	{
		if (m_sizeOfEncryptedData % AES_BLOCK_SIZE_SUPPORTED_BY_HW == 0)
		{
			m_sizeOfDecryptedText = m_sizeOfEncryptedData ;
		}

	}

	else if (m_algoType == DECRYPT_CBC_WITH_PADDING)
	{
		/* A padded Cipher text is always receieved from Encryption manager*/
		//Cipher output buffer = input size + block size for PKCS7 padding - (input size % 16)
		// It becomes multiple of 16
		// https://github.com/ARMmbed/mbed-os-example-mbed-crypto/pull/2/files
		m_sizeOfDecryptedText =m_sizeOfEncryptedData;
	}

	else
	{
		/*The algorithm applied is invalid*/
		errorCode = DECRYPT_INVALID_ALGO ;
	}

	return errorCode ;

}

/**
 * \brief  Creates key handle for Decryption
 * @return Error status:
 * 		   ENCRYPT_HANDLE_SUCCESS : Valid key handle for AES Decryption
 * 		   ENCRYPT_HANDLE_FAIL : Invalid key handle for AES Decryption
 */
errorTypeDecrypt_t decryptMgr::createKeyHandleForDecryption(uint8_t * decryptionKey)
{
	errorTypeDecrypt_t errorCode  = DECRYPT_HANDLE_SUCCESS ;
	/* Definition required to set attribute for initialization of hardware
	   crypto for performing AES decryption */

	psa_set_key_usage_flags(&m_attributes, PSA_KEY_USAGE_DECRYPT);// Flag for enabling decryption
	psa_set_key_algorithm(&m_attributes, m_algSupportedByPSA);    // Mode of AES decryption
	psa_set_key_type(&m_attributes, PSA_KEY_TYPE_AES);            // Key type for AES decryption
	psa_set_key_bits(&m_attributes, 128);                         // Length of key size in bits
	                                                              // determines the type of AES algo (128,256,192)
	
	// Initializes the PSoC hardware crypto with the provided attributes and key for performing decryption
	if (psa_import_key(&m_attributes, decryptionKey, AES_DECRYPTION_KEY_SIZE, &m_handle) != PSA_SUCCESS)
	{
		printf("Failed to import a key\n");
		errorCode  = DECRYPT_HANDLE_FAIL ;

	}
	// Resets the attributes after initialization
	psa_reset_key_attributes(&m_attributes);



	return errorCode ;

}

/**
 * \brief Used to create Key handle for diffie hellman keys
 * @param dhkey : DH key
 * \note   The key used for decryption will be derived from DH algorithm.
 *         This API has to be used to assign the diffie hellmann key to the
 *         key handle created during initialization/object creation.hence
 *         this has to be called before performing decryption if Diffie
 *         hellmann key exchange algo is used by application.
 * @return true : if key handle is successfully created
 * 		   false : On failure of creating key handle
 */
bool decryptMgr::decryptionDhKeyHandle(uint8_t *dhKey)
{
	if ( DECRYPT_HANDLE_SUCCESS == createKeyHandleForDecryption(dhKey))
	{
		return true;
	}
	else
	{
		return false;
	}
}

/**
 * \brief  Initializes all the data structure essential for
 * 		   performing decryption
 * \note  The key handle is created with default AES decryption key
 * 		  so that the manager supports AES- decryption for application which
 * 		  does not use Diffie Hellmann key exchange algorithm for deriving
 * 		  the shared secret key between the sender and receiver
 * @return void
 */
void decryptMgr::initializeDecryptionServices()
{
	int idx = 0;

	/*Always Block size should be initialized before cipher text size calculation */
	m_errorStatus = calculateBlockSize() ;

	for (idx = 0; idx < AES_DECRYPTION_KEY_SIZE; idx++)
	{
		m_decryptionKey[idx] = defaultDecryptAesKeyBuf[idx];
	}

	if (DECRYPT_VALID_BLOCK_SIZE == m_errorStatus)
	{
		m_errorStatus = setDecryptionAlgo() ;

		if ( DECRYPT_VALID_ALGO == m_errorStatus)
		{
			/*Calculates cipher text size based on Algorithm and checks block size */
			m_errorStatus = calculateSizeOfDecryptedText();

			if (DECRYPT_PADDING_SUCCESS == m_errorStatus)
			{
				m_attributes = PSA_KEY_ATTRIBUTES_INIT ;
				m_operation  = PSA_CIPHER_OPERATION_INIT ;

				m_errorStatus = createKeyHandleForDecryption(m_decryptionKey);
			}

		}
	}

}

/**
 * \brief Constructor for the decryption Manager
 *
 * @param m_cipherencryptedData : Encrypted data received
 * @param m_Iv : IV used for Encryption
 * @param sizeOfEncryptedData : Size of Encrypted data
 * @param algoType : AES- Algorithim used for Decryption
 */
decryptMgr::decryptMgr(uint8_t *m_cipherencryptedData, uint8_t *m_Iv,
		uint16_t sizeOfEncryptedData, typeOfDecryptAlgo_t algoType)
{
	//Assigning Algo Type
	m_algoType=algoType;
	m_sizeOfEncryptedData=sizeOfEncryptedData;
	//Initializing Decryption
	initializeDecryptionServices();
	if (DECRYPT_HANDLE_SUCCESS == m_errorStatus)
	{
		//Memory allocating for encrypting data
		m_cipherEncryptedData=new uint8_t [m_sizeOfEncryptedData];
		m_cipherEncryptedData=m_cipherencryptedData;
		//Allocating IV
		for(int i=0;i<AES_BLOCK_SIZE_SUPPORTED_BY_HW;i++)
		{
		m_iv[i]=m_Iv[i];
		}

		//Memory allocation for Decryption buffer
		m_DecryptedText=new uint8_t [m_sizeOfDecryptedText];
		m_errorStatus = DECRYPT_INIT_SUCCESS;
	}
}

/**
 * \brief Performs symmetric Decryption
 * @return DECRYPT_SUCCESS : When decryption is successful
 * 		   DECRYPT_CIPHER_SETUP_FAIL : When decryption setup fails
 * 		   DECRYPT_IV_GENERATE_FAIL : When Initialisation vector is not generated
 * 		   DECRYPT_CIPHER_UPDATE_FAIL : when updation of decrypted buffer fails
 * 		   DECRYPT_CIPHER_NOT_FINISHED : When multipart decryption is not finished
 */
errorTypeDecrypt_t decryptMgr::DecryptUsingSymmetricCiphers()
{
	size_t output_len;
	psa_status_t status;
	if (DECRYPT_INIT_SUCCESS == m_errorStatus)
	{
		/* Decrypt the ciphertext */
		status = psa_cipher_decrypt_setup(&m_operation, m_handle, m_algSupportedByPSA);
		if (status != PSA_SUCCESS) {
			m_errorStatus  = DECRYPT_CIPHER_SETUP_FAIL;

		}
		/*Set IV*/
		status = psa_cipher_set_iv(&m_operation, m_iv, sizeof(m_iv));
		if (status != PSA_SUCCESS) {
			m_errorStatus  = DECRYPT_IV_GENERATE_FAIL;

		}
		/*SET Cipher update*/
		status = psa_cipher_update(&m_operation, m_cipherEncryptedData, m_sizeOfEncryptedData,
				m_DecryptedText, m_sizeOfDecryptedText, &output_len);
		if (status != PSA_SUCCESS) {
			m_errorStatus  = DECRYPT_CIPHER_UPDATE_FAIL;

		}

		/*Finishing cipher finishing*/
		status = psa_cipher_finish(&m_operation, m_DecryptedText+output_len,
				m_sizeOfDecryptedText-output_len , &output_len);
		if (status != PSA_SUCCESS) {
			m_errorStatus  = DECRYPT_CIPHER_NOT_FINISHED;
		}
		else
		{
			m_errorStatus = DECRYPT_SUCCESS;
		}
	}
	else
	{
		m_errorStatus  = DECRYPT_INIT_FAIL ;
	}

	return m_errorStatus ;
}

/**
 *  \brief Displays the decrypted message in UART
 *  \note  used for debug logs
 */
void decryptMgr::displayDecryptedMessage()
{
	pc2.printf(" \n Decrypted message : ");
	for (uint8_t i = 0; i<m_sizeOfDecryptedText; i++)
	{

		pc2.putc(m_DecryptedText[i]);
	}
	pc2.printf(" \n");
}

/**
 * \brief Destructor
 * 		  - Frees the decrypted data buffer created in runtime
 * 		  - Destroy the key handle
 * 		  - Frees the data structures used by mbed tls crypto library
 */

decryptMgr::~decryptMgr() {
	/* Clean up cipher operation context */
	psa_cipher_abort(&m_operation);

	/* Destroy the key */
	psa_destroy_key(m_handle);
	m_handle=0;

	/*Deleteing memory for created buffers*/
	delete[] m_DecryptedText;
	delete[] m_cipherEncryptedData;


	mbedtls_psa_crypto_free();
}

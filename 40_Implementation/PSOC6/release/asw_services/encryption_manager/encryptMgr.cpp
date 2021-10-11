/****************************************************************************
* Filename        : encryptMgr.cpp
* Author          : Aadarsh Kumar Singh
* Description     : Source file that defines APIs supported for AES block mode
* 					of encryption.
* Created on	  : Jun 6, 2020
****************************************************************************/

/***************************************************************************
 * Includes
 ***************************************************************************/
#include "encryptMgr.h"


/***************************************************************************
 * Global variables
 ***************************************************************************/

/**
 * \brief UART Object to print on Hterm
 */
Serial pc1(USBTX, USBRX); // tx, rx

/***************************************************************************
 * API definitions
 ***************************************************************************/

/**
 * \brief  Calculates size of a encryption block
 * @return Error status:
 * 		   ENCRYPT_VALID_BLOCK_SIZE : Valid Block size for AES Encryption
 * 		   ENCYPT_INVALID_BLOCK_SIZE : Block size not supported by h/w for AES Encryption
 */
errorTypeEncrypt_t encryptMgr::calculateBlockSize()
{
	errorTypeEncrypt_t errorCode  = ENCRYPT_VALID_BLOCK_SIZE ;

	/* PSA API to find the block size supported by the PSoC hardware crypto
	   for AES block mode of encryption. */
	m_blockSize = PSA_BLOCK_CIPHER_BLOCK_SIZE(PSA_KEY_TYPE_AES);

	// Verifying the block size returns matches with block size specified in the documentation.
	if (m_blockSize != AES_BLOCK_SIZE_SUPPORTED_BY_HW)
	{
		errorCode = ENCYPT_INVALID_BLOCK_SIZE ;
	}

	return errorCode ;
}

/**
 * \brief  Initialization of Encryption algorithm
 *
 * @return Error status:
 * 		   ENCRYPT_VALID_ALGO : Algorithm supported by user application
 * 		   ENCRYPT_INVALID_ALGO : Algorithm not supported by user application
 *
 * \note   - When the length of the plain text is divisible by the block size
 * 		     then we do not need padding. It is advised to use padding because
 * 		     the it is not ensured that the size of the plain text to be encrypted
 * 		     will always be divisible by block size.
 *
 * 		   - Padding algorithm supported is PKCS7
 */
errorTypeEncrypt_t encryptMgr::setEncryptionAlgo()
{
	errorTypeEncrypt_t errorCode  = ENCRYPT_VALID_ALGO ;

	switch (m_algoType)
	{
		case ENCRYPT_CBC_NO_PADDING :
			// Macro used by PSA layer for no padding
			m_algSupportedByPSA = PSA_ALG_CBC_NO_PADDING ;
			break;

		case ENCRYPT_CBC_WITH_PADDING:
			// Macro used by PSA layer for padding with PKCS7 algorithm.
			m_algSupportedByPSA = PSA_ALG_CBC_PKCS7;
			break;

		default :
			errorCode  = ENCRYPT_INVALID_ALGO ;
	}

	return errorCode ;
}

/**
 * \brief  Calculates size of a encrypted data buffer
 * @return Error status:
 * 		   ENCRYPT_PADDING_SUCCESS : when buffer size to store encrypted values is proper
 * 		   ENCRYPT_PADDING_FAIL : when buffer size to store encrypted values is not proper
 */
errorTypeEncrypt_t encryptMgr::calculateSizeOfCipherText()
{
	errorTypeEncrypt_t errorCode  = ENCRYPT_PADDING_SUCCESS ;

	/* Checks if no padding is used the size of plain text is divisible
	   by block size , if not error code is returned. */
	if (m_algoType == ENCRYPT_CBC_NO_PADDING)
	{
		if (m_sizeOfPlainText % AES_BLOCK_SIZE_SUPPORTED_BY_HW == 0)
		{
			m_sizeOfCipherText = m_sizeOfPlainText ;
		}
		else
		{
			m_sizeOfCipherText  = 0;
			errorCode = ENCRYPT_PADDING_FAIL ;
		}
	}

	/* if padding is used it calculates the length of the cipher text when
	   PKCS7 padding is used */
	else if (m_algoType == ENCRYPT_CBC_WITH_PADDING)
	{
		//Cipher output buffer = input size + block size for PKCS7 padding - (input size % 16)
		// It becomes multiple of 16
		// https://github.com/ARMmbed/mbed-os-example-mbed-crypto/pull/2/files
		m_sizeOfCipherText = m_sizeOfPlainText + m_blockSize - (m_sizeOfPlainText % 16);
	}

	else
	{
		errorCode = ENCRYPT_INVALID_ALGO ;
	}

	return errorCode ;
}

/**
 * \brief  Creates key handle for Encryption
 * @return Error status:
 * 		   ENCRYPT_HANDLE_SUCCESS : Valid key handle for AES Encryption
 * 		   ENCRYPT_HANDLE_FAIL : Invalid key handle for AES Encryption
 */
errorTypeEncrypt_t encryptMgr::createKeyHandleForEncryption(uint8_t *encryptionKey)
{
	errorTypeEncrypt_t errorCode  = ENCRYPT_HANDLE_SUCCESS ;

	/* Definition required to set attribute for initialization of hardware
	   crypto for performing AES encryption */

    psa_set_key_usage_flags(&m_attributes, PSA_KEY_USAGE_ENCRYPT); // Flag for enabling encryption
    psa_set_key_algorithm(&m_attributes, m_algSupportedByPSA);	   // Mode of AES Encryption
    psa_set_key_type(&m_attributes, PSA_KEY_TYPE_AES);			   // Key type for AES encryption
    psa_set_key_bits(&m_attributes, 128); 		                   // Length of key size in bits
    															   // determines the type of AES algo (128,256,192)

    // Initializes the PSoC hardware crypto with the provided attributes and key for performing encryption
    if (PSA_SUCCESS != psa_import_key(&m_attributes,encryptionKey, AES_ENCRYPTION_KEY_SIZE, &m_handle))
    {
    	pc1.printf("Failed to import a key\n");
    	errorCode  = ENCRYPT_HANDLE_FAIL ;
    }

    // Resets the attributes after initialization
    psa_reset_key_attributes(&m_attributes);

    return errorCode ;
}


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
bool encryptMgr::encryptionDhKeyHandle(uint8_t *dhKey)
{
	if ( ENCRYPT_HANDLE_SUCCESS == createKeyHandleForEncryption(dhKey))
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
 * 		   performing encryption
 * @return void
 *
 * \note  The key handle is created with default AES encryption key
 * 		  so that the manager supports AES- Encryption for application which
 * 		  does not use Diffie Hellmann key exchange algorithm for deriving
 * 		  the shared secret key between the sender and receiver
 */
void encryptMgr::initializeEncryptionServices()
{
	int idx = 0;

	/*Always Block size should be initialized before cipher text size calculation */
	m_errorStatus = calculateBlockSize() ;

	/* Initializes the AES encryption key with default values */
	for (idx = 0; idx < AES_ENCRYPTION_KEY_SIZE; idx++)
	{
		m_encryptionKey[idx] = defaultEncryptAesKeyBuf[idx];
	}

	if (ENCRYPT_VALID_BLOCK_SIZE == m_errorStatus)
	{
		/* Sets Encryption algorithm */
		m_errorStatus = setEncryptionAlgo() ;

		if ( ENCRYPT_VALID_ALGO == m_errorStatus)
		{
			/*Calculates cipher text size based on Algorithm and checks block size */
			m_errorStatus = calculateSizeOfCipherText();

			if (ENCRYPT_PADDING_SUCCESS == m_errorStatus)
			{
				m_attributes = PSA_KEY_ATTRIBUTES_INIT ;
				m_operation  = PSA_CIPHER_OPERATION_INIT ;

				/* Assigns default key and creates key handle */
				m_errorStatus = createKeyHandleForEncryption(m_encryptionKey);
			}

		}
	}

}



/**
 * \brief Constructor for the encryption Manager
 *
 * @param plainText : address of the Input plain text buffer
 * @param sizeOfPlainText : size of the input plain text buffer
 * @param algoType : AES- Algorithim used for encryption
 */
encryptMgr::encryptMgr(uint8_t *plainText, uint16_t sizeOfPlainText,
		typeOfEncryptAlgo_t algoType):m_plainText(plainText),m_sizeOfPlainText(sizeOfPlainText),
	    m_algoType(algoType)
{
	// initializes the PSoC hardware crypto for performing AES encryption
	initializeEncryptionServices();

	if (ENCRYPT_HANDLE_SUCCESS == m_errorStatus)
	{
		m_cipherText = new uint8_t [m_sizeOfCipherText];
		m_errorStatus = ENCRYPT_INIT_SUCCESS;
	}
}

/**
 * \brief Performs symmetric encryption
 * @return ENCRYPT_SUCCESS : When encryption is successful
 * 		   ENCRYPT_CIPHER_SETUP_FAIL : When encryption setup fails
 * 		   ENCRYPT_IV_GENERATE_FAIL : When Initialisation vector is not generated
 * 		   ENCRYPT_CIPHER_UPDATE_FAIL : when updation of encrypted buffer fails
 * 		   ENCRYPT_CIPHER_NOT_FINISHED : When multipart encryption is not finished
 */
errorTypeEncrypt_t encryptMgr::encryptUsingSymmetricCiphers()
{
    size_t iv_len;
    size_t output_len;
    psa_status_t status;

    /* Encryption process */
	if (ENCRYPT_INIT_SUCCESS == m_errorStatus)
	{
		/* API populates operation handle with key handle and algo used for encryption */
		status = psa_cipher_encrypt_setup(&m_operation, m_handle, m_algSupportedByPSA);
		if (PSA_SUCCESS != status)
		{
			pc1.printf("Failed to begin cipher operation\n");
			m_errorStatus  = ENCRYPT_CIPHER_SETUP_FAIL;
		}
		/* API to generate random initialization vector */
		status = psa_cipher_generate_iv(&m_operation, m_iv, sizeof(m_iv), &iv_len);
		if (PSA_SUCCESS != status)
		{
			pc1.printf("Failed to generate IV\n");
			m_errorStatus  = ENCRYPT_IV_GENERATE_FAIL;
		}
		/* API to encrypt the plain text and stores it in cipher text*/
		status = psa_cipher_update(&m_operation, m_plainText, m_sizeOfPlainText,
									m_cipherText, m_sizeOfCipherText, &output_len);
		if (PSA_SUCCESS != status )
		{
			pc1.printf("Failed to update cipher operation\n");
			m_errorStatus  = ENCRYPT_CIPHER_UPDATE_FAIL;
		}
		/* API to finish encrypting, if encryption is successful clears the operation handle */
		status = psa_cipher_finish(&m_operation, m_cipherText + output_len,
				m_sizeOfCipherText - output_len, &output_len);

		if (PSA_SUCCESS == status)
		{
			m_errorStatus = ENCRYPT_SUCCESS;
		}
		else
		{
			pc1.printf("Failed to finish cipher operation\n");
			m_errorStatus  = ENCRYPT_CIPHER_NOT_FINISHED;
		}
	}

	else
	{
		pc1.printf("Initialization failed \n");
		m_errorStatus  = ENCRYPT_INIT_FAIL ;
	}

	return m_errorStatus ;
}

/**
 *  \brief Displays the encrypted message in UART
 *  \note  used for debug logs
 */
void encryptMgr::displayEncryptedMessage()
{

	pc1.printf("INP:\n");

	for (uint8_t i = 0; i<m_sizeOfPlainText; i++)
	{
		pc1.putc(m_plainText[i]);
	}

	pc1.printf("IV:\n");

	for (uint8_t i = 0; i<m_blockSize; i++)
	{
		pc1.putc(m_iv[i]);
	}

	if (m_errorStatus != ENCRYPT_SUCCESS)
	{
		pc1.printf("Encryption Failed");
	}
	else
	{
		pc1.printf("o:\n");

		for (uint8_t i = 0; i<m_sizeOfCipherText; i++)
		{

			pc1.putc(m_cipherText[i]);
		}

	}
}

/**
 * \brief API to get the encrypted data
 * @return Pointer to the encrypted Data
 */
uint8_t* encryptMgr::GetEncryptedData()
{
	return m_cipherText;
}

/**
 * \brief API to get the initialization vector
 * @return Pointer to the initialization vector
 */
uint8_t* encryptMgr::GetIvData()
{
	return m_iv;
}

/**
 * \brief API to get the size of encryption data
 * @return size of encryption data
 */
uint8_t encryptMgr::sizeofEncryption()
{
	return m_sizeOfCipherText;
}

/**
 * \brief Destructor
 * 		  - Frees the encrypted data buffer created in runtime
 * 		  - Destroy the key handle
 * 		  - Frees the data structures used by mbed tls crypto library
 */
encryptMgr::~encryptMgr()
{
    /* Clean up cipher operation context */
    psa_cipher_abort(&m_operation);

    /* Destroy the key */
    psa_destroy_key(m_handle);

    m_handle = 0;

    delete[] m_cipherText ;

    mbedtls_psa_crypto_free();
}



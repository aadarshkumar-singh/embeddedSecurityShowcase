/****************************************************************************
* Filename        : decryptServicesCfg.h
* Author          : Vaishnavi Sankaranarayanan 
* Description     : Header file that provides configuration required for AES
* 					block mode of Decryption and ECDSA signature.
* Created on	  : Jun 29, 2020
****************************************************************************/

#ifndef ASW_SERVICES_DECRYPTION_MANAGER_DECRYPTSERVICESCFG_H_
#define ASW_SERVICES_DECRYPTION_MANAGER_DECRYPTSERVICESCFG_H_

/***************************************************************************
 * Includes
 ***************************************************************************/
#include <stdlib.h>
/***************************************************************************
 * Macro Definitions
 ***************************************************************************/

/************************ < Macros for AES Decryption **********************/

/**< Type of algorithm used for Decrytpion */
#define AES_DECRYPTION_TYPE PSA_KEY_TYPE_AES

/**< Size of block supported by PSoC hardware crypto for AES algorithm */
#define AES_BLOCK_SIZE_SUPPORTED_BY_HW 16

/**< Size of decryption key size */
#define AES_DECRYPTION_KEY_SIZE 16

/***************************************************************************
 * Enumerations and Type definitions
 ***************************************************************************/

/*
 * This key is present as a global define for the purpose of example. In
 * real-world applications, you would not have a key hardcoded in source like
 * this.
 */
static const uint8_t defaultDecryptAesKeyBuf[AES_DECRYPTION_KEY_SIZE] =
{
    0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
    0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
};



enum typeOFDecryptAlgo
{
	DECRYPT_CBC_NO_PADDING,
	DECRYPT_CBC_WITH_PADDING,
};
typedef typeOFDecryptAlgo typeOfDecryptAlgo_t ;

enum errorTypeDecrypt_e
{
	DECRYPT_INIT_FAIL,
	DECRYPT_PADDING_FAIL,
	DECRYPT_HANDLE_FAIL,
	DECRYPT_INVALID_BLOCK_SIZE,
	DECRYPT_INVALID_ALGO,
	DECRYPT_CIPHER_SETUP_FAIL,
	DECRYPT_IV_GENERATE_FAIL,
	DECRYPT_CIPHER_UPDATE_FAIL,
	DECRYPT_CIPHER_NOT_FINISHED,
	DECRYPT_VALID_BLOCK_SIZE,
	DECRYPT_VALID_ALGO,
	DECRYPT_HANDLE_SUCCESS,
	DECRYPT_PADDING_SUCCESS,
	DECRYPT_INIT_SUCCESS,
	DECRYPT_SUCCESS,
};

typedef errorTypeDecrypt_e errorTypeDecrypt_t ;




#endif /* ASW_SERVICES_DECRYPTION_MANAGER_DECRYPTSERVICESCFG_H_ */

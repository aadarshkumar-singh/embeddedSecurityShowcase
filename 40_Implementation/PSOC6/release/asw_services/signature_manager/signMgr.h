/****************************************************************************
* Filename        : signMgr.h
* Author          : Hari Krishna Yelchuri
* Description     : Header file that declares APIs supported for genertaing
* 					digital signature.
* Created on	  : Jun 6, 2020
****************************************************************************/

#ifndef ASW_SERVICES_SIGNATURE_MANAGER_SIGNMGR_H_
#define ASW_SERVICES_SIGNATURE_MANAGER_SIGNMGR_H_

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
 * \brief Signature Manager for providing services for generating digital signature.
 */
class signMgr {

private:
	/**
	 * \brief Stores the input to be hashed and signed.
     */
	uint8_t* m_encryptedData;

	/**
	 * \brief Buffer to store the generated signature.
     */
	uint8_t m_signature[ECDSA_SIGN_LENGTH];

	/**
	 * \brief Stores the signature length.
     */
	size_t m_signature_length;

	/**
	 * \brief Attributes of the key used for signing.
     */
	psa_key_attributes_t m_attributes_sign;

	/**
	 * \brief Stores the public key corresponding to the private key,
     */
	uint8_t m_exported[ECDSA_PUB_KEY_LENGTH];

	/**
	 * \brief Length of the public key to be exported.
     */
	size_t m_exported_length = 0;

	/**
	 * \brief Algorithm associated with key generation
     */
	psa_algorithm_t m_key_gen_alg;

	/**
	 * \brief Algorithm used to generate hash.
     */
	psa_algorithm_t m_alg_hash;

	/**
	 * \brief Type of the key to be generated.
     */
	psa_key_type_t m_key_gen_key_type;

	/**
	 * \brief Type of eleptic curve on which the keys should lie on.
     */
	psa_ecc_curve_t m_key_gen_curve;

	/**
	 * \brief Stores the generated key using the given attributes.
     */
	psa_key_handle_t m_handle;

	/**
	 * \brief A structure to store the data to be sent via UART.
     */
	uartData_t data_send;
public:

	/**
	 * \brief Constructor for the Signature Manager
	 * @param encryptedData : Encrypted Data to be hashed and signed.
	 */
	signMgr(uint8_t* encryptedData);

	/**
	 * \brief Function that generates the private key and signs data after hashing.
	 */
	void signhash();

	/**
	 * \brief Function that derives public key from the generated private key.
	 * @return struct populated with data for verification
	 */
	uartData_t& export_PublicKey();
};

#endif /* ASW_SERVICES_SIGNATURE_MANAGER_SIGNMGR_H_ */

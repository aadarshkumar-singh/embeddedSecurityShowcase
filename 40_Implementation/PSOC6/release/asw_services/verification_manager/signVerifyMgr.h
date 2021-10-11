/****************************************************************************
* Filename        : bleMgr.h
* Author          : Hari Krishna Yelchuri
* Description     : Header file that declares functions for the Verification
* 					of digital signature.
****************************************************************************/
#ifndef ASW_SERVICES_VERIFICATION_MANAGER_SIGNVERIFYMGR_H_
#define ASW_SERVICES_VERIFICATION_MANAGER_SIGNVERIFYMGR_H_

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
     * \brief Signature Verification Manager to authenticate digital
     * signatures by verification.
	 */
class signVerifyMgr {

private:

	/**
	 * \brief Used to represent error and success codes.
	 */
	psa_status_t status;

	/**
     * \brief The data against which the signature has to be verified.
	 */
	const uint8_t * encryptedData;

	/**
     * \brief Stores the received signature.
	 */
	uint8_t m_signature[ECDSA_SIGN_LENGTH];

	/**
     * \brief Length of signature.
	 */
	size_t m_signature_length;

	/**
     * \brief Attributes to import key for verification.
	 */
	psa_key_attributes_t m_attributes_verify;

	/**
     * \brief Algorithm for which the key is being generated.
	 */
	psa_algorithm_t m_key_import_alg;

	/**
     * \brief Algorithm defined for performing hashing on data.
	 */
	psa_algorithm_t m_alg_hash;

	/**
     * \brief Kype of the key to be imported.
	 */
	psa_key_type_t m_key_gen_key_type;

	/**
     * \brief Curve on which the imported key lies.
	 */
	psa_ecc_curve_t m_key_gen_curve;

	/**
     * \brief Stores the generated key.
	 */
	psa_key_handle_t m_handle;

	/**
     * \brief Structure to hold data received from UART.
	 */
	uartData_t m_data_received;

public:

	/**
	 * \brief Constructor for the Signature Verification Manager
	 *
	 * @param data_received : Structure containg the data for public key generation and signature verification.
	 */
	signVerifyMgr(uartData_t& data_received);


	/**
	 * \brief This function verifies the given signature using the encrypted data.
	 */
	bool verifySignature();
};

#endif /* ASW_SERVICES_VERIFICATION_MANAGER_SIGNVERIFYMGR_H_ */

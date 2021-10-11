/****************************************************************************
* Filename        : bleMgr.cpp
* Author          : Hari Krishna Yelchuri
* Description     : Source file that declares functions for the Signature
* 					Verification Manager
*
****************************************************************************/
#include "signVerifyMgr.h"

//Used to print data over uart.
Serial serial_print(USBTX, USBRX);

/**
 * \brief Constructor for the Signature Verification Manager
 *
 * @param data_received : Structure containg the data for public key generation and signature verification.
 */
signVerifyMgr::signVerifyMgr(uartData_t& data_received) : m_data_received(data_received) {

	/**
	 * Represents the data that is looked upon when verifying the signature. This
	 * is the same data that was used for generating the signature.
	 */
	encryptedData = (uint8_t*)"I am 01234567891";

    for(int i=0; i<ECDSA_SIGN_LENGTH; ++i)
    	{
    		m_signature[i] = m_data_received.signature[i];
    	}

	m_signature_length = m_data_received.signature_length;

	m_attributes_verify = PSA_KEY_ATTRIBUTES_INIT;

	m_alg_hash = PSA_ALG_SHA_256; //The hashing algorithm used is Secure Hash Algorithm - 256 bits
	m_key_import_alg = PSA_ALG_DETERMINISTIC_ECDSA(m_alg_hash);

	//The elleptic curves here should be same as the one used for generating signature.
	m_key_gen_curve = PSA_ECC_CURVE_SECP256R1;
	m_key_gen_key_type = PSA_KEY_TYPE_ECC_PUBLIC_KEY(m_key_gen_curve);

}

/**
 * \brief This function verifies the given signature using the encrypted data.
 */
bool signVerifyMgr::verifySignature() {

	psa_set_key_usage_flags(&m_attributes_verify, PSA_KEY_USAGE_VERIFY);

	psa_set_key_algorithm(&m_attributes_verify, m_key_import_alg);

	psa_set_key_type(&m_attributes_verify, m_key_gen_key_type);

	psa_set_key_bits(&m_attributes_verify, key_bits_256);

	//Provides with a key using the received public key,to verify the signature. This key is
	//represented as a handle.
	psa_import_key(&m_attributes_verify, m_data_received.publicKey, m_data_received.publiKey_length, &m_handle);

	// Finally verify signature using the handle and the encrypted data.
	status = psa_asymmetric_verify(m_handle,
						  m_key_import_alg,
					      encryptedData, sizeof(encryptedData),
						  m_signature, m_signature_length
						  );
	if (status != PSA_SUCCESS)
	{
		serial_print.printf("Failed to verify\n");
        return false;
	}
	else
	{
		serial_print.printf("Sucessfully verified\n");
		return true;
	}
}

/****************************************************************************
* Filename        : signMgr.cpp
* Author          : Hari Krishna Yelchuri
* Description     : Source file that declares APIs supported for genertaing
* 					digital signature.
* Created on	  : Jun 6, 2020
****************************************************************************/
/***************************************************************************
 * Includes
 ***************************************************************************/
#include "signMgr.h"

/***************************************************************************
 * API definitions
 ***************************************************************************/
/**
 * \brief Constructor for the Signature Manager
 * @param encryptedData : Encrypted Data to be hashed and signed.
 */
signMgr::signMgr(uint8_t* encryptedData): m_encryptedData(encryptedData)
{
	//Initialize crypto hardware
    psa_crypto_init();

    /* Select the curve for eleptic curve cryptography and algorithm for hashing. */
	m_attributes_sign = PSA_KEY_ATTRIBUTES_INIT;
	m_key_gen_curve = PSA_ECC_CURVE_SECP256R1;
	m_key_gen_key_type = PSA_KEY_TYPE_ECC_KEY_PAIR(m_key_gen_curve);
	m_alg_hash = PSA_ALG_SHA_256;
	m_key_gen_alg = PSA_ALG_DETERMINISTIC_ECDSA(m_alg_hash);

}

/**
 * \brief Function that generates the private key and signs data after hashing.
 */
void signMgr::signhash()
{
	/*Fill the signature attributes according to the required specification.*/
    psa_set_key_usage_flags(&m_attributes_sign, PSA_KEY_USAGE_SIGN);
    psa_set_key_algorithm(&m_attributes_sign,
    		m_key_gen_alg);
    psa_set_key_type(&m_attributes_sign,
    		m_key_gen_key_type);
    psa_set_key_bits(&m_attributes_sign, key_bits_256);

    //Generate a handle containg key info
    psa_generate_key(&m_attributes_sign, &m_handle);

    //Obtain the signature for the encrypted data
    psa_asymmetric_sign(m_handle,
    					m_key_gen_alg,
						m_encryptedData, sizeof(m_encryptedData),
						m_signature, sizeof(m_signature),
    	                &m_signature_length);

    //Populate the struct storing the data required for verification
    for(int i=0; i<ECDSA_SIGN_LENGTH; ++i)
    {
    	data_send.signature[i] = m_signature[i];
    }
    data_send.signature_length = m_signature_length;

}

/**
 * \brief Function that derives public key from the generated private key.
 * @return struct populated with data for verification
 */
uartData_t& signMgr::export_PublicKey()
{
	//Generate a public key that can be exported to other device for verifying the
	//generated signature
	psa_export_public_key(m_handle,
					      m_exported, sizeof(m_exported),
		                  &m_exported_length);

	//Populate the struct storing the data required for verification
    for(int i=0; i<ECDSA_PUB_KEY_LENGTH; ++i)
    {
    	data_send.publicKey[i] = m_exported[i];
    }
   data_send.publiKey_length = m_exported_length;

   return data_send;
}


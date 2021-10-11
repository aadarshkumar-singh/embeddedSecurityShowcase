/****************************************************************************
* Filename        : dhKeyExchangeMgr.cpp
* Author          : Aadarsh Kumar Singh
* Description     : Source file that defines APIs for DH key exchange manager 
* Created on	  : Jun 19, 2020
****************************************************************************/

/***************************************************************************
 * Includes
 ***************************************************************************/
 
#include "dhKeyExchangeMgr.h"


/***************************************************************************
 * Function definitions
 ***************************************************************************/
 
/**
* \brief Initializes deffie-Helmann key exchange services
*
* @return errorTypeKeyExchange_T : error code of deffie Helmann key exchange manager
*/
errorTypeKeyExchange_T dhKeyExchangeMgr::intializeDhKeyExchangeServices()
{
	m_errorStatus = KEYEXCHANGE_INIT_SUCCESS;

	mbedtls_entropy_context entropy;

	/* context of random number generator  */
	m_ctr_drbg = new mbedtls_ctr_drbg_context;

	const char pers[] = "ecdh";

	/* Elliptic Curve Deffie Helmann context initialization*/
	mbedtls_ecdh_init( &m_ctx );

	/* Initialization of random number generator*/
	mbedtls_ctr_drbg_init( m_ctr_drbg );

	/* Initialization of entropy context*/
	mbedtls_entropy_init( &entropy );
	
	/* Creating seed for random number generation*/
	if((mbedtls_ctr_drbg_seed( m_ctr_drbg, mbedtls_entropy_func, &entropy,
								   (const unsigned char *) pers,sizeof pers )) != 0 )
	{
		m_errorStatus = KEYEXCHANGE_INIT_FAIL;
	}

	return m_errorStatus;
}

/**
 *  \brief Constructor to initialize diffie hellmann algorithm
 */
dhKeyExchangeMgr::dhKeyExchangeMgr()
{
	m_errorStatus = intializeDhKeyExchangeServices();
}

/**
 * \brief Generate the Deffie Helmann keys using Elliptic Curve 25519
 * 		  creates key to be exchanged
 *
 * @param publicKeyForExchange[i/o] : Deffie Helmann public key that has to be exchanged
 *
 * @return errorTypeKeyExchange_T : error code of deffie Helmann key exchange manager
 */
errorTypeKeyExchange_T dhKeyExchangeMgr::generateDhKey(uint8_t *publicKeyForExchange)
{
	if ( KEYEXCHANGE_INIT_SUCCESS == m_errorStatus)
	{
		/* setting an ECP group context */
		if(mbedtls_ecp_group_load( &m_ctx.grp, MBEDTLS_ECP_DP_CURVE25519 ) != 0 )
		{
			m_errorStatus = KEYEXCHANGE_EC_NOT_FOUND ;
		}

		/* Generating Deffie helmann public key for exchange */
		if( mbedtls_ecdh_gen_public( &m_ctx.grp, &m_ctx.d, &m_ctx.Q,
				   mbedtls_ctr_drbg_random, m_ctr_drbg ) != 0 )
		{
			m_errorStatus = KEYEXCHANGE_PUBLIC_KEY_GEN_FAIL ;
		}


		/* Assigning DH public key to the i/o parameter */
		if( mbedtls_mpi_write_binary( &m_ctx.Q.X, publicKeyForExchange, DEFFIE_HELMANN_KEY_SIZE ) != 0 )
		{
			m_errorStatus = KEYEXCHANGE_WRITE_FAIL ;
		}
	}
	else
	{
		m_errorStatus = KEYEXCHANGE_DH_KEYGEN_FAIL;
	}

	return m_errorStatus ;
}

/**
* \brief Computes the shared secret key using deffie helmann algorithm
*		 using received deffie helmann public key of the ally
* 
* @param exchangedPublicKey[i]: received deffie helmann public key of the ally
* @param encryptionKey[o] : shared secret Key for Encryption
* @return errorTypeKeyExchange_T : error code of deffie Helmann key exchange manager
*/
errorTypeKeyExchange_T dhKeyExchangeMgr::computeSharedSecretDhKey(uint8_t *exchangedPublicKey, uint8_t *encryptionKey)
{
	int i =0 ,j =0;

	if( mbedtls_mpi_lset( &m_ctx.Qp.Z, 1 ) != 0 )
	{
		m_errorStatus = KEYEXCHANGE_ERROR_SECRET_GEN;
	}

	/* The DH public key received from the ally is assigned to the context */
	if( mbedtls_mpi_read_binary( &m_ctx.Qp.X, exchangedPublicKey, DEFFIE_HELMANN_KEY_SIZE ) != 0 )
	{
		m_errorStatus = KEYEXCHANGE_READ_FAIL;
	}

	/* Shared secret key is computed */
	if( mbedtls_ecdh_compute_shared( &m_ctx.grp, &m_ctx.z, &m_ctx.Qp, &m_ctx.d,
			mbedtls_ctr_drbg_random, m_ctr_drbg )!= 0 )
	{
		m_errorStatus = KEYEXCHANGE_COMPUTE_SECRET_GEN;
	}

	while(i < 16)
	{
		/*Shifting and extracting bits from m_ctx to output Encryption Key buffer*/
		encryptionKey[i++] = m_ctx.z.p[j] >> 24;
		encryptionKey[i++] = m_ctx.z.p[j]  >> 16;
		encryptionKey[i++] = m_ctx.z.p[j]  >>  8;
		encryptionKey[i++] = m_ctx.z.p[j] ;
		j++;
	}

	return m_errorStatus ;
}

/**
 * \brief destructor to free the created resources
 */
dhKeyExchangeMgr::~dhKeyExchangeMgr()
{
	/* Context of ECDH created during initialization is freed*/
    mbedtls_ecdh_free( &m_ctx );

    /* Context of random number generator is freed */
    mbedtls_ctr_drbg_free( m_ctr_drbg );
}


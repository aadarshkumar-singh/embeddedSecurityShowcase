/****************************************************************************
* Filename        : dhKeyExchangeServiceCfg.h
* Author          : Aadarsh Kumar Singh
* Description     : Header file that provides configuration required for Deffie
*					Helmann Key Exchange algorithm
* Created on	  : Jun 19, 2020
****************************************************************************/

#ifndef ASW_SERVICES_KEY_EXCHANGE_MANAGER_DHKEYEXCHANGESERVICECFG_H_
#define ASW_SERVICES_KEY_EXCHANGE_MANAGER_DHKEYEXCHANGESERVICECFG_H_

/***************************************************************************
 * Includes
 ***************************************************************************/
#include <stdlib.h>

/***************************************************************************
 * Macro Definitions
 ***************************************************************************/

/**
 * \brief Size of the Elliptic Curve key for Deffie Helmann Key exchange.
 */
#define DEFFIE_HELMANN_KEY_SIZE 32

/***************************************************************************
 * Enumerations and Type definitions
 ***************************************************************************/
/**
 * \brief Error Type of the deffie helmann Key Exchange Manager
 */
enum errorTypeKeyExchange_e
{
	KEYEXCHANGE_INIT_FAIL,			//!< KEYEXCHANGE_INIT_FAIL
	KEYEXCHANGE_INIT_SUCCESS,		//!< KEYEXCHANGE_INIT_FAIL
	KEYEXCHANGE_EC_NOT_FOUND,		//!< Elliptic curve not found
	KEYEXCHANGE_PUBLIC_KEY_GEN_FAIL,//!< KEYEXCHANGE_PUBLIC_KEY_GEN_FAIL
	KEYEXCHANGE_WRITE_FAIL,         //!< KEYEXCHANGE_WRITE_FAIL
	KEYEXCHANGE_DH_KEYGEN_FAIL,     //!< KEYEXCHANGE_DH_KEYGEN_FAIL
	KEYEXCHANGE_ERROR_SECRET_GEN,   //!< KEYEXCHANGE_ERROR_SECRET_GEN
	KEYEXCHANGE_READ_FAIL,          //!< KEYEXCHANGE_READ_FAIL
	KEYEXCHANGE_COMPUTE_SECRET_GEN, //!< KEYEXCHANGE_COMPUTE_SECRET_GEN
	KEYEXCHANGE_SUCCESS,  			//!< KEYEXCHANGE_SUCCESS
};

/**
 * \brief error code type for deffie helmann key exchange manager.
 */
typedef errorTypeKeyExchange_e errorTypeKeyExchange_T ;


#endif /* ASW_SERVICES_KEY_EXCHANGE_MANAGER_DHKEYEXCHANGESERVICECFG_H_ */

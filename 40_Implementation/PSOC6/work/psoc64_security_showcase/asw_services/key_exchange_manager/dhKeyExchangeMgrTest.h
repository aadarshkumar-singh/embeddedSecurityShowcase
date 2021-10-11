/****************************************************************************
* Filename        : dhKeyExchangeMgrTest.cpp
* Author          : Aadarsh Kumar Singh
* Description     : Header file that declares API to test functions of Deffie
* 				    helmann key exchange algorithm
* Created on	  : Jun 19, 2020
****************************************************************************/

#ifndef ASW_SERVICES_KEY_EXCHANGE_MANAGER_DHKEYEXCHANGEMGRTEST_H_
#define ASW_SERVICES_KEY_EXCHANGE_MANAGER_DHKEYEXCHANGEMGRTEST_H_

/***************************************************************************
 * Includes
 ***************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "dhKeyExchangeMgr.h"


/***************************************************************************
 * Function Declarations
 ***************************************************************************/

/**
 * \brief Test case to check if the shared secret key computed by the ally and the host
 * 		  are equal
 * @return True : When shared secret key matches
 * 		   False : When shared secret key doesn't match
 */
bool dhKeyExchange_matchEncryptionKeys();

#endif /* ASW_SERVICES_KEY_EXCHANGE_MANAGER_DHKEYEXCHANGEMGRTEST_H_ */

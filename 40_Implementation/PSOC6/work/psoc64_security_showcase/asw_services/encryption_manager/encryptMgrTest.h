/****************************************************************************
* Filename        : encryptMgrTest.h
* Author          : Aadarsh Kumar Singh
* Description     : Header file that provides API to test the encryption manager
* Created on	  : Jun 28, 2020
****************************************************************************/

#ifndef ASW_SERVICES_ENCRYPTION_MANAGER_ENCRYPTMGRTEST_H_
#define ASW_SERVICES_ENCRYPTION_MANAGER_ENCRYPTMGRTEST_H_

/***************************************************************************
 * Includes
 ***************************************************************************/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "encryptMgr.h"


/***************************************************************************
 * Function Declarations
 ***************************************************************************/

/**
 * \brief Test case to test AES encryption success case
 * 		  When the size of Plain text is multiple of 16 and
 * 		  no padding is done.
 */
void encryptionNoPadding_SuccessCase();

/**
 * \brief Test case to test AES encryption success case
 * 		  When the size of Plain text is not a multiple of 16 and
 * 		  no padding is done.
 */
void encryptionNoPadding_FailureCase();

/**
 * \brief Test case to test AES encryption success case
 * 		  When the size of Plain text is not a multiple of 16 and
 * 		  padding is done.
 */
void encryptionPadding_SuccessCase();



#endif /* ASW_SERVICES_ENCRYPTION_MANAGER_ENCRYPTMGRTEST_H_ */

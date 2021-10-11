/****************************************************************************
* Filename        : decryptMgrTest.h
* Author          : Vaishnavi Sankaranarayanan
* Description     : Header file that provides API to test the decryption manager
* Created on	  : Jun 28, 2020
****************************************************************************/


#ifndef ASW_SERVICES_DECRYPTION_MANAGER_DECRYPTMGRTEST_H_
#define ASW_SERVICES_DECRYPTION_MANAGER_DECRYPTMGRTEST_H_


/***************************************************************************
 * Includes
 ***************************************************************************/


#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include"decryptMgr.h"


/***************************************************************************
 * Function Declarations
 ***************************************************************************/

	/**
	* \brief Test case to test AES Decryption .
	*		  The EncryptedCipher and IV is predefined in the test case
	* 		  When the size of Plain text is multiple of 16 and
	* 		  no padding is done.
	*/
	void decryptMgrTest_TestCase_Default();
	
	/**
	* \brief Test case to test AES Decryption .
	*		  The Encrypted Text and IV will be taken from the Encryption manager and passed
	*/
	
	void decryptMgrTest_TestCase_EncryptedData(uint8_t* EncryptedText,uint16_t SizeofEncryptedText,uint8_t* IV,uint8_t* stubDhDecryptAesKeyBuf);


#endif /* ASW_SERVICES_DECRYPTION_MANAGER_DECRYPTMGRTEST_H_ */

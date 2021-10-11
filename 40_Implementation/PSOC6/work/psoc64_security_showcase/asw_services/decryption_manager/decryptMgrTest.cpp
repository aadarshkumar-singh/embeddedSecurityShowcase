/****************************************************************************
* Filename        : decryptMgrTest.h
* Author          : Vaishnavi Sankaranarayanan
* Description     : Application file defining Test functions for Decryption Manager
* Created on	  : Jun 28, 2020
****************************************************************************/

/***************************************************************************
 * Includes
 ***************************************************************************/

#include "decryptMgrTest.h"

/***************************************************************************
 * Function definitions
 ***************************************************************************/
 	
	
	/**
	* \brief Test case to test AES Decryption .
	*		  The EncryptedCipher and IV is predefined in the test case
	* 		  When the size of Plain text is multiple of 16 and
	* 		  no padding is done.
	*/
	void decryptMgrTest_TestCase_Default()
	{
		uint8_t EncryptedCipher[]={ \
				0x9f, 0xbf, 0x0b, 0x99, 0x70, 0xe0, 0x3d, 0xab, \
				0xf7, 0x65, 0x43, 0x88, 0x09, 0x2c, 0xb4, 0x66, \
				};

		uint8_t IV[]={
				0x0e, 0x42, 0x75, 0x78, 0xb5, 0x0d, 0x17, 0x4f, \
				0x6e, 0x13, 0xf4, 0xfd, 0x16, 0x30, 0x3e, 0xc7, \
				};
		uint8_t sizeOfEncryptednIV=16;
		decryptMgr obj(EncryptedCipher,IV,sizeOfEncryptednIV,DECRYPT_CBC_NO_PADDING);
		obj.DecryptUsingSymmetricCiphers();
		obj.displayDecryptedMessage();
	}

	/**
	* \brief Test case to test AES Decryption .
	*		  The Encrypted Text and IV will be taken from the Encryption manager and passed
	*/
	
	void decryptMgrTest_TestCase_EncryptedData(uint8_t* EncryptedText,uint16_t SizeofEncryptedText,uint8_t* IV,uint8_t* stubDhDecryptAesKeyBuf)
	{
		decryptMgr obj(EncryptedText,IV,SizeofEncryptedText,DECRYPT_CBC_NO_PADDING);
		if (obj.decryptionDhKeyHandle(stubDhDecryptAesKeyBuf))
		{
			//printf("\n DH Decrypt Key Handle Success \n ");
		}
		obj.DecryptUsingSymmetricCiphers();
		obj.displayDecryptedMessage();
	}

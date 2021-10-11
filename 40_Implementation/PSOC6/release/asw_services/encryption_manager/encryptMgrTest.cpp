/****************************************************************************
* Filename        : encryptMgrTest.cpp
* Author          : Aadarsh Kumar Singh
* Description     : Source file that provides API to test the encryption manager
* Created on	  : Jun 28, 2020
****************************************************************************/

/***************************************************************************
 * Includes
 ***************************************************************************/
#include "encryptMgrTest.h"



/***************************************************************************
 * Function definitions
 ***************************************************************************/

/**
 * \brief Test case to test AES encryption success case
 * 		  When the size of Plain text is multiple of 16 and
 * 		  no padding is done.
 */
void encryptionNoPadding_SuccessCase()
{
  uint8_t plainText[]= {"I am plaintext."};
  encryptMgr encryptObj (plainText,sizeof(plainText),ENCRYPT_CBC_NO_PADDING);
  encryptObj.encryptUsingSymmetricCiphers();
  encryptObj.displayEncryptedMessage();
}

/**
 * \brief Test case to test AES encryption success case
 * 		  When the size of Plain text is not a multiple of 16 and
 * 		  no padding is done.
 */
void encryptionNoPadding_FailureCase()
{
  uint8_t plainText[]= {"I am plaintext.1234"};
  encryptMgr encryptObj (plainText,sizeof(plainText),ENCRYPT_CBC_NO_PADDING);
  encryptObj.encryptUsingSymmetricCiphers();
  encryptObj.displayEncryptedMessage();
}

/**
 * \brief Test case to test AES encryption success case
 * 		  When the size of Plain text is not a multiple of 16 and
 * 		  padding is done.
 */
void encryptionPadding_SuccessCase()
{
  uint8_t plainText[]= {"I am plaintext.1234"};
  encryptMgr encryptObj (plainText,sizeof(plainText),ENCRYPT_CBC_WITH_PADDING);
  encryptObj.encryptUsingSymmetricCiphers();
  encryptObj.displayEncryptedMessage();
}

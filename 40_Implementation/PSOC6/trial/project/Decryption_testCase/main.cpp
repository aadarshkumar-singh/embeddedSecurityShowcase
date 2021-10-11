/*
 * Copyright (c) 2018-2019, Arm Limited and affiliates
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "encryptMgrTest.h"
#include "decryptMgrTest.h"

using namespace std;



Serial pc(USBTX, USBRX); // tx, rx


int main(void)
{
	  pc.printf("-- Begin Mbed Crypto Getting Started --!\n\r");


	  if (psa_crypto_init() != PSA_SUCCESS)
	  {
		  printf("Failed to initialize PSA Crypto\n");
		  return 0;
	  }

	  pc.printf("\n -- Success Case Padding --!\n\r");
	  encryptionPadding_SuccessCase();

	  if (psa_crypto_init() != PSA_SUCCESS)
	  {
		  printf("Failed to initialize PSA Crypto\n");
		  return 0;
	  }

	  pc.printf("\n-- Success Case No Padding --!\n\r");
	  encryptionNoPadding_SuccessCase();

	  if (psa_crypto_init() != PSA_SUCCESS)
	  {
		  printf("Failed to initialize PSA Crypto\n");
		  return 0;
	  }
	  pc.printf("\n -- Failure Case No Padding --!\n\r");
	  encryptionNoPadding_FailureCase();


	  pc.printf("\n-- End Mbed Crypto Getting Started --\n\n\r");

	  pc.printf("\n-- Decryption Started --\n\n\r");

	  if (psa_crypto_init() != PSA_SUCCESS)
	  {
		  printf("Failed to initialize PSA Crypto\n");
		  return 0;
	  }

	  //Executing Default Test Case
 	  //decryptMgrTest_TestCase_Default();

	  if (psa_crypto_init() != PSA_SUCCESS)
	  {
		  printf("Failed to initialize PSA Crypto\n");
		  return 0;
	  }

	  uint8_t plainText[]= {"I am plaintext.1234"};
	  encryptMgr encryptObj (plainText,sizeof(plainText),ENCRYPT_CBC_WITH_PADDING);
	  encryptObj.encryptUsingSymmetricCiphers();
	  encryptObj.displayEncryptedMessage();

	  //Executing Value received from Encrypted data
	 decryptMgrTest_TestCase_EncryptedData(encryptObj.GetEncryptedData(),encryptObj.sizeofEncryption(),encryptObj.GetIvData());

     return 0;
}


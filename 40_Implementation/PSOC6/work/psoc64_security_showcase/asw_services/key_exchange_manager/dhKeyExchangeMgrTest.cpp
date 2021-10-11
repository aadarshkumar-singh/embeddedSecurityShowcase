/****************************************************************************
* Filename        : dhKeyExchangeMgrTest.cpp
* Author          : Aadarsh Kumar Singh
* Description     : Application file defining Test functions for Deffie Helmann
* 					Key exchange algorithm
* Created on	  : Jun 19, 2020
****************************************************************************/

/***************************************************************************
 * Includes
 ***************************************************************************/
#include "dhKeyExchangeMgrTest.h"

/***************************************************************************
 * API definitions
 ***************************************************************************/

/**
 * \brief Test case to check if the shared secret key computed by the ally and the host
 * 		  are equal
 * @return True : When shared secret key matches
 * 		   False : When shared secret key doesn't match
 */
bool dhKeyExchange_matchEncryptionKeys()
{
	uint8_t cli_to_srv[32];
	uint8_t srv_to_cli[32];
	uint8_t encryptionKey1[16];
	uint8_t encryptionKey2[16];
	bool flag = true;

	dhKeyExchangeMgr ctx_cli;
	dhKeyExchangeMgr ctx_srv;

	/* Generating Deffie Helmann key of ally , that has to be exchanged(sent) to host */
	ctx_cli.generateDhKey(cli_to_srv);

	/* Generating Deffie Helmann key of host , this has to be exchanged(sent) to ally */
	ctx_srv.generateDhKey(srv_to_cli);

	/* Shared secret key of the host */
	ctx_srv.computeSharedSecretDhKey(cli_to_srv,encryptionKey1);

	/* Shared secret key of the ally */
	ctx_cli.computeSharedSecretDhKey(srv_to_cli,encryptionKey2);

	/*Check if the shared secret key(used for encryption) of host and ally are equal*/
	for (int i = 0 ; i<16 ; i++)
	{
		if (encryptionKey1[i] != encryptionKey2[i])
		{
			flag = false;
		}
	}

	return flag;
}

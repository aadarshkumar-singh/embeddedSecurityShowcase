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
#include "psa/crypto.h"
#include "mbedtls/version.h"
#include "mbed.h"
#include <string.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>



#define SOME_PLAINTEXT "I am plaintext.I am plaintext."

/* This key is present as a global define for the purpose of example. In
 * real-world applications, you would not have a key hardcoded in source like
 * this. */
static const uint8_t AES_KEY[] =
{
    0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
    0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
};

Serial pc(USBTX, USBRX); // tx, rx

#define  TEST_TOTAL_BLOCK_LENGTH 32

static void encrypt_with_symmetric_ciphers(const uint8_t *key, size_t key_len)
{
    enum {
        block_size = PSA_BLOCK_CIPHER_BLOCK_SIZE(PSA_KEY_TYPE_AES),
    };


    psa_status_t status;
    psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;
    psa_algorithm_t alg = PSA_ALG_CBC_NO_PADDING;
    uint8_t plaintext[TEST_TOTAL_BLOCK_LENGTH] = SOME_PLAINTEXT;
    uint8_t iv[block_size];
    size_t iv_len;
    uint8_t output[TEST_TOTAL_BLOCK_LENGTH];
    size_t output_len;
    psa_key_handle_t handle;
    psa_cipher_operation_t operation = PSA_CIPHER_OPERATION_INIT;

    printf("Encrypt with cipher...\t");
    fflush(stdout);

    /* Initialize PSA Crypto */
    status = psa_crypto_init();
    if (status != PSA_SUCCESS)
    {
        printf("Failed to initialize PSA Crypto\n");
        return;
    }

    /* Import a key */
    psa_set_key_usage_flags(&attributes, PSA_KEY_USAGE_ENCRYPT);
    psa_set_key_algorithm(&attributes, alg);
    psa_set_key_type(&attributes, PSA_KEY_TYPE_AES);
    psa_set_key_bits(&attributes, 128);
    status = psa_import_key(&attributes, key, key_len, &handle);
    if (status != PSA_SUCCESS) {
        printf("Failed to import a key\n");
        return;
    }
    psa_reset_key_attributes(&attributes);

    /* Encrypt the plaintext */
    status = psa_cipher_encrypt_setup(&operation, handle, alg);
    if (status != PSA_SUCCESS) {
        printf("Failed to begin cipher operation\n");
        return;
    }
    status = psa_cipher_generate_iv(&operation, iv, sizeof(iv), &iv_len);
    if (status != PSA_SUCCESS) {
        printf("Failed to generate IV\n");
        return;
    }
    status = psa_cipher_update(&operation, plaintext, sizeof(plaintext),
                               output, sizeof(output), &output_len);
    if (status != PSA_SUCCESS) {
        printf("Failed to update cipher operation\n");
        return;
    }
    status = psa_cipher_finish(&operation, output + output_len,
                               sizeof(output) - output_len, &output_len);
    if (status != PSA_SUCCESS) {
        printf("Failed to finish cipher operation\n");
        return;
    }
	pc.printf("INP:\n");
	for (uint8_t i = 0; i<32; i++)
	{
		pc.putc(plaintext[i]);
	}

	pc.printf("IV:\n");

	for (uint8_t i = 0; i<block_size; i++)
	{
		pc.putc(iv[i]);
	}
	pc.printf("o:\n");

	for (uint8_t i = 0; i<32; i++)
	{

		pc.putc(output[i]);
	}


    /* Clean up cipher operation context */
    psa_cipher_abort(&operation);

    /* Destroy the key */
    psa_destroy_key(handle);

    mbedtls_psa_crypto_free();
}

int main(void)
{



	pc.printf("-- Begin Mbed Crypto Getting Started --!\n\r");

    encrypt_with_symmetric_ciphers(AES_KEY, sizeof(AES_KEY));

    pc.printf("\n-- End Mbed Crypto Getting Started --\n\n\r");

    return 0;
}


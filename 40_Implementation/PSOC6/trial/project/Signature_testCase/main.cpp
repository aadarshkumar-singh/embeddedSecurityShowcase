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
#include <string.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include "signMgr.h"
#include "signVerifyMgr.h"

#define BLINKING_RATE_MS     100
#define DEVICE_SERIAL           1
using namespace std;


int main(void)
{
    printf("-- Begin Mbed Crypto Getting Started --\n\n");

    uint8_t plainText[]= {"I am plaintext."};

    signMgr objSignMgr(plainText);
    objSignMgr.signhash();

    uartData_t data_send = objSignMgr.export_PublicKey();

    signVerifyMgr objVerifyMgr(data_send);

    objVerifyMgr.verifySignature();

    printf("\n-- End Mbed Crypto Getting Started --\n");


    return 0;
}

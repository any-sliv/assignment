/**
 **********************************************************************************
 * File: [gpNvmMap.c]
 * Author: [Maciej Sliwinski]
 * Description: [Non-volatile memory storage component]
 *
 * Copyright (c) 2024 Maciej Sliwinski. All rights reserved.
 * 
 * This software is provided "as is" without any warranties.
 */

#include "../include/gpNvmMap.h"

#ifdef GPNVM_USE_ECC
    const gpNvmBlock GpNvmMap[GPNVM_BLOCKS + 1] =
#else
    const gpNvmBlock GpNvmMap[GPNVM_BLOCKS] =
#endif /* ifdef GPNVM_USE_ECC */
{
    // ******************************************
    // ****** PUT YOUR CODE HERE **** START *****
    // ******************************************
    {
        // ******** Block 0 ********
        .startAddr = (UInt8 *)0x80000,
        .length = 0xA0,
    },
    {
        // ******** Block 1 ********
        .startAddr = (UInt8 *)0x80100,
        .length = 0xFF,
    },
    {
        // ******** Block 2 ********
        .startAddr = (UInt8 *)0x80800,
        .length = 0x80,
    },
    // ******************************************
    // ****** PUT YOUR CODE HERE **** END *******
    // ******************************************
#ifdef GPNVM_USE_ECC
    {
        // ******** Block for redundancy bits ECC ********
        .startAddr = (UInt8 *)GPNVM_FLASH_END - 0xFF,
        .length = 0xFF,
    },
#endif /* ifdef GPNVM_USE_ECC */
};

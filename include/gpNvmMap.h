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

#include <stdlib.h>
#include "gpNvm.h"

// ******************************************
// ****** PUT YOUR CODE HERE **** START *****
// ******************************************

#define GPNVM_FLASH_SIZE (0x1FFF)
#define GPNVM_PAGE_SIZE (0x800)
#define GPNVM_FLASH_START (0x80000)
#define GPNVM_FLASH_END (GPNVM_FLASH_START + GPNVM_FLASH_SIZE)

#define GPNVM_BLOCKS (3)

#define GPNVM_USE_ECC

// ******************************************
// ****** PUT YOUR CODE HERE **** END *******
// ******************************************

#ifdef GPNVM_USE_ECC
    #define GPNVM_SINGLE_PAGE_ECC_SIZE 2
    #define GPNVM_ECC_BLOCK_ID GPNVM_BLOCKS
#endif /* ifdef GPNVM_USE_ECC */

typedef struct {
    UInt8 * startAddr;
    UInt8 length;
} gpNvmBlock;

extern const gpNvmBlock GpNvmMap[];

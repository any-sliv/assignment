/**
 **********************************************************************************
 * File: [gpNvm.c]
 * Author: [Maciej Sliwinski]
 * Description: [Non-volatile memory storage component]
 *
 * Copyright (c) 2024 Maciej Sliwinski. All rights reserved.
 * 
 * This software is provided "as is" without any warranties.
 **********************************************************************************

                    ##### NVM component features #####
    
    [] Interface flash memory with user-defined blocks and manage them based on AttributeId
    [] AttribueId blocks get (read), set (write) and erase
    [] Detect and correct single bit errors in memory


                    ##### HOW TO USE INSTRUCTION #####

    [] This general-purpose NVM component abstracts the flash memory with attributeIds,
        abstraction is done due to pre-defined memory map of blocks which must be done by user.
        Each block has is start address and length which must be valid in terms of memory storage.
    [] Correct NVM map (gpNvmMap.c/.h) configuration includes:
            - Block memory area within flash memory boundaries
            - Not overlapping blocks
            - Length limited to size of 0xFF
            - One block cannot extend beyond one flash memory page
            - Defining properties of your flash memory: SIZE, PAGE_SIZE, FLASH_START, NUMBER_OF_BLOCKS
        ID of a block is its position in map array, thus be careful when referring to block ID.
        Example configuration is available.
    [] ECC mechanism can be enabled (optional) which uses Hamming code to detect and correct single-bit
        errors. ECC works page-wise and its contents are stored in the last 0x100 bytes of flash memory,
        thus user-defined block cannot be defined in this area.
 */

#include <string.h>
#include "gpNvm.h"
#include "gpNvmMap.h"
#include "flash.h"
#include "hamming.h"

enum {
    GPNVM_OK = 0,
    GPNVM_PAGE_NOT_ERASED,
    GPNVM_PARAM_ERR,
    GPNVM_OUT_OF_BOUNDS,
    GPNVM_INCORRECT_ID,
} gpNvmStatus;

enum {
    ECC_SCAN_AND_FIX,
    ECC_UPDATE_PARITY,
} eccOperation;

// ---------------------- GLOBAL VARIABLES ----------------------
// Map of blocks
static gpNvmBlock * const GpNvmBlocks = (gpNvmBlock * const)GpNvmMap;

// ---------------------- LOCAL FUNCTIONS ----------------------
static gpNvm_Result gpNvm_WriteFlash(UInt8 * addr, uint16_t length, UInt8* pValue);
static UInt8 * gpNvm_GetPageStartAddr(gpNvm_AttrId attrId);

#ifdef GPNVM_USE_ECC
static UInt8 * eccGetPageParityAddr(gpNvm_AttrId attrId);
static void eccUpdate(gpNvm_AttrId attrId, UInt8 operation);
#endif /* ifdef GPNVM_USE_ECC */

// ---------------------- FUNCTION DEFINITIONS ----------------------

#ifdef GPNVM_USE_ECC
static UInt8 * eccGetPageParityAddr(gpNvm_AttrId attrId) {
    UInt8 * pageStart = gpNvm_GetPageStartAddr(attrId);
    UInt8 pageNo = (((int)pageStart - GPNVM_FLASH_START) / GPNVM_PAGE_SIZE);
    UInt8 * parityAddr = (UInt8 *)(GpNvmBlocks[GPNVM_ECC_BLOCK_ID].startAddr + 
                                        (pageNo * GPNVM_SINGLE_PAGE_ECC_SIZE));
    return parityAddr;
}

/**
 * @brief Gets start address of page
 * @param attrId data of interest
 * @return Pointer to start address of page where attrId belongs to
 */
static UInt8 * gpNvm_GetPageStartAddr(gpNvm_AttrId attrId) {
    return (UInt8 *)(((int)GpNvmBlocks[attrId].startAddr / GPNVM_PAGE_SIZE) * GPNVM_PAGE_SIZE);
}

/**
 * @brief Update ECC data based on hamming code. ECC works flash page-wise
 * @param attrId desired block of data to be verified
 * @param operation type of operation (scan&fix verifies and fixes content
 *                  and update parity recalculates data from memory and updates parity data)
 */
static void eccUpdate(gpNvm_AttrId attrId, UInt8 operation) {
    UInt8 * pageStart = gpNvm_GetPageStartAddr(attrId);
    UInt8 res;
    // Parity data calculated from memory
    UInt8 calculatedParity[GPNVM_SINGLE_PAGE_ECC_SIZE] = {0};
    // Parity data read from memory
    UInt8 readParity[GPNVM_SINGLE_PAGE_ECC_SIZE] = {0};
    // Buffer holding page data
    UInt8 gpNvmBuffer[GPNVM_PAGE_SIZE] = {0};
    // Address of parity bits storage for page
    UInt8 * parityAddr = eccGetPageParityAddr(attrId);
    
    // Page backup
    res = flashReadData((UInt8 *)pageStart, gpNvmBuffer, GPNVM_PAGE_SIZE);
    if(operation == ECC_SCAN_AND_FIX) {
        // Read parity bits from memory
        flashReadData(parityAddr, readParity, 2);
        // Check data integrity
        if(decodeAndCorrect(gpNvmBuffer, readParity) != GPNVM_OK) {
            // Error detected and corrected
            // Calculate new parity bits for corrected memory data
            calculateParityBits(gpNvmBuffer, calculatedParity);
            // Write corrected parity bits
            res = gpNvm_WriteFlash(parityAddr, sizeof(calculatedParity), calculatedParity);
            if(res == GPNVM_OK) {
                // Write corrected page
                res = gpNvm_WriteFlash((UInt8 *)pageStart, GPNVM_PAGE_SIZE, gpNvmBuffer);
            }
        }
    }
    else if (operation == ECC_UPDATE_PARITY) {
        calculateParityBits(gpNvmBuffer, calculatedParity);
        // Write new parity bits for corresponding page
        storeParityExternally(calculatedParity);
        res = gpNvm_WriteFlash(parityAddr, sizeof(calculatedParity), calculatedParity);
    }
    if(res != GPNVM_OK) {
        // Callback
    }
}
#endif /* ifdef GPNVM_USE_ECC */

/**
 * @brief Reads <attrId> memory block
 * @param pLength Length of read block
 * @param pValue Buffer to copy read data
 * @return gpNvm_Result result of operation
 */
gpNvm_Result gpNvm_GetAttribute(gpNvm_AttrId attrId, UInt8* pLength, UInt8* pValue) {
    gpNvm_Result res = GPNVM_OK;

    if((UInt8)attrId > GPNVM_BLOCKS) {
        res = GPNVM_INCORRECT_ID;
    }
    if(pLength == NULL || pValue == NULL) {
        res = GPNVM_PARAM_ERR;
    }
    if(res == GPNVM_OK) {
#ifdef GPNVM_USE_ECC
        // Check memory ECC before write and fix errors
        eccUpdate(attrId, ECC_SCAN_AND_FIX);
#endif /* ifdef GPNVM_USE_ECC */
        res = flashReadData(GpNvmBlocks[attrId].startAddr, pValue, GpNvmBlocks[attrId].length);
        if(res == GPNVM_OK) {
            // Return read length
            *pLength = GpNvmBlocks[attrId].length;
        }
    }
    return res;
}

/**
 * @brief Program data in specified address in flash memory
 * @param pLength Length of data to be programmed
 * @param pValue Data to be programmed
 * @return gpNvm_Result result of operation
 */
static gpNvm_Result gpNvm_WriteFlash(UInt8 * addr, uint16_t length, UInt8* pValue) {
    // Buffer for page backup (page must be erased before write)
    UInt8 gpNvmBuffer[GPNVM_PAGE_SIZE] = {0};
    UInt8 res = GPNVM_OK;

    UInt8 * pageStart = (UInt8 *)((int)addr / GPNVM_PAGE_SIZE * GPNVM_PAGE_SIZE);
    // Load page into buffer
    res = flashReadData(pageStart, gpNvmBuffer, GPNVM_PAGE_SIZE);
    if(res == GPNVM_OK) {
        if(flashErasePage(addr) != GPNVM_OK) {
            res = GPNVM_PAGE_NOT_ERASED;
        }
        // Calculate new data offset from page start
        int newDataPos = (int)((int)addr - (int)pageStart);
        // Copy new data to page backup
        memcpy(&gpNvmBuffer[newDataPos], pValue, length);
        // Rewrite page with new data
        res = flashWrite(pageStart, gpNvmBuffer, GPNVM_PAGE_SIZE);
    }
    return res;
}

/**
 * @brief Program <attrId> memory block
 * @param pLength Length of data to be programmed
 * @param pValue Data to be programmed
 * @return gpNvm_Result result of operation
 */
gpNvm_Result gpNvm_SetAttribute(gpNvm_AttrId attrId, UInt8 length, UInt8* pValue) {
    gpNvm_Result res = GPNVM_OK;

    if((UInt8)attrId > GPNVM_BLOCKS) {
        res = GPNVM_INCORRECT_ID;
    }
    if(pValue == NULL || length == 0 || length > GpNvmBlocks[attrId].length) {
        res = GPNVM_PARAM_ERR;
    }
#ifdef GPNVM_USE_ECC
    // Detect and fix ECC errors before accessing memory
    eccUpdate(attrId, ECC_SCAN_AND_FIX);
#endif /* ifdef GPNVM_USE_ECC */

    res = gpNvm_WriteFlash(GpNvmBlocks[attrId].startAddr, length, pValue);

#ifdef GPNVM_USE_ECC
    // Update parity due to new data in the block
    eccUpdate(attrId, ECC_UPDATE_PARITY);
#endif /* ifdef GPNVM_USE_ECC */

    return res;
}
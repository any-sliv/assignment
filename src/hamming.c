/**
 **********************************************************************************
 * File: [hamming.c]
 * Author: [Maciej Sliwinski]
 * Description: [Non-volatile memory storage component]
 *
 * This software is provided "as is" without any warranties.
 */

#include <stdio.h>
#include <stdlib.h>
#include "hamming.h"
#include "gpNvmMap.h"

#define PARITY_BITS 14
#define TOTAL_BITS (GPNVM_PAGE_SIZE * 8)
#define DATA_BITS (GPNVM_PAGE_SIZE * 8)

// Function to calculate parity bits based on the data bits
void calculateParityBits(uint8_t *data, uint8_t *parity) {
    for (int i = 0; i < PARITY_BITS; i++) {
        parity[i / 8] &= ~(1 << (i % 8)); // Clear parity bit
    }

    int bitIndex, parityIndex;
    for (parityIndex = 0; parityIndex < PARITY_BITS; parityIndex++) {
        int parityBitPosition = 1 << parityIndex; // 2^parityIndex
        int count = 0;  // Count of 1's for the current parity bit
        for (bitIndex = parityBitPosition - 1; bitIndex < DATA_BITS; bitIndex += 2 * parityBitPosition) {
            for (int j = bitIndex; j < bitIndex + parityBitPosition && j < DATA_BITS; j++) {
                if ((data[j / 8] & (1 << (j % 8))) != 0) {
                    count++;
                }
            }
        }
        // Set the current parity bit based on the count of 1's
        if (count % 2 != 0) { // Odd parity
            parity[parityIndex / 8] |= (1 << (parityIndex % 8));
        }
    }
}

// Function to decode and correct errors in the data
UInt8 decodeAndCorrect(uint8_t *data, uint8_t *parity) {
    uint32_t error_pos = 0;

    for (uint32_t i = 0; i < PARITY_BITS; i++) {
        uint32_t parityBitPosition = 1 << i;
        uint32_t count = 0;
        for (uint32_t j = parityBitPosition - 1; j < DATA_BITS; j += 2 * parityBitPosition) {
            for (uint32_t k = j; k < j + parityBitPosition && k < DATA_BITS; k++) {
                if ((data[k / 8] & (1 << (k % 8))) != 0) {
                    count++;
                }
            }
        }
        if ((count % 2) != ((parity[i / 8] >> (i % 8)) & 1)) {
            error_pos += parityBitPosition;
        }
    }

    if (error_pos != 0 && error_pos <= DATA_BITS) {
        data[(error_pos - 1) / 8] ^= (1 << ((error_pos - 1) % 8));
        return error_pos; // Return position of corrected error
    }
    return 0; // No error or error not correctable
}

// Example function to simulate storing and retrieving parity bits
void storeParityExternally(UInt8 *parity) {
    printf("Stored parity bits: ");
    for (uint32_t i = 0; i < (PARITY_BITS / 8) + (PARITY_BITS % 8 != 0); i++) {
        printf("%02x ", parity[i]);
    }
    printf("\n");
}

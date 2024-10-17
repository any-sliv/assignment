/**
 **********************************************************************************

                THIS FILE IS FOR TEST PURPOSES ONLY

 **********************************************************************************
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "flash.h"

#define FILENAME "flash.txt"

#define FLASH_SIZE 0x2000 // 8192 bytes (address 0x0000 to 0x2000)
#define PAGE_SIZE 0x800
#define FLASH_START 0x80000
#define FLASH_END FLASH_START + FLASH_SIZE

typedef enum {
    FLASH_OK = 0,
    FLASH_PAGE_NOT_ERASED,
    FLASH_PARAM_ERR,
    FLASH_OUT_OF_BOUNDS,
} FlashStatus;

uint8_t Memory[FLASH_SIZE];

static const char * filename = "flash.txt";

static uint8_t * getMemoryAddr(uint8_t * addr) {
    return &Memory[(uintptr_t)addr - FLASH_START];
}

static void parse_memory_line(const char *line, uint8_t memory[], size_t base_address) {
    unsigned int value;
    const char *data_start = line + 7; // Skip the "0xXXXX " (address part)
    // Parse 16 hex values and store them in the memory array
    for (int i = 0; i < 16; i++) {
        if (sscanf(data_start, " 0x%02X", &value) == 1) {
            memory[base_address + i - FLASH_START] = (uint8_t)value;
        }
        data_start += 5; // Move to the next "0xXX " (5 characters per value)
    }
}

void readFromFile(void) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        return;
    }

    char line[256];  // Buffer to hold each line from the file
    size_t address;  // The address read from the file

    // Read the file line by line
    while (fgets(line, sizeof(line), file)) {
        // Read the base address from the line (skip parsing if address invalid or out of bounds)
        if (sscanf(line, "0x%lx", &address) == 1 && address < FLASH_END) {
            // Parse the line and put the values into the memory array at the correct address
            parse_memory_line(line, Memory, address);
        }
    }

    fclose(file);
}

void saveMemoryToFile(void)
{
    // Open the file in write mode
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        perror("Error opening file");
        return;
    }
    // Write the memory contents to the file in the specified format
    for (size_t i = 0; i < FLASH_SIZE; i += 16) {
        // Write the address
        fprintf(file, "0x%04lx ", 0x80000 + i);
        
        // Write 16 bytes of memory content in hexadecimal
        for (size_t j = 0; j < 16 && (i + j) < FLASH_SIZE; ++j) {
            fprintf(file, "0x%02X ", Memory[i + j]);
        }
        
        // New line after every 16 bytes
        fprintf(file, "\n");
    }

    // Close the file
    fclose(file);
}

static bool isPageErased(uint8_t * addr) {
    bool status = true;
    uint32_t pageStart = ((uintptr_t)addr / PAGE_SIZE) * PAGE_SIZE;
    for(uint32_t i = 0; i < PAGE_SIZE ; i++)
    {
        if(Memory[pageStart - FLASH_START] != 0xFF) {
            status = false;
            break; 
        }
    }
    return status;
}

void MemoryInit(void) {
    memset(Memory, 0xFF, sizeof(Memory));
    saveMemoryToFile();
}

uint8_t flashWrite(uint8_t * addr, uint8_t * data, uint16_t len) {
    FlashStatus status = FLASH_OK;
    if(!isPageErased(addr) || !isPageErased((uint8_t *)(((uintptr_t)addr+(uintptr_t)len - 1)))) {
        status = FLASH_PAGE_NOT_ERASED;
    }
    if(addr == NULL || data == NULL || len == 0) {
        status = FLASH_PARAM_ERR;
    }
    if((uintptr_t) addr > FLASH_END || (uintptr_t)addr < FLASH_START) {
        status = FLASH_OUT_OF_BOUNDS;
    }

    if (status == FLASH_OK) {
        memcpy(getMemoryAddr(addr), data, len);
    }

    saveMemoryToFile();
    return (uint8_t)status;
}

uint8_t flashErasePage(uint8_t * addr) {
    FlashStatus status = FLASH_OK;
    if(addr == NULL) {
        status = FLASH_PARAM_ERR;
    }
    if((uintptr_t) addr > FLASH_END || (uintptr_t)addr < FLASH_START) {
        status = FLASH_OUT_OF_BOUNDS;
    }
    if(status == FLASH_OK) {
        uint32_t pageStart = ((uintptr_t)addr / PAGE_SIZE) * PAGE_SIZE;
        memset(getMemoryAddr((uint8_t *)pageStart), 0xFF, PAGE_SIZE);
    }

    saveMemoryToFile();
    return status;
}

uint8_t flashReadData(uint8_t * addr, uint8_t * data, uint16_t length) {
    FlashStatus status = FLASH_OK;
    if(addr == NULL || data == NULL || length == 0) {
        status = FLASH_PARAM_ERR;
    }
    if(((uintptr_t)length + (uintptr_t)addr) > FLASH_END || 
        (uintptr_t)addr < FLASH_START ) {
        status = FLASH_OUT_OF_BOUNDS;
    }
    if(status == FLASH_OK) {
        memcpy(data, getMemoryAddr(addr), length);
    }
    return status;
}
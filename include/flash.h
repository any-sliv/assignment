/**
 **********************************************************************************

                THIS FILE IS FOR TEST PURPOSES ONLY

 **********************************************************************************
 */

#include <stdlib.h>

void MemoryInit(void);

uint8_t flashWrite(uint8_t * addr, uint8_t * data, uint16_t len);

uint8_t flashErasePage(uint8_t * addr);

void readFromFile(void);
void saveMemoryToFile(void);

uint8_t flashReadData(uint8_t * addr, uint8_t * data, uint16_t length);
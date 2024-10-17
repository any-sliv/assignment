#include "gtest/gtest.h"
#include <iostream>
#include <random>
extern "C" {
    #include "../include/gpNvm.h"
    #include "./flash.h"
    #include "../include/gpNvmMap.h"
}
void testSetup();
void testExit();

std::random_device rd;
std::mt19937 gen(rd());

extern uint8_t Memory[];

int getRandomNum(int range) {
    std::uniform_int_distribution<> dist(0, range);
    return dist(gen);
}

void testSetup() {
    readFromFile();
    MemoryInit();
}

void testExit() {
    saveMemoryToFile();
}

TEST(MemoryBasicTest, Test) {
    testSetup();

    gpNvm_Result nvmResult;
    uint8_t blockNo = 1;
    const uint32_t blockSize = 0xFF;
    uint32_t blockStartAddr = 0x80100;
    uint8_t writeData[blockSize] = {0};
    uint8_t readData[blockSize] = {0};
    // Pointer to where block's memory data resides
    uint8_t * blockMemoryPtr = &Memory[blockStartAddr - GPNVM_FLASH_START];

    for (size_t i = 0; i < sizeof(writeData); i++) {
        writeData[i] = getRandomNum(0xFF);
    }

    int res = memcmp(writeData, blockMemoryPtr, blockSize);

    // Verify if memory does not contain what we want to write
    EXPECT_NE(res, 0);

    // Write to memory
    nvmResult = gpNvm_SetAttribute(blockNo /*block*/, blockSize, writeData);

    res = memcmp(writeData, blockMemoryPtr, blockSize);
    // Verify if memory represents data it was programmed with
    EXPECT_EQ(res, 0);
    EXPECT_EQ(nvmResult, 0);
    
    uint8_t len = 0;

    // Read from memory
    nvmResult = gpNvm_GetAttribute(blockNo, &len, readData);

    res = memcmp(readData, writeData, blockSize);
    // Verify if read from memory is what we used to program it with
    EXPECT_EQ(res, 0);
    EXPECT_EQ(nvmResult, 0);
    
    testExit();
}

TEST(EccTest, Test) {
    testSetup();

    gpNvm_Result nvmResult;
    uint8_t blockNo = 1;
    const uint32_t blockSize = 0xFF;
    uint32_t blockStartAddr = 0x80100;
    uint8_t writeData[blockSize] = {0xAA};
    uint8_t readData[blockSize] = {0};
    // Pointer to where block's memory data resides
    uint8_t * blockMemoryPtr = &Memory[blockStartAddr - GPNVM_FLASH_START];
    uint8_t len = 0;

    // Write any data, ECC updates parity bits
    nvmResult = gpNvm_SetAttribute(blockNo, blockSize, writeData);
    EXPECT_EQ(nvmResult, 0);

    // Read data from memory
    nvmResult = gpNvm_GetAttribute(blockNo, &len, readData);
    EXPECT_EQ(nvmResult, 0);

    uint8_t randomNum = getRandomNum(0xFF);
    // Flip bit
    blockMemoryPtr[randomNum] ^= 0x01;
    // Save value of byte with flipped bit
    uint8_t flippedByte = blockMemoryPtr[randomNum];
    // Read data from memory
    nvmResult = gpNvm_GetAttribute(blockNo, &len, readData);
    EXPECT_EQ(nvmResult, 0);
    // At this point corrupted byte should be fixed

    // Compare saved flipped byte with data from memory
    EXPECT_NE(flippedByte, readData[randomNum]);

    testExit();
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

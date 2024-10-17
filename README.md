## NVM component features

- Interface flash memory with user-defined blocks and manage them based on AttributeId
- AttribueId blocks get (read), set (write) and erase
- Detect and correct single bit errors in memory

## File structure
- gpNvm.c/.h Main logic and interface of NVM component
- gpNvmMap.c/.h Configuration of memory blocks
- hamming.c/.h Helper functions for Hamming code parity bits calculation/decoding/fixing
- flash.c/.h File for test purposes only. Serves as flash memory driver, stores memory in text file via C byte array

### HOW TO USE INSTRUCTION

- This general-purpose NVM component abstracts the flash memory with attributeIds,
    abstraction is done due to pre-defined memory map of blocks which must be done by user.
    Each block has is start address and length which must be valid in terms of memory storage.
- Correct NVM map (gpNvmMap.c/.h) configuration includes:
        - Block memory area within flash memory boundaries
        - Not overlapping blocks
        - Length limited to size of 0xFF
        - One block cannot extend beyond one flash memory page
        - Defining properties of your flash memory: SIZE, PAGE_SIZE, FLASH_START, NUMBER_OF_BLOCKS
    ID of a block is its position in map array, thus be careful when referring to block ID.
    Example configuration is available.
- ECC mechanism can be enabled (optional) which uses Hamming code to detect and correct single-bit
    errors. ECC works page-wise and its contents are stored in the last 0x100 bytes of flash memory,
    thus user-defined block cannot be defined in this area.
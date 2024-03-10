#include "../include/simulador-ias/memory.h"

#include <string.h>

int memory_write(int address, int64_t data, void *memory)
{
    // Function to write a 40-bit number to a specific memory line (max = 549755813887, min = -549755813888)

    if (address < 0 || address >= MEMORY_SIZE)
    {
        return -1;
    }

    int offset = address * BYTES_PER_LINE;
    int64_t mask = (1ULL << (BYTES_PER_LINE * 8)) - 1;
    data = data & mask;

    uint8_t *byte_memory = (uint8_t *)memory + offset;
    memcpy(byte_memory, &data, BYTES_PER_LINE);

    return 0;
}

int memory_read(int address, int64_t *output, void *memory)
{
    // Function to read a 40-bit number from a specific memory line

    if (address < 0 || address >= MEMORY_SIZE)
    {
        return -1;
    }

    int offset = address * BYTES_PER_LINE;
    *output = 0;
    uint8_t *byte_memory = (uint8_t *)memory + offset;

    // Copy the 5 bytes from memory to the output variable
    memcpy(output, byte_memory, BYTES_PER_LINE);

    if (*output & (1ULL << 39)) // Check if the 40th bit is 1 (indicating a negative number)
    {
        // If so, set the upper 24 bits to 1 (sign extension)
        *output |= (-1ULL << 40);
    }

    return 0;
}

#include "memory.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>

int memory_write(int n_line, uint64_t data, void *memory)
{
    // Function to write a 40-bit number to a specific memory line (max = 1099511627775)

    if (n_line < 0 || n_line >= MEMORY_SIZE)
    {
        return -1;
    }

    int offset = n_line * BYTES_PER_LINE;
    uint64_t mask = (1ULL << (BYTES_PER_LINE * 8)) - 1;
    data = data & mask;

    uint8_t *byte_memory = (uint8_t *)memory + offset;
    memcpy(byte_memory, &data, BYTES_PER_LINE);

    return 0;
}

int memory_read(int n_line, uint64_t *output, void *memory)
{
    // Function to read a 40-bit number from a specific memory line

    if (n_line < 0 || n_line >= MEMORY_SIZE)
    {
        return -1;
    }

    int offset = n_line * BYTES_PER_LINE;
    *output = 0;
    uint8_t *byte_memory = (uint8_t *)memory + offset;
    memcpy(output, byte_memory, BYTES_PER_LINE);

    return 0;
}

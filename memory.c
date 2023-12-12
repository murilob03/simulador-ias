#include "memory.h"

#include <stdio.h>
#include <stdint.h>

// Function to clear the main memory
void clear_memory(MemoryLine *memory)
{
    for (int i = 0; i < MEMORY_SIZE; i++)
    {
        for (int j = 0; j < BITS_PER_LINE / 8; j++)
        {
            memory[i].data[j] = 0;
        }
    }
}

// Function to write a 40-bit number to a specific memory line (max = 1099511627775)
void write_to_memory(int line_number, uint64_t data, MemoryLine *memory)
{
    if (line_number >= 0 && line_number < MEMORY_SIZE)
    {
        // Ensure only the lower 40 bits are used
        data &= ((uint64_t)1 << BITS_PER_LINE) - 1;

        // Copy the data to the memory line
        for (int i = 0; i < BITS_PER_LINE / 8; i++)
        {
            unsigned char byte = (data >> (8 * i)) & 0xFF;
            memory[line_number].data[i] = byte;
        }

        printf("Wrote %lu to memory line %d\n", data, line_number);
    }
    else
    {
        printf("Invalid memory line number\n");
    }
}

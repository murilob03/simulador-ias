#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>

#define MEMORY_SIZE 1024
#define BITS_PER_LINE 40

// Define a structure to represent a memory line
typedef struct
{
    uint8_t data[BITS_PER_LINE / 8];
} MemoryLine;



// Function to clear the main memory
void clear_memory(MemoryLine *memory);

// Function to write a 40-bit number to a specific memory line (max = 1099511627775)
void write_to_memory(int line_number, uint64_t data, MemoryLine *memory);

#endif
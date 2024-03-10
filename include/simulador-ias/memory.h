#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>

#define MEMORY_SIZE 4096
#define BYTES_PER_LINE 5

#define memory_init malloc(MEMORY_SIZE * BYTES_PER_LINE) 

// Function to write a 40-bit number to a specific memory line (max = 1099511627775)
int memory_write(int address, int64_t data, void *memory);

// Function to read a 40-bit number from a specific memory line
int memory_read(int address, int64_t *output, void *memory);

#endif
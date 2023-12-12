#include "memory.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

// Function to print the content of a specific memory line in decimal
void print_memory_line_dec(int line_number, MemoryLine *memory)
{
    if (line_number >= 0 && line_number < MEMORY_SIZE)
    {
        printf("Memory line %d: ", line_number);

        void *ptr = &memory[line_number];
        uint64_t value = (*(uint64_t *)ptr) & 0xFFFFFFFFFF;

        printf("%lu\n", value);
    }
    else
    {
        printf("Invalid memory line number\n");
    }
}

// Function to print the content of a specific memory line in binary
void print_memory_line_bin(int line_number, MemoryLine *memory)
{
    if (line_number >= 0 && line_number < MEMORY_SIZE)
    {
        printf("Memory line %d: ", line_number);

        for (int i = (BITS_PER_LINE / 8) - 1; i >= 0; i--)
        {
            for (int j = 0; j < 8; j++)
            {
                printf("%d", (memory[line_number].data[i] >> (7 - j)) & 1);
            }
            printf(" ");
        }
        printf("\n");
    }
    else
    {
        printf("Invalid memory line number\n");
    }
}

int main()
{
    MemoryLine *memory = malloc(MEMORY_SIZE * sizeof(MemoryLine));

    // Example usage
    write_to_memory(0, 12345, memory);         // Write the number 12345 to memory line 0
    write_to_memory(1, 9876543210, memory);    // Write a larger number to memory line 1
    write_to_memory(2, 1099511627775, memory); // Write a larger number to memory line 1

    // Print the content of both memory lines
    print_memory_line_dec(0, memory);
    print_memory_line_dec(1, memory);
    print_memory_line_dec(2, memory);

    print_memory_line_bin(2, memory);

    return 0;
}

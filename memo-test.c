#include "memory.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

// // Function to print the content of a specific memory line in decimal
// void print_memory_line_dec(int line_number, MemoryLine *memory)
// {
//     if (line_number >= 0 && line_number < MEMORY_SIZE)
//     {
//         printf("Memory line %d: ", line_number);

//         void *ptr = &memory[line_number];
//         uint64_t value = (*(uint64_t *)ptr) & 0xFFFFFFFFFF;

//         printf("%lu\n", value);
//     }
//     else
//     {
//         printf("Invalid memory line number\n");
//     }
// }

// // Function to print the content of a specific memory line in binary
// void print_memory_line_bin(int line_number, MemoryLine *memory)
// {
//     if (line_number >= 0 && line_number < MEMORY_SIZE)
//     {
//         printf("Memory line %d: ", line_number);

//         for (int i = (BITS_PER_LINE / 8) - 1; i >= 0; i--)
//         {
//             for (int j = 0; j < 8; j++)
//             {
//                 printf("%d", (memory[line_number].data[i] >> (7 - j)) & 1);
//             }
//             printf(" ");
//         }
//         printf("\n");
//     }
//     else
//     {
//         printf("Invalid memory line number\n");
//     }
// }

int main()
{
    void *memory = malloc(MEMORY_SIZE * BYTES_PER_LINE);

    // Example usage
    memory_write(0, 12345, memory);         // Write the number 12345 to memory line 0
    memory_write(1, 9876543210, memory);    // Write a large number to memory line 1
    memory_write(2, 1099511627775, memory); // Write the max number to memory line 2

    // Print the content of both memory lines
    uint64_t value = 0;
    memory_read(0, &value, memory);
    printf("Memory line 0: %lu\n", value);

    memory_read(1, &value, memory);
    printf("Memory line 1: %lu\n", value);

    memory_read(2, &value, memory);
    printf("Memory line 2: %lu\n", value);

    return 0;
}

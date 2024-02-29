#include "memory.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <inttypes.h>

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
    void *memory = memory_init;

    // Example usage
    memory_write(0, 12345, memory);         // Write the number 12345 to memory line 0
    memory_write(1, 9876543210, memory);    // Write a large number to memory line 1
    memory_write(2, 549755813887, memory);  // Write the max number to memory line 2
    memory_write(3, -158, memory);          // Write -158 to memory line 3
    memory_write(4, -1, memory);            // Write -1 to memory line 4

    // Print the content of both memory lines
    int64_t value = 0;
    memory_read(0, &value, memory);
    printf("Memory line 0: %" PRId64 "\n", value);

    memory_read(1, &value, memory);
    printf("Memory line 1: %" PRId64 "\n", value);

    memory_read(2, &value, memory);
    printf("Memory line 2: %" PRId64 "\n", value);

    memory_read(3, &value, memory);
    printf("Memory line 3: %" PRId64 "\n", value);

    memory_read(4, &value, memory);
    printf("Memory line 4: %" PRId64 "\n", value);

    return 0;
}

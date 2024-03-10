#ifndef CONVERSOR_H
#define CONVERSOR_H

// Function to convert an ias instruction file to a memory
void write_memory(void *memory, char *input_file);

// Function to get the number of cycles of each operation
void get_op_cycles(int *op_cycles, char *input_file);

#endif
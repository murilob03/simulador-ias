#ifndef CONVERSOR_H
#define CONVERSOR_H

// Função para converter um arquivo de instruções ias e escrever na memória
void write_memory(void *memory, const char *input_file);

// Função para obter o número de ciclos de cada operação a partir de um arquivo
void get_op_cycles(int *op_cycles, const char *input_file);

#endif
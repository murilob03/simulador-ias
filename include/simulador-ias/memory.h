#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>

#define MEMORY_SIZE 4096
#define BYTES_PER_LINE 5

#define memory_init malloc(MEMORY_SIZE *BYTES_PER_LINE)

// Função para escrever um número de 40 bits em um endereço
int memory_write(int address, int64_t data, void *memory);

// Função para ler um número de 40 bits a partir de um endereço
int memory_read(int address, int64_t *output, void *memory);

// Função para limpar a memória
void memory_clear(void *memory);

#endif
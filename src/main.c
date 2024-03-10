#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>

#include "../include/simulador-ias/memory.h"
#include "../include/simulador-ias/conversor.h"
#include "../include/simulador-ias/ias.h"

int main(int argc, char const *argv[])
{
    // Verifica se o número correto de argumentos foi fornecido
    if (argc != 5)
    {
        printf("Uso: %s -p XXX.ias -i YYY\n", argv[0]);
        return 1;
    }

    // Define o número padrão de ciclos para cada instrução
    int op_cycles[34];
    op_cycles[1] = 1;  // LOAD M(X)
    op_cycles[2] = 1;  // LOAD -M(X)
    op_cycles[3] = 1;  // LOAD |M(X)|
    op_cycles[4] = 1;  // LOAD -|M(X)|
    op_cycles[5] = 1;  // ADD M(X)
    op_cycles[6] = 1;  // SUB M(X)
    op_cycles[7] = 1;  // ADD |M(X)|
    op_cycles[8] = 1;  // SUB |M(X)|
    op_cycles[9] = 1;  // LOAD MQ,M(X)
    op_cycles[10] = 1; // LOAD MQ
    op_cycles[11] = 1; // MUL M(X)
    op_cycles[12] = 1; // DIV M(X)
    op_cycles[13] = 1; // JUMP M(X,0:19)
    op_cycles[14] = 1; // JUMP M(X,20:39)
    op_cycles[15] = 1; // JUMP +M(X,0:19)
    op_cycles[16] = 1; // JUMP +M(X,20:39)
    op_cycles[18] = 1; // STOR M(X,8:19)
    op_cycles[19] = 1; // STOR M(X,28:39)
    op_cycles[20] = 1; // LSH
    op_cycles[21] = 1; // RSH
    op_cycles[33] = 1; // STOR M(X)

    // Obtém o número de ciclos para cada instrução
    get_op_cycles(op_cycles, argv[2]);

    // Escreve as instruções na memória
    void *memory = memory_init;
    memory_clear(memory);
    write_memory(memory, argv[2]);

    // Processa as instruções
    processador(atoi(argv[4]), memory, op_cycles);

    // Percorre a memória de trás para frente até encontrar o primeiro valor diferente de zero
    int64_t value = 0;
    int i = 4095;
    while (value == 0 && i >= 0)
    {
        memory_read(i, &value, memory);
        i--;
    }

    // Imprime a memória e escreve a saída em um arquivo
    char *output_filename = malloc(strlen(argv[2]) + 5);
    strcpy(output_filename, argv[2]);
    strcat(output_filename, ".out");
    FILE *output = fopen(output_filename, "w");

    for (int j = 0; j <= i; j++)
    {
        memory_read(j, &value, memory);
        printf("%d: %" PRId64 "\n", j, value);
        fprintf(output, "%d: %" PRId64 "\n", j, value);
    }

    fclose(output);
}

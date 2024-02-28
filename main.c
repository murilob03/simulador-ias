#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "memory.h"

// Estrutura para o banco de registradores
struct BR
{
    // program counter
    uint64_t PC;
    // memory address register
    uint64_t MAR;
    // instruction register
    uint64_t IR;
    // instruction buffer register
    uint64_t IBR;
    // memory buffer register
    uint64_t MBR;
    // accumulator
    uint64_t AC;
    // multiply-cotient register
    uint64_t MQ;
};

void barramento(int is_write, void* memory, struct BR *banco)
{
    if (is_write)
    {
        memory_write(banco->MAR, banco->MBR, memory);
    }
    else
    {
        memory_read(banco->MAR, &banco->MBR, memory);
    }
}

// Unidade de Controle
void UC(int estagio, void* memory, struct BR *banco)
{
    switch (estagio)
    {
    case 0:
        // busca
        break;
    case 1:
        // decodificação
        break;
    case 2:
        // busca operando
        break;
    case 3:
        // execução
        break;
    case 4:
        // escrita resultado
        break;
    default:
        break;
    }
}

// Unidade Lógica e Aritmética
void ULA(int op, struct BR *banco)
{
    switch (op)
    {
    case 0:
        // add
        banco->AC += banco->MBR;
        break;
    case 1:
        // sub
        banco->AC -= banco->MBR;
        break;
    case 2:
        // mul
        banco->AC *= banco->MBR;
        break;
    case 3:
        // div
        banco->AC /= banco->MBR;
        break;
    case 4:
        // abs
        banco->AC = abs(banco->AC);
        break;
    case 5:
        // neg
        banco->AC = -banco->AC;
        break;
    case 6:
        // right shift
        banco->AC >>= banco->AC;
        break;
    case 7:
        // left shift
        banco->AC <<= banco->AC;
        break;
    default:
        break;
    }
    
}

void processador()
{

}

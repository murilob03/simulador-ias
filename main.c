#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "memory.h"

// Estrutura para o banco de registradores
struct BR
{
    // program counter
    int64_t PC;
    // memory address register
    int64_t MAR;
    // instruction register
    int64_t IR;
    // instruction buffer register
    int64_t IBR;
    // memory buffer register
    int64_t MBR;
    // accumulator
    int64_t AC;
    // multiply-cotient register
    int64_t MQ;
};

struct aux
{
    int left_necessary;
    int memory_search_necessary;
    int memory_write_necessary;
    int ULA_op;
    int memory_op;
};

void barramento(int is_write, void *memory, struct BR *banco)
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
void UC(int estagio, void *memory, struct BR *banco, struct aux *signal)
{
    switch (estagio)
    {
    case 0:
        // busca
        if (banco->IBR == 0)
        {
            banco->MAR = banco->PC;
            barramento(0, memory, banco);
            if (signal->left_necessary)
            {
                banco->IBR = banco->MBR;
                banco->IR = banco->MBR >> 32;
                banco->MAR = (banco->MBR >> 20) & 0xFFF;
                banco->PC++;
            }
            else
            {
                banco->IR = (banco->MBR >> 12) & 0xFF;
                banco->MAR = banco->IBR & 0xFFF;
                signal->left_necessary = 1;
            }
        }
        else
        {
            banco->IR = banco->IBR >> 12;
            banco->MAR = banco->IBR & 0xFFF;
            banco->IBR = 0;
        }

        break;
    case 1:
        // decodificação
        switch (banco->IR)
        {
        case 0b00001010:
            // load mq
            signal->ULA_op = -1; // do nothing
            signal->memory_search_necessary = 0;
            signal->memory_write_necessary = 0;
            break;
        case 0b00001001:
            // load mm
            signal->ULA_op = -1; // do nothing
            signal->memory_search_necessary = 1;
            signal->memory_write_necessary = 0;
            break;
        case 0b00100001:
            // stor
            signal->ULA_op = -1; // do nothing
            signal->memory_search_necessary = 0;
            signal->memory_write_necessary = 1;
            break;
        case 0b00000001:
            // load M(X)
            signal->ULA_op = -1; // do nothing
            signal->memory_search_necessary = 1;
            signal->memory_write_necessary = 0;
            break;
        case 0b00000010:
            // load -M(X)
            signal->ULA_op = 5; // neg
            signal->memory_search_necessary = 1;
            signal->memory_write_necessary = 0;
            break;
        case 0b00000011:
            // load |M(X)|
            signal->ULA_op = 4; // abs
            signal->memory_search_necessary = 1;
            signal->memory_write_necessary = 0;
            break;
        case 0b00000100:
            // load -|M(X)|
            signal->ULA_op = 5; // neg
            signal->memory_search_necessary = 1;
            signal->memory_write_necessary = 0;
            break;
        case 0b00001101:
            // jump M(X,0:19)
            signal->ULA_op = -1; // do nothing
            signal->memory_search_necessary = 0;
            signal->memory_write_necessary = 0;
            break;
        case 0b00001110:
            // jump M(X,20:39)
            signal->ULA_op = -1; // do nothing
            signal->memory_search_necessary = 0;
            signal->memory_write_necessary = 0;
            signal->left_necessary = 0;
            break;
        case 0b00001111:
            // jump +M(X,0:19)
            signal->ULA_op = -1; // do nothing
            signal->memory_search_necessary = 0;
            signal->memory_write_necessary = 0;
            break;
        case 0b00010000:
            // jump +M(X,20:39)
            signal->ULA_op = -1; // do nothing
            signal->memory_search_necessary = 0;
            signal->memory_write_necessary = 0;
            signal->left_necessary = 0;
            break;
        case 0b00000101:
            // add M(X)
            signal->ULA_op = 0; // add
            signal->memory_search_necessary = 1;
            signal->memory_write_necessary = 0;
            break;
        case 0b00000111:
            // add |M(X)|
            signal->ULA_op = 0; // add
            signal->memory_search_necessary = 1;
            signal->memory_write_necessary = 0;
            break;
        case 0b00000110:
            // sub M(X)
            signal->ULA_op = 1; // sub
            signal->memory_search_necessary = 1;
            signal->memory_write_necessary = 0;
            break;
        case 0b00001000:
            // sub |M(X)|
            signal->ULA_op = 1; // sub
            signal->memory_search_necessary = 1;
            signal->memory_write_necessary = 0;
            break;
        case 0b00001011:
            // mul M(X)
            signal->ULA_op = 2; // mul
            signal->memory_search_necessary = 1;
            signal->memory_write_necessary = 0;
            break;
        case 0b00001100:
            // div M(X)
            signal->ULA_op = 3; // div
            signal->memory_search_necessary = 1;
            signal->memory_write_necessary = 0;
            break;
        case 0b00010100:
            // lsh
            signal->ULA_op = 7; // lsh
            signal->memory_search_necessary = 0;
            signal->memory_write_necessary = 0;
            break;
        case 0b00010101:
            // rsh
            signal->ULA_op = 6; // rsh
            signal->memory_search_necessary = 0;
            signal->memory_write_necessary = 0;
            break;
        case 0b00010010:
            // stor M(X,8:19)
            signal->ULA_op = -1; // do nothing
            signal->memory_search_necessary = 0;
            signal->memory_write_necessary = 1;
            break;
        case 0b00010011:
            // stor M(X,28:39)
            signal->ULA_op = -1; // do nothing
            signal->memory_search_necessary = 0;
            signal->memory_write_necessary = 1;
            break;
        default:
            // unknown instruction
            break;
        }

        break;
    case 2:
        // busca operando
        if (signal->memory_search_necessary)
        {
            barramento(0, memory, banco);
        }
        
        break;
    case 3:
        // execução
        if (signal->ULA_op != -1)
        {
            ULA(signal->ULA_op, banco);
        }
        break;
    case 4:
        // escrita resultado
        if (signal->memory_write_necessary)
        {
            barramento(1, memory, banco);
        }
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
        banco->MQ = banco->AC / banco->MBR;
        banco->AC = banco->AC % banco->MBR;
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

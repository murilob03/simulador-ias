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
    int decoded_op;
    int memory_op[2];
    int counter;
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
void UC(int estagio, void *memory, struct BR *banco, struct aux *signal, int *n_cycles)
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
            signal->decoded_op = 0;
            break;
        case 0b00001001:
            // load mm
            signal->decoded_op = 1;
            signal->memory_search_necessary = 1;
            break;
        case 0b00100001:
            // stor
            signal->decoded_op = 2;
            break;
        case 0b00000001:
            // load M(X)
            signal->decoded_op = 3;
            signal->memory_search_necessary = 1;
            break;
        case 0b00000010:
            // load -M(X)
            signal->decoded_op = 4;
            signal->memory_search_necessary = 1;
            signal->memory_op[0] = 5; // neg
            break;
        case 0b00000011:
            // load |M(X)|
            signal->decoded_op = 5;
            signal->memory_search_necessary = 1;
            signal->memory_op[0] = 4; // abs
            break;
        case 0b00000100:
            // load -|M(X)|
            signal->decoded_op = 6;
            signal->memory_search_necessary = 1;
            signal->memory_op[0] = 4; // abs
            signal->memory_op[1] = 5; // neg
            break;
        case 0b00001101:
            // jump M(X,0:19)
            signal->decoded_op = 7;
            break;
        case 0b00001110:
            // jump M(X,20:39)
            signal->decoded_op = 8;
            break;
        case 0b00001111:
            // jump +M(X,0:19)
            signal->decoded_op = 9;
            break;
        case 0b00010000:
            // jump +M(X,20:39)
            signal->decoded_op = 10;
            break;
        case 0b00000101:
            // add M(X)
            signal->decoded_op = 11;
            signal->memory_search_necessary = 1;
            break;
        case 0b00000111:
            // add |M(X)|
            signal->decoded_op = 12;
            signal->memory_search_necessary = 1;
            signal->memory_op[0] = 4; // abs
            break;
        case 0b00000110:
            // sub M(X)
            signal->decoded_op = 13;
            signal->memory_search_necessary = 1;
            break;
        case 0b00001000:
            // sub |M(X)|
            signal->decoded_op = 14;
            signal->memory_search_necessary = 1;
            signal->memory_op[0] = 4; // abs
            break;
        case 0b00001011:
            // mul M(X)
            signal->decoded_op = 15;
            signal->memory_search_necessary = 1;
            break;
        case 0b00001100:
            // div M(X)
            signal->decoded_op = 16;
            signal->memory_search_necessary = 1;
            break;
        case 0b00010100:
            // lsh
            signal->decoded_op = 17;
            break;
        case 0b00010101:
            // rsh
            signal->decoded_op = 18;
            break;
        case 0b00010010:
            // stor M(X,8:19)
            signal->decoded_op = 19;
            signal->memory_search_necessary = 1;
            break;
        case 0b00010011:
            // stor M(X,28:39)
            signal->decoded_op = 20;
            signal->memory_search_necessary = 1;
            break;
        }
        break;
    case 2:
        // busca operando
        if (signal->memory_search_necessary)
        {
            barramento(0, memory, banco);
            signal->memory_search_necessary = 0;
        }
        
        break;
    case 3:
        // execução
        
        // conta os ciclos para cada operação
        if (signal->counter > 0)
        {
            signal->counter--;
            break;
        }
        else
        {
            signal->counter = n_cycles[signal->decoded_op] - 1;
        }

        // executa operações da ULA
        exec_ula_ops(banco, signal);

        // executa a operação em si
        if (signal->decoded_op > 2 && signal->decoded_op < 7)
            banco->AC = banco->MBR;

        switch (signal->decoded_op)
        {
            case 0:
                // load mq
                banco->AC = banco->MQ;
                break;
            case 1:
                // load mm
                banco->MQ = banco->MAR;
                break;
            case 2:
                // stor
                banco->MBR = banco->AC;
                break;
            case 7:
                // jump M(X,0:19)
                banco->PC = banco->MAR;
                break;
            case 8:
                // jump M(X,20:39)
                banco->PC = banco->MAR;
                signal->left_necessary = 0;
                break;
            case 9:
                // jump +M(X,0:19)
                if (banco->AC >= 0)
                {
                    banco->PC = banco->MAR;
                }
                break;
            case 10:
                // jump +M(X,20:39)
                if (banco->AC >= 0)
                {
                    banco->PC = banco->MAR;
                    signal->left_necessary = 0;
                }
                break;
            case 19:
                // stor M(X,8:19)
                int64_t mask = ~((int64_t)(0xFFF) << 8);
                banco->MBR = (banco->MBR & mask);

                banco->AC = banco->AC << 8;
                banco->MBR = banco->MBR | banco->AC;
                break;
            case 20:
                // stor M(X,28:39)
                banco->MBR = banco->MBR | banco->AC;
                break;
        }

        break;
    case 4:
        // escrita resultado
        if (signal->memory_write_necessary)
        {
            barramento(1, memory, banco);
            signal->memory_write_necessary = 0;
        }
        break;
    default:
        break;
    }
}

// Unidade Lógica e Aritmética
void ULA(int use_ac, int op, struct BR *banco)
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
        if (use_ac)
            banco->AC = abs(banco->AC);
        else
            banco->MBR = abs(banco->MBR);
        break;
    case 5:
        // neg
        if (use_ac)
            banco->AC = -banco->AC;
        else
            banco->MBR = -banco->MBR;
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

void exec_ula_ops(struct BR *banco, struct aux *signal)
{
    if (signal->memory_op[0] != -1)
    {
        ULA(0, signal->memory_op[0], banco);
    }

    if (signal->memory_op[1] != -1)
    {
        ULA(0, signal->memory_op[1], banco);
    }

    if (signal->ULA_op != -1)
    {
        ULA(1, signal->ULA_op, banco);
    }
}
    

void processador()
{
}

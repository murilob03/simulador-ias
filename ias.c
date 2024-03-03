#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "memory.h"

// Estrutura para o banco de registradores
typedef struct
{
    int64_t PC;  // program counter
    int64_t MAR; // memory address register
    int64_t IR;  // instruction register
    int64_t IBR; // instruction buffer register
    int64_t MBR; // memory buffer register
    int64_t AC;  // accumulator
    int64_t MQ;  // multiply-cotient register
} IAS_REGS;

// Estrutura para o registrador entre IF e ID
typedef struct
{
    // Fields for IF/ID pipeline register
    int64_t IR;
    int64_t MAR;
} IF_ID;

// Estrutura para o registrador entre ID e OF
typedef struct
{
    int opcode;    // The decoded opcode
    int enable_of; // Enable the OF stage
    int64_t MAR;   // The address of the operand
} ID_OF;

// Estrutura para o registrador entre OF e EX
typedef struct
{
    int opcode;  // The decoded opcode
    int64_t MAR; // The address
    int64_t MBR; // The operand
} OF_EX;

// Estrutura para o registrador entre EX e WB
typedef struct
{
    int enable_wb; // Enable the WB stage
    int64_t MAR;   // The address to write
    int64_t AC;    // The result of the operation
} EX_WB;

// Estrutura para agrupar os registros do pipeline
typedef struct
{
    IF_ID *if_id;
    ID_OF *id_of;
    OF_EX *of_ex;
    EX_WB *ex_wb;
} pipeline_regs;

// Estrutura para os sinais de controle
typedef struct
{
    int left_necessary;
    int counter;
} control_signals;

void barramento(int is_write, void *memory, IAS_REGS *banco)
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
void UC(IAS_REGS *banco, control_signals *signal, pipeline_regs p_rgs, int *n_cycles, void *memory, int estagio)
{
    switch (estagio)
    {
    case 0:
        // busca
        busca(banco, signal, memory, p_rgs.if_id);
        break;
    case 1:
        // decodificação
        decodifica(banco, p_rgs.if_id, p_rgs.id_of, signal);
        break;
    case 2:
        // busca operando
        busca_operando(banco, p_rgs.id_of, p_rgs.of_ex, signal, memory);
        break;
    case 3:
        // execução
        executa(banco, p_rgs.of_ex, p_rgs.ex_wb, signal, n_cycles);
        break;
    case 4:
        // escrita resultado
        escreve_resultado(banco, p_rgs.ex_wb, memory);
        break;
    }
}

// Unidade Lógica e Aritmética
void ULA(int use_mbr, int op, IAS_REGS *banco)
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
        if (use_mbr)
            banco->MBR = abs(banco->MBR);
        else
            banco->AC = abs(banco->AC);
        break;
    case 5:
        // neg
        if (use_mbr)
            banco->MBR = -banco->MBR;
        else
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

void busca(IAS_REGS *banco, struct aux *signal, void *memory, IF_ID *if_id)
{
    // busca
    banco->MAR = banco->PC;
    barramento(0, memory, banco);

    if_id->IR = banco->IR;

    banco->PC++;
}

void decodifica(IAS_REGS *banco, IF_ID *if_id, ID_OF *id_of, control_signals *signal)
{
    // decodificação
    if (banco->IBR == 0)
    {
        if (signal->left_necessary)
        {
            banco->IBR = banco->MBR;
            banco->IR = banco->MBR >> 32;
            banco->MAR = (banco->MBR >> 20) & 0xFFF;
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

    id_of->MAR = banco->MAR;
    id_of->opcode = banco->IR;
}

void busca_operando(IAS_REGS *banco, ID_OF *id_of, OF_EX *of_ex, struct aux *signal, void *memory)
{
    // busca operando
    banco->MAR = id_of->MAR;
    barramento(0, memory, banco);

    of_ex->MBR = banco->MBR;
    of_ex->MAR = id_of->MAR;
    of_ex->opcode = id_of->opcode;
}

void executa(IAS_REGS *banco, OF_EX *of_ex, EX_WB *ex_wb, control_signals *signal, int *n_cycles)
{
    // execução
    // conta os ciclos para cada operação
    if (signal->counter > 0)
    {
        signal->counter--;
        return;
    }
    else
    {
        signal->counter = n_cycles[of_ex->opcode] - 1;
    }

    // executa a operação em si
    switch (of_ex->opcode)
    {
    case 0b00001010:
        // load mq
        banco->AC = banco->MQ;
        break;
    case 0b00001001:
        // load mm
        banco->MQ = banco->MBR;
        break;
    case 0b00100001:
        // stor
        ex_wb->enable_wb = 1;
        break;
    case 0b00000001:
        // load M(X)
        banco->AC = banco->MBR;
        break;
    case 0b00000010:
        // load -M(X)
        banco->AC = banco->MBR;
        ULA(0, 5, banco); // neg
        break;
    case 0b00000011:
        // load |M(X)|
        banco->AC = banco->MBR;
        ULA(0, 4, banco); // abs
        break;
    case 0b00000100:
        // load -|M(X)|
        banco->AC = banco->MBR;
        ULA(0, 4, banco); // abs
        ULA(0, 5, banco); // neg
        break;
    case 0b00001101:
        // jump M(X,0:19)
        banco->PC = banco->MBR & 0xFFF;
        break;
    case 0b00001110:
        // jump M(X,20:39)
        banco->PC = (banco->MBR >> 20) & 0xFFF;
        break;
    case 0b00001111:
        // jump +M(X,0:19)
        if (banco->AC >= 0)
        {
            banco->PC = banco->MBR & 0xFFF;
        }
        break;
    case 0b00010000:
        // jump +M(X,20:39)
        if (banco->AC >= 0)
        {
            banco->PC = (banco->MBR >> 20) & 0xFFF;
        }
        break;
    case 0b00000101:
        // add M(X)
        ULA(0, 0, banco); // add
        break;
    case 0b00000111:
        // add |M(X)|
        ULA(1, 4, banco); // abs
        ULA(0, 0, banco); // add
        break;
    case 0b00000110:
        // sub M(X)
        ULA(0, 1, banco); // sub
        break;
    case 0b00001000:
        // sub |M(X)|
        ULA(1, 4, banco); // abs
        ULA(0, 1, banco); // sub
        break;
    case 0b00001011:
        // mul M(X)
        ULA(0, 2, banco); // mul
        break;
    case 0b00001100:
        // div M(X)
        ULA(0, 3, banco); // div
        break;
    case 0b00010100:
        // lsh
        ULA(0, 7, banco); // left shift
        break;
    case 0b00010101:
        // rsh
        ULA(0, 6, banco); // right shift
        break;
    case 0b00010010:
        // stor M(X,8:19)
        int64_t mask = ~((int64_t)(0xFFF) << 8);
        banco->MBR = (banco->MBR & mask);

        banco->AC = banco->AC << 8;
        banco->MBR = banco->MBR | banco->AC;
        break;
    case 0b00010011:
        // stor M(X,28:39)
        banco->MBR = banco->MBR | banco->AC;
        break;
    }

    ex_wb->MAR = of_ex->MAR;
    ex_wb->AC = banco->AC;
}

void escreve_resultado(IAS_REGS *banco, EX_WB *ex_wb, void *memory)
{
    // escrita resultado
    if (ex_wb->enable_wb)
    {
        banco->MAR = ex_wb->MAR;
        banco->MBR = ex_wb->AC;
        barramento(1, memory, banco);
    }
}

void processador(void *memory, int *n_cycles)
{
    // Inicializa o banco de registradores
    IAS_REGS banco;
    banco.PC = 0;
    banco.MAR = 0;
    banco.IR = 0;
    banco.IBR = 0;
    banco.MBR = 0;
    banco.AC = 0;
    banco.MQ = 0;

    // Inicializa os sinais de controle
    control_signals signal;
    signal.left_necessary = 0;
    signal.counter = 0;

    // Inicializa os registradores do pipeline
    pipeline_regs p_rgs;
    p_rgs.if_id = malloc(sizeof(IF_ID));
    p_rgs.id_of = malloc(sizeof(ID_OF));
    p_rgs.of_ex = malloc(sizeof(OF_EX));
    p_rgs.ex_wb = malloc(sizeof(EX_WB));

    p_rgs.if_id->IR = 0;
    p_rgs.if_id->MAR = 0;

    p_rgs.id_of->opcode = 0;
    p_rgs.id_of->enable_of = 0;
    p_rgs.id_of->MAR = 0;

    p_rgs.of_ex->opcode = 0;
    p_rgs.of_ex->MAR = 0;
    p_rgs.of_ex->MBR = 0;

    p_rgs.ex_wb->enable_wb = 0;
    p_rgs.ex_wb->MAR = 0;
    p_rgs.ex_wb->AC = 0;

    // Inicializa o ciclo de clock
    int clock = 0;

    // Inicia a execução do pipeline
}

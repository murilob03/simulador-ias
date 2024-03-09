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

// Estrutura para os sinais de controle
typedef struct
{
    int left_necessary;
    int counter;
    int stall;
    int restart_pipeline;
    int forward;
} control_signals;

// Estrutura para o registrador entre IF e ID
typedef struct
{
    // Fields for IF/ID pipeline register
    int64_t mem_buffer;
} IF_ID;

// Estrutura para o registrador entre ID e OF
typedef struct
{
    int opcode;       // The decoded opcode
    int64_t mem_addr; // The address of the operand
} ID_OF;

// Estrutura para o registrador entre OF e EX
typedef struct
{
    int opcode;         // The decoded opcode
    int64_t mem_addr;   // The address
    int64_t mem_buffer; // The operand
} OF_EX;

// Estrutura para o registrador entre EX e WB
typedef struct
{
    int enable_wb;    // Enable the WB stage
    int64_t mem_addr; // The address to write
    int64_t ac;       // The result of the operation
} EX_WB;

// Estrutura para agrupar os registros do pipeline
typedef struct
{
    IF_ID *if_id;
    ID_OF *id_of;
    OF_EX *of_ex;
    EX_WB *ex_wb;
} pipeline_regs;

void busca_operacao(IAS_REGS *banco, control_signals *signal, void *memory, IF_ID *if_id);
void decodifica(IAS_REGS *banco, IF_ID *if_id, ID_OF *id_of, control_signals *signal);
void busca_operando(IAS_REGS *banco, ID_OF *id_of, OF_EX *of_ex, control_signals *signal, void *memory, int *n_cycles);
void executa_operacao(IAS_REGS *banco, OF_EX *of_ex, EX_WB *ex_wb, control_signals *signal);
void escreve_resultado(IAS_REGS *banco, EX_WB *ex_wb, void *memory);

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
    // detect pipeline stall
    if (signal->stall && estagio != 3)
        return;

    switch (estagio)
    {
    case 0:
        // busca
        busca_operacao(banco, signal, memory, p_rgs.if_id);
        break;
    case 1:
        // decodificação
        decodifica(banco, p_rgs.if_id, p_rgs.id_of, signal);
        break;
    case 2:
        // busca operando

        // detect raw hazard
        if (p_rgs.id_of->mem_addr == p_rgs.ex_wb->mem_addr)
            signal->forward = 1;

        busca_operando(banco, p_rgs.id_of, p_rgs.of_ex, signal, memory, n_cycles);
        break;
    case 3:
        // execução
        executa_operacao(banco, p_rgs.of_ex, p_rgs.ex_wb, signal);
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

void busca_operacao(IAS_REGS *banco, control_signals *signal, void *memory, IF_ID *if_id)
{
    // busca

    if (banco->IBR != 0)
        return;

    banco->MAR = banco->PC;
    barramento(0, memory, banco);

    if_id->mem_buffer = banco->MBR;

    banco->PC++;
}

void decodifica(IAS_REGS *banco, IF_ID *if_id, ID_OF *id_of, control_signals *signal)
{
    // decodificação

    banco->MBR = if_id->mem_buffer;

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

    id_of->mem_addr = banco->MAR;
    id_of->opcode = banco->IR;
}

void busca_operando(IAS_REGS *banco, ID_OF *id_of, OF_EX *of_ex, control_signals *signal, void *memory, int *n_cycles)
{
    // busca operando
    
    banco->MAR = id_of->mem_addr;
    
    barramento(0, memory, banco);

    of_ex->opcode = id_of->opcode;
    of_ex->mem_addr = id_of->mem_addr;
    of_ex->mem_buffer = banco->MBR;

    // set counter
    signal->counter = n_cycles[of_ex->opcode] - 1;
}

void executa_operacao(IAS_REGS *banco, OF_EX *of_ex, EX_WB *ex_wb, control_signals *signal)
{
    // execução

    // se o contador de ciclos for 0, executa a operação
    if (signal->counter == 0)
    {
        banco->IR = of_ex->opcode;
        banco->MBR = of_ex->mem_buffer;
        banco->MAR = of_ex->mem_addr;

        // executa a operação
        switch (banco->IR)
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
            banco->PC = banco->MAR;
            signal->restart_pipeline = 1;
            return;
        case 0b00001110:
            // jump M(X,20:39)
            banco->PC = banco->MAR;
            signal->left_necessary = 0;
            signal->restart_pipeline = 1;
            return;
        case 0b00001111:
            // jump +M(X,0:19)
            if (banco->AC >= 0)
            {
                banco->PC = banco->MAR;
                signal->restart_pipeline = 1;
                return;
            }
            break;
        case 0b00010000:
            // jump +M(X,20:39)
            if (banco->AC >= 0)
            {
                banco->PC = banco->MAR;
                signal->left_necessary = 0;
                signal->restart_pipeline = 1;
                return;
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
            banco->MBR = banco->MBR | (banco->AC << 8);

            ex_wb->mem_addr = of_ex->mem_addr;
            ex_wb->ac = banco->MBR;
            ex_wb->enable_wb = 1;
            return;
        case 0b00010011:
            // stor M(X,28:39)
            banco->MBR = banco->MBR | banco->AC;

            ex_wb->mem_addr = of_ex->mem_addr;
            ex_wb->ac = banco->MBR;
            ex_wb->enable_wb = 1;
            return;
        }

        ex_wb->mem_addr = of_ex->mem_addr;
        ex_wb->ac = banco->AC;
        signal->stall = 0;
    }
    else
    {
        signal->stall = 1;
        signal->counter--;
    }
}

void escreve_resultado(IAS_REGS *banco, EX_WB *ex_wb, void *memory)
{
    // escrita resultado
    if (ex_wb->enable_wb)
    {
        banco->MAR = ex_wb->mem_addr;
        banco->MBR = ex_wb->ac;
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

    p_rgs.if_id->mem_buffer = 0;

    p_rgs.id_of->opcode = 0;
    p_rgs.id_of->mem_addr = 0;

    p_rgs.of_ex->opcode = 0;
    p_rgs.of_ex->mem_addr = 0;
    p_rgs.of_ex->mem_buffer = 0;

    p_rgs.ex_wb->enable_wb = 0;
    p_rgs.ex_wb->mem_addr = 0;
    p_rgs.ex_wb->ac = 0;

    // Inicializa o ciclo de clock
    int clock = 0;

    // Inicia a execução do pipeline
}

void step_pipeline(void *memory, IAS_REGS *banco, control_signals *signal, pipeline_regs *p_rgs, int *n_cycles)
{
    // Unidade de Controle
    UC(banco, signal, *p_rgs, n_cycles, memory, 0);
    UC(banco, signal, *p_rgs, n_cycles, memory, 1);
    UC(banco, signal, *p_rgs, n_cycles, memory, 2);
    UC(banco, signal, *p_rgs, n_cycles, memory, 3);
    UC(banco, signal, *p_rgs, n_cycles, memory, 4);
}

void step_pipeline_reverse(void *memory, IAS_REGS *banco, control_signals *signal, pipeline_regs *p_rgs, int *n_cycles)
{
    // Unidade de Controle
    UC(banco, signal, *p_rgs, n_cycles, memory, 4);
    UC(banco, signal, *p_rgs, n_cycles, memory, 3);
    UC(banco, signal, *p_rgs, n_cycles, memory, 2);
    UC(banco, signal, *p_rgs, n_cycles, memory, 1);
    UC(banco, signal, *p_rgs, n_cycles, memory, 0);
}

void start_pipeline(void *memory, IAS_REGS *banco, control_signals *signal, pipeline_regs *p_rgs, int *n_cycles)
{
    // Zero the pipeline registers
    p_rgs->if_id->mem_buffer = 0;

    p_rgs->id_of->opcode = 0;
    p_rgs->id_of->mem_addr = 0;

    p_rgs->of_ex->opcode = 0;
    p_rgs->of_ex->mem_addr = 0;
    p_rgs->of_ex->mem_buffer = 0;

    p_rgs->ex_wb->enable_wb = 0;
    p_rgs->ex_wb->mem_addr = 0;
    p_rgs->ex_wb->ac = 0;
    
    // Unidade de Controle
    UC(banco, signal, *p_rgs, n_cycles, memory, 0);

    UC(banco, signal, *p_rgs, n_cycles, memory, 1);
    UC(banco, signal, *p_rgs, n_cycles, memory, 0);
    
    UC(banco, signal, *p_rgs, n_cycles, memory, 2);
    UC(banco, signal, *p_rgs, n_cycles, memory, 1);
    UC(banco, signal, *p_rgs, n_cycles, memory, 0);

    UC(banco, signal, *p_rgs, n_cycles, memory, 3);
    UC(banco, signal, *p_rgs, n_cycles, memory, 2);
    UC(banco, signal, *p_rgs, n_cycles, memory, 1);
    UC(banco, signal, *p_rgs, n_cycles, memory, 0);

    signal->restart_pipeline = 0;
}
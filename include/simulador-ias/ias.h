#ifndef IAS_H
#define IAS_H

#include <inttypes.h>

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
    int raw;
    int halt;
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
    int64_t mem_addr; // The address to write
    int64_t result;   // The result of the operation
} EX_WB;

// Estrutura para agrupar os registros do pipeline
typedef struct
{
    IF_ID *if_id;
    ID_OF *id_of;
    OF_EX *of_ex;
    EX_WB *ex_wb;
} pipeline_regs;

/*
 * Função para processar as instruções na memória.
 *
 * pc: endereço de início do programa
 * memory: ponteiro para a memória
 * n_cycles: ponteiro para o número de ciclos para cada operação
 */
void processador(int pc, void *memory, int *n_cycles);

#endif
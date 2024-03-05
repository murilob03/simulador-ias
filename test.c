#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "memory.h"
#include "conversor.c"
#include "ias.c"

int main(int argc, char const *argv[])
{
    void *memory = memory_init;

    // Teste de conversor.c
    char *input_file = "inputs/input.txt";

    write_memory(memory, input_file);

    // Print memory
    int64_t value;
    for (int i = 0; i < 100; i++)
    {
        memory_read(i, &value, memory);
        printf("%d: %" PRId64 "\n", i, value);
    }

    // Teste de ias.c
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

    p_rgs.if_id->MBR = 0;
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

    // Teste de funções
    banco.PC = 11;
    busca(&banco, &signal, memory, p_rgs.if_id);

    printf("PC: %d\n", banco.PC);
    printf("IR: %" PRId64 "\n", p_rgs.if_id->MBR);
}

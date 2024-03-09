#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "../memory.h"
#include "../conversor.c"
#include "../ias.c"

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
    signal.left_necessary = 1;
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

    // Inicializa a quantidade de ciclos por op
    int op_cycles[33];
    op_cycles[1] = 2;  // LOAD M(X)
    op_cycles[2] = 2;  // LOAD -M(X)
    op_cycles[3] = 2;  // LOAD |M(X)|
    op_cycles[4] = 2;  // LOAD -|M(X)|
    op_cycles[5] = 2;  // ADD M(X)
    op_cycles[6] = 2;  // SUB M(X)
    op_cycles[7] = 2;  // ADD |M(X)|
    op_cycles[8] = 2;  // SUB |M(X)|
    op_cycles[9] = 2;  // LOAD MQ,M(X)
    op_cycles[10] = 2; // LOAD MQ
    op_cycles[11] = 2; // MUL M(X)
    op_cycles[12] = 2; // DIV M(X)
    op_cycles[13] = 2; // JUMP M(X,0:19)
    op_cycles[14] = 2; // JUMP M(X,20:39)
    op_cycles[15] = 2; // JUMP +M(X,0:19)
    op_cycles[16] = 2; // JUMP +M(X,20:39)
    op_cycles[18] = 2; // STOR M(X,8:19)
    op_cycles[19] = 2; // STOR M(X,28:39)
    op_cycles[20] = 2; // LSH
    op_cycles[21] = 2; // RSH
    op_cycles[33] = 2; // STOR M(X)

    // Teste de funções

    // busca_operacao
    printf("\n***** Teste de busca_operacao *****\n");

    // ADD M(7) SUB M(5)
    // 0000010100000000011100000110000000000101 (2)
    // 21482201093 (10)
    banco.PC = 10; 
    busca_operacao(&banco, &signal, memory, p_rgs.if_id);

    printf("MBR: %" PRId64 "\n", p_rgs.if_id->mem_buffer); // Expected: 21482201093

    printf("***** Fim do teste de busca_operacao *****\n\n");

    // decodifica_operacao
    printf("***** Teste de decodifica_operacao *****\n");

    decodifica(&banco, p_rgs.if_id, p_rgs.id_of, &signal);

    printf("Opcode: %" PRId64 "\n", p_rgs.id_of->opcode);     // Expected: 5
    printf("Mem_addr: %" PRId64 "\n", p_rgs.id_of->mem_addr); // Expected: 7
    
    printf("***** Fim do teste de decodifica_operacao *****\n\n");

    // busca_operando
    printf("***** Teste de busca_operando *****\n");

    busca_operando(&banco, p_rgs.id_of, p_rgs.of_ex, &signal, memory, op_cycles);

    printf("Opcode: %d\n", p_rgs.of_ex->opcode);     // Expected: 5
    printf("Mem_addr: %" PRId64 "\n", p_rgs.of_ex->mem_addr); // Expected: 7
    printf("Mem_buffer: %" PRId64 "\n", p_rgs.of_ex->mem_buffer); // Expected: 95589247770
    printf("Counter: %d\n", signal.counter); // Expected: 1

    printf("***** Fim do teste de busca_operando *****\n\n");

    // executa_operacao
    printf("***** Teste de executa_operacao *****\n");

    executa_operacao(&banco, p_rgs.of_ex, p_rgs.ex_wb, &signal);

    printf("First Cycle\n");
    printf("Enable_wb: %d\n", p_rgs.ex_wb->enable_wb); // Expected: 0
    printf("Mem_addr: %d\n", p_rgs.ex_wb->mem_addr); // Expected: 0
    printf("AC: %" PRId64 "\n", p_rgs.ex_wb->ac); // Expected: 0
    printf("Counter: %d\n", signal.counter); // Expected: 0

    executa_operacao(&banco, p_rgs.of_ex, p_rgs.ex_wb, &signal);
    
    printf("\nSecond Cycle\n");
    printf("Enable_wb: %d\n", p_rgs.ex_wb->enable_wb); // Expected: 0
    printf("Mem_addr: %d\n", p_rgs.ex_wb->mem_addr); // Expected: 7
    printf("AC: %" PRId64 "\n", p_rgs.ex_wb->ac); // Expected: 95589247770
    printf("Counter: %d\n", signal.counter); // Expected: 0

    printf("***** Fim do teste de executa_operacao *****\n\n");

    // escreve_resultado
    printf("***** Teste de escreve_resultado *****\n");

    escreve_resultado(&banco, p_rgs.ex_wb, memory);

    int64_t value2;
    memory_read(7, &value2, memory);
    printf("Mem[7]: %" PRId64 "\n", value2); // Expected: 95589247770

    printf("***** Fim do teste de escreve_resultado *****\n\n");
}

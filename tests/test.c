#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include "../include/tests/ias_test.h"
#include "../include/simulador-ias/memory.h"
#include "../include/simulador-ias/conversor.h"

void reset_processor(IAS_REGS *banco, control_signals *signal, pipeline_regs *p_rgs)
{
    banco->PC = 0;
    banco->MAR = 0;
    banco->IR = 0;
    banco->IBR = 0;
    banco->MBR = 0;
    banco->AC = 0;
    banco->MQ = 0;

    signal->left_necessary = 1;
    signal->counter = 0;
    signal->stall = 0;
    signal->restart_pipeline = 1;
    signal->raw = 0;

    p_rgs->if_id->mem_buffer = 0;

    p_rgs->id_of->opcode = 0;
    p_rgs->id_of->mem_addr = 0;

    p_rgs->of_ex->opcode = 0;
    p_rgs->of_ex->mem_addr = 0;
    p_rgs->of_ex->mem_buffer = 0;

    p_rgs->ex_wb->mem_addr = -1;
    p_rgs->ex_wb->result = 0;
}

int main(int argc, char const *argv[])
{
    void *memory = memory_init;

    write_memory(memory, "/home/murilob/codes/simulador-ias/tests/input.txt");

    // Print memory
    int64_t value;
    for (int i = 0; i < 50; i++)
    {
        memory_read(i, &value, memory);
        printf("%d: %" PRId64 "\n", i, value);
    }

    // Teste de ias.c

    // Declaração de variáveis
    IAS_REGS banco; // Banco de registradores

    control_signals signal; // Sinais de controle

    // Banco de registradores do pipeline
    pipeline_regs p_rgs;
    p_rgs.if_id = malloc(sizeof(IF_ID));
    p_rgs.id_of = malloc(sizeof(ID_OF));
    p_rgs.of_ex = malloc(sizeof(OF_EX));
    p_rgs.ex_wb = malloc(sizeof(EX_WB));

    // Inicializa as variáveis
    reset_processor(&banco, &signal, &p_rgs);

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

    // Teste de get_op_cycles
    printf("***** Teste de get_op_cycles *****\n");

    get_op_cycles(op_cycles, "/home/murilob/codes/simulador-ias/tests/input.txt");

    // print op_cycles
    printf("\n");
    for (int i = 1; i < 17; i++)
    {
        printf("op_cycles[%d] = %d\n", i, op_cycles[i]);
    }
    for (int i = 18; i < 22; i++)
    {
        printf("op_cycles[%d] = %d\n", i, op_cycles[i]);
    }
    printf("op_cycles[33] = %d\n", op_cycles[33]);

    printf("***** Fim do teste de get_op_cycles *****\n\n");

    // Teste de funções

    // busca_operacao
    printf("\n***** Teste de busca_operacao *****\n");

    // ADD M(10) STOR M(13)
    banco.PC = 18;
    busca_operacao(&banco, &signal, memory, p_rgs.if_id);

    printf("MBR: %" PRId64 "\n", p_rgs.if_id->mem_buffer); // Expected: 21485457421

    printf("***** Fim do teste de busca_operacao *****\n\n");

    // decodifica_operacao
    printf("***** Teste de decodifica_operacao *****\n");

    decodifica(&banco, p_rgs.if_id, p_rgs.id_of, &signal);

    printf("Opcode: %" PRId64 "\n", p_rgs.id_of->opcode);     // Expected: 5
    printf("Mem_addr: %" PRId64 "\n", p_rgs.id_of->mem_addr); // Expected: 10

    printf("***** Fim do teste de decodifica_operacao *****\n\n");

    // busca_operando
    printf("***** Teste de busca_operando *****\n");

    busca_operando(&banco, p_rgs.id_of, p_rgs.of_ex, &signal, memory, op_cycles);

    printf("Opcode: %d\n", p_rgs.of_ex->opcode);                  // Expected: 5
    printf("Mem_addr: %" PRId64 "\n", p_rgs.of_ex->mem_addr);     // Expected: 10
    printf("Mem_buffer: %" PRId64 "\n", p_rgs.of_ex->mem_buffer); // Expected: 1
    printf("Counter: %d\n", signal.counter);                      // Expected: 1

    printf("***** Fim do teste de busca_operando *****\n\n");

    // executa_operacao
    printf("***** Teste de executa_operacao *****\n");

    executa_operacao(&banco, p_rgs.of_ex, p_rgs.ex_wb, &signal);

    printf("First Cycle\n");
    printf("Mem_addr: %d\n", p_rgs.ex_wb->mem_addr);   // Expected: -1
    printf("AC: %" PRId64 "\n", p_rgs.ex_wb->result);  // Expected: 0
    printf("Counter: %d\n", signal.counter);           // Expected: 0
    printf("Stall: %d\n", signal.stall);               // Expected: 1

    executa_operacao(&banco, p_rgs.of_ex, p_rgs.ex_wb, &signal);

    printf("\nSecond Cycle\n");
    printf("Mem_addr: %d\n", p_rgs.ex_wb->mem_addr);   // Expected: -1
    printf("AC: %" PRId64 "\n", p_rgs.ex_wb->result);  // Expected: 1
    printf("Counter: %d\n", signal.counter);           // Expected: 0
    printf("Stall: %d\n", signal.stall);               // Expected: 0

    printf("***** Fim do teste de executa_operacao *****\n\n");

    // escreve_resultado
    printf("***** Teste de escreve_resultado *****\n");

    escreve_resultado(&banco, p_rgs.ex_wb, memory);

    int64_t value2;
    memory_read(10, &value2, memory);
    printf("Mem[10]: %" PRId64 "\n", value2); // Expected: 1

    printf("***** Fim do teste de escreve_resultado *****\n\n");


    // Teste de pipeline
    printf("***** Teste de processamento *****\n");

    // Inicializa as variáveis
    reset_processor(&banco, &signal, &p_rgs);

    // Limpa a memória
    for (int i = 0; i < 100; i++)
    {
        memory_write(i, 0, memory);
    }

    // Carrega a memória com o novo arquivo
    write_memory(memory, "/home/murilob/codes/simulador-ias/tests/input.txt");

    // Print memory
    for (int i = 0; i < 36; i++)
    {
        memory_read(i, &value, memory);
        printf("%d: %" PRId64 "\n", i, value);
    }

    // Teste de start_pipeline
    printf("\n***** Teste de start_pipeline *****\n");

    // LOAD M(12) SUB M(11)
    banco.PC = 14;

    start_pipeline(&banco, &signal, &p_rgs, memory, op_cycles);

    printf("ExOpcode: %d\n", p_rgs.of_ex->opcode); // Expected: 1 (LOAD M(12))
    printf("Mem_buffer: %" PRId64 "\n", p_rgs.of_ex->mem_buffer); // Expected: 0 (valor de M(11))

    printf("OfOpcode: %d\n", p_rgs.id_of->opcode); // Expected: 6 (SUB M(11))
    printf("Mem_addr: %" PRId64 "\n", p_rgs.id_of->mem_addr); // Expected: 11

    printf("***** Fim do teste de start_pipeline *****\n\n");

    // Teste de processador
    printf("***** Teste de processador *****\n");

    processador(14, memory, op_cycles);

    // Print memory
    for (int i = 0; i < 36; i++)
    {
        memory_read(i, &value, memory);
        printf("%d: %" PRId64 "\n", i, value);
    }

    printf("***** Fim do teste de processador *****\n\n");

    printf("***** Fim do teste de processamento *****\n\n");
}

#include <stdlib.h>

#include "../include/simulador-ias/ias.h"
#include "../include/simulador-ias/memory.h"

void busca_operacao(IAS_REGS *banco, control_signals *signal, void *memory, IF_ID *if_id);
void decodifica(IAS_REGS *banco, IF_ID *if_id, ID_OF *id_of, control_signals *signal);
void busca_operando(IAS_REGS *banco, ID_OF *id_of, OF_EX *of_ex, control_signals *signal, void *memory, int *n_cycles);
void executa_operacao(IAS_REGS *banco, OF_EX *of_ex, EX_WB *ex_wb, control_signals *signal);
void escreve_resultado(IAS_REGS *banco, EX_WB *ex_wb, void *memory);
void start_pipeline(IAS_REGS *banco, control_signals *signal, pipeline_regs *p_rgs, void *memory, int *n_cycles);
void step_pipeline_reverse(void *memory, IAS_REGS *banco, control_signals *signal, pipeline_regs *p_rgs, int *n_cycles);
int is_read(int opcode);

/*
 * Função para acesso da memória
 * is_write: 1 para escrita, 0 para leitura
 * memory: ponteiro para a memória
 * banco: banco de registradores
 *
 * A função acessa a memória no endereço armazenado em MAR
 * e armazena o valor em MBR ou lê o valor de MBR e armazena
 * no endereço de memória armazenado em MAR.
 */
void barramento(int is_write, void *memory, IAS_REGS *banco)
{
    if (is_write)
        memory_write(banco->MAR, banco->MBR, memory);
    else
        memory_read(banco->MAR, &banco->MBR, memory);
}

/*
 * Unidade de Controle
 *
 * A função UC é responsável por controlar o fluxo de dados
 * no pipeline. Ela é chamada em cada estágio do pipeline e
 * verifica se o estágio atual deve ser executado ou não.
 *
 * A função também é responsável por detectar hazards e
 * stall no pipeline.
 */
void UC(IAS_REGS *banco, control_signals *signal, pipeline_regs p_rgs, int *n_cycles, void *memory, int estagio)
{
    // detecta pipeline stall
    if (signal->stall && estagio != 3)
        return;

    switch (estagio)
    {
    case 0:
        // busca
        if (signal->halt > 0)
            break;

        busca_operacao(banco, signal, memory, p_rgs.if_id);
        break;
    case 1:
        // decodificação
        if (signal->halt > 0)
            break;

        decodifica(banco, p_rgs.if_id, p_rgs.id_of, signal);

        break;
    case 2:
        // busca operando
        if (signal->halt > 0)
            break;

        busca_operando(banco, p_rgs.id_of, p_rgs.of_ex, signal, memory, n_cycles);
        break;
    case 3:
        // execução
        executa_operacao(banco, p_rgs.of_ex, p_rgs.ex_wb, signal);

        if (signal->stall == 0)
        {
            // detecta hazard RAW (escrita em endereço em uso por uma instrução anterior)
            if (p_rgs.of_ex->opcode == 0b00010010 || p_rgs.of_ex->opcode == 0b00010011)
            {
                // se o endereço já foi lido por instruções subsequentes, reinicia o pipeline
                if (p_rgs.of_ex->mem_addr > banco->PC - 3 && p_rgs.of_ex->mem_addr < banco->PC + 1)
                {
                    // verifica se as instruções subsequentes são esquerda ou direita
                    if (banco->IBR)
                    {
                        signal->left_necessary = 1;
                        banco->PC = banco->PC - 1;
                    }
                    else
                    {
                        signal->left_necessary = 0;
                        banco->PC = banco->PC - 2;
                    }

                    UC(banco, signal, p_rgs, n_cycles, memory, 4); // escreve o resultado
                    signal->restart_pipeline = 1;
                }
            }
        }

        break;
    case 4:
        // escreve resultado
        escreve_resultado(banco, p_rgs.ex_wb, memory);

        // Encaminhamento
        if (p_rgs.ex_wb->mem_addr != 255)
        {
            // se a próxima instrução for de leitura e o endereço de memória for o mesmo
            // da instrução atual, encaminha o valor diretamente para a execução
            if (is_read(p_rgs.of_ex->opcode) && p_rgs.of_ex->mem_addr == p_rgs.ex_wb->mem_addr)
                p_rgs.of_ex->mem_buffer = p_rgs.ex_wb->result;
        }

        break;
    }
}

/*
 * Unidade Lógica e Aritmética (ULA)
 *
 * A função ULA é responsável por executar as operações
 * lógicas e aritméticas. Ela é chamada no estágio de
 * execução do pipeline.
 *
 * @param use_mbr 1 para usar o valor de MBR, 0 para usar o valor de AC
 * @param op operação a ser executada
 * @param banco banco de registradores
 */
void ULA(int use_mbr, int op, IAS_REGS *banco)
{
    switch (op)
    {
    case 0:
        // add
        banco->AC += banco->MBR;
        break;
    case 1:
        // subtração
        banco->AC -= banco->MBR;
        break;
    case 2:
        // multiplicação
        banco->MQ = banco->MQ * banco->MBR;
        break;
    case 3:
        // divisão
        banco->MQ = banco->AC / banco->MBR;
        banco->AC = banco->AC % banco->MBR;
        break;
    case 4:
        // absoluto
        if (use_mbr)
            banco->MBR = abs(banco->MBR);
        else
            banco->AC = abs(banco->AC);
        break;
    case 5:
        // negativo
        if (use_mbr)
            banco->MBR = -banco->MBR;
        else
            banco->AC = -banco->AC;
        break;
    case 6:
        // deslocamento direita
        banco->AC >>= banco->AC;
        break;
    case 7:
        // deslocamento esquerda
        banco->AC <<= banco->AC;
        break;
    default:
        break;
    }
}

/*
 * Busca Instrução.
 *
 * Caso IBR esteja vazio, busca as próximas instruções
 * na memória e armazena em if_id->mem_buffer. Caso
 * contrário, retorna sem fazer nada.
 *
 * @param banco banco de registradores
 * @param signal sinais de controle
 * @param memory ponteiro para a memória
 * @param if_id registrador IF/ID
 */
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

/*
 * Decodificação.
 *
 * Decodifica a instrução armazenada em MBR e armazena
 * o opcode e o endereço de memória em id_of.
 *
 * @param banco banco de registradores
 * @param if_id registrador IF/ID
 * @param id_of registrador ID/OF
 * @param signal sinais de controle
 */
void decodifica(IAS_REGS *banco, IF_ID *if_id, ID_OF *id_of, control_signals *signal)
{
    // decodificação

    banco->MBR = if_id->mem_buffer;

    if (banco->IBR == 0)
    {
        if (signal->left_necessary)
        {
            banco->IBR = banco->MBR;
            banco->IR = (banco->MBR >> 32) & 0xFF;
            banco->MAR = (banco->MBR >> 20) & 0xFFF;
        }
        else
        {
            banco->IR = (banco->MBR >> 12) & 0xFF;
            banco->MAR = banco->MBR & 0xFFF;
            banco->IBR = 0;
            signal->left_necessary = 1;
        }
    }
    else
    {
        banco->IR = banco->IBR >> 12 & 0xFF;
        banco->MAR = banco->IBR & 0xFFF;
        banco->IBR = 0;
    }

    id_of->mem_addr = banco->MAR;
    id_of->opcode = banco->IR;
}

/*
 * Busca Operando.
 *
 * Busca o operando na memória e armazena em of_ex->mem_buffer.
 *
 * @param banco banco de registradores
 * @param id_of registrador ID/OF
 * @param of_ex registrador OF/EX
 * @param signal sinais de controle
 * @param memory ponteiro para a memória
 * @param n_cycles número de ciclos de cada operação
 */
void busca_operando(IAS_REGS *banco, ID_OF *id_of, OF_EX *of_ex, control_signals *signal, void *memory, int *n_cycles)
{
    // busca operando

    banco->MAR = id_of->mem_addr;

    barramento(0, memory, banco);

    of_ex->opcode = id_of->opcode;
    of_ex->mem_addr = id_of->mem_addr;
    of_ex->mem_buffer = banco->MBR;

    // configura o contador
    signal->counter = n_cycles[of_ex->opcode] - 1;
}

/*
 * Execução.
 *
 * Executa a operação armazenada em of_ex->opcode.
 *
 * @param banco banco de registradores
 * @param of_ex registrador OF/EX
 * @param ex_wb registrador EX/WB
 * @param signal sinais de controle
 */
void executa_operacao(IAS_REGS *banco, OF_EX *of_ex, EX_WB *ex_wb, control_signals *signal)
{
    // execução

    // verifica operação de parada
    if (of_ex->opcode == 255)
    {
        signal->halt = 1;
        return;
    }

    // se o contador de ciclos for 0, executa a operação
    if (signal->counter == 0)
    {
        banco->IR = of_ex->opcode;
        banco->MBR = of_ex->mem_buffer;
        banco->MAR = of_ex->mem_addr;

        ex_wb->mem_addr = -1;

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
            ex_wb->mem_addr = of_ex->mem_addr;
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
            int64_t mask = ~((int64_t)(0xFFF) <<

                             20);
            banco->MBR = (banco->MBR & mask);
            banco->MBR = banco->MBR | (banco->AC << 20);

            ex_wb->mem_addr = of_ex->mem_addr;
            ex_wb->result = banco->MBR;

            signal->stall = 0;
            return;
        case 0b00010011:
            // stor M(X,28:39)
            int64_t mask2 = (int64_t)(0xFFFFFFF) << 12;
            banco->MBR = (banco->MBR & mask2);
            banco->MBR = banco->MBR | banco->AC;

            ex_wb->mem_addr = of_ex->mem_addr;
            ex_wb->result = banco->MBR;

            signal->stall = 0;
            return;
        }

        ex_wb->result = banco->AC;
        signal->stall = 0;
    }
    else
    {
        signal->stall = 1;
        signal->counter--;
    }
}

/*
 * Escrita Resultado.
 *
 * Escreve o resultado da operação na memória.
 *
 * @param banco banco de registradores
 * @param ex_wb registrador EX/WB
 * @param memory ponteiro para a memória
 */
void escreve_resultado(IAS_REGS *banco, EX_WB *ex_wb, void *memory)
{
    // escrita resultado
    if (ex_wb->mem_addr != -1)
    {
        banco->MAR = ex_wb->mem_addr;
        banco->MBR = ex_wb->result;
        barramento(1, memory, banco);

        ex_wb->mem_addr = -1;
    }
}

void processador(int pc, void *memory, int *n_cycles)
{
    // Inicializa o banco de registradores
    IAS_REGS banco;
    banco.PC = pc;
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
    signal.stall = 0;
    signal.restart_pipeline = 1;
    signal.raw = 0;

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

    p_rgs.ex_wb->mem_addr = -1;
    p_rgs.ex_wb->result = 0;

    // Inicia a execução do pipeline
    start_pipeline(&banco, &signal, &p_rgs, memory, n_cycles);

    // Loop principal
    while (1)
    {
        // Executa um ciclo do pipeline
        step_pipeline_reverse(memory, &banco, &signal, &p_rgs, n_cycles);

        if (signal.halt)
            break;

        if (signal.restart_pipeline)
            start_pipeline(&banco, &signal, &p_rgs, memory, n_cycles);
    }

    // Libera a memória alocada
    free(p_rgs.if_id);
    free(p_rgs.id_of);
    free(p_rgs.of_ex);
    free(p_rgs.ex_wb);
}

// Executa um ciclo do pipeline em ordem reversa
void step_pipeline_reverse(void *memory, IAS_REGS *banco, control_signals *signal, pipeline_regs *p_rgs, int *n_cycles)
{
    // Unidade de Controle
    UC(banco, signal, *p_rgs, n_cycles, memory, 4);

    UC(banco, signal, *p_rgs, n_cycles, memory, 3);
    if (signal->restart_pipeline)
        return;

    UC(banco, signal, *p_rgs, n_cycles, memory, 2);
    UC(banco, signal, *p_rgs, n_cycles, memory, 1);
    UC(banco, signal, *p_rgs, n_cycles, memory, 0);
}

// Inicia a execução do pipeline
void start_pipeline(IAS_REGS *banco, control_signals *signal, pipeline_regs *p_rgs, void *memory, int *n_cycles)
{
    // Zera os registradores do pipeline
    p_rgs->if_id->mem_buffer = 0;

    p_rgs->id_of->opcode = 0;
    p_rgs->id_of->mem_addr = 0;

    p_rgs->of_ex->opcode = 0;
    p_rgs->of_ex->mem_addr = 0;
    p_rgs->of_ex->mem_buffer = 0;

    p_rgs->ex_wb->mem_addr = -1;
    p_rgs->ex_wb->result = 0;

    signal->stall = 0;

    banco->IBR = 0;

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

// Verifica se a operação é de leitura
int is_read(int opcode)
{
    return opcode == 0b00000001 ||
           opcode == 0b00000010 ||
           opcode == 0b00000011 ||
           opcode == 0b00000100 ||
           opcode == 0b00000101 ||
           opcode == 0b00000110 ||
           opcode == 0b00000111 ||
           opcode == 0b00001000 ||
           opcode == 0b00001001 ||
           opcode == 0b00001011 ||
           opcode == 0b00001100;
}

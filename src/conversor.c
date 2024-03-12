#include <stdio.h>
#include <stdlib.h>
#include <regex.h>
#include <inttypes.h>
#include <string.h>
#include <ctype.h>

#include "../include/simulador-ias/conversor.h"
#include "../include/simulador-ias/memory.h"

#define MAX_LINE_LENGTH 100

struct operation
{
    char *name; // nome
    int opcode; // código da operação
};

// função para extrair o endereço de memória de uma operação
void extract_address(char *line, char *address)
{
    // move o ponteiro para o início do endereço de memória
    while (*line != '(')
    {
        line++;
    }
    line++;

    // copia o endereço de memória para a variável de endereço
    while (*line != ')' && *line != ',')
    {
        *address = *line;
        address++;
        line++;
    }

    // adiciona o terminador nulo ao final da variável de endereço
    *address = '\0';
}

// função para remover endereços de memória das operações
// formato de saída: operação;endereço_de_memória;
// exemplo: STOR M(123) -> STOR M();123;
// exemplo: LOAD MQ,M(1234,8:19) -> LOAD MQ,M(,8:19);1234;
// exemplo: LOAD MQ -> LOAD MQ;
void remove_memory_address(char *line)
{
    regex_t regex;
    regmatch_t match;

    // Compila o padrão de expressão regular para corresponder a endereços de memória (M(X))
    if (regcomp(&regex, "m\\([^)]*\\)", REG_EXTENDED) != 0)
    {
        fprintf(stderr, "Erro ao compilar expressão regular\n");
        exit(EXIT_FAILURE);
    }

    // Usa regexec para encontrar e substituir endereços de memória na linha
    if (regexec(&regex, line, 1, &match, 0) == 0)
    {
        char address[10] = {'\0'};
        extract_address(line, address);

        // Usa memmove para substituir o endereço de memória correspondido por "M()"
        memmove(&line[match.rm_so + 2], &line[match.rm_so + 2 + strlen(address)], strlen(&line[match.rm_so + strlen(address)]) + 1);

        // concatena o endereço de memória ao final da linha separado por um ponto e vírgula
        line[strlen(line) - 1] = '\0';
        strcat(line, ";");
        strcat(line, address);
        strcat(line, ";");
        strcat(line, "\n");
    }
    else
    {
        line[strlen(line) - 1] = '\0';
        strcat(line, ";");
        strcat(line, "\n");
    }

    // Libera a expressão regular compilada
    regfree(&regex);
}

struct operation ops[] = {
    {"load mq", 0b00001010},
    {"load mq,m()", 0b00001001},
    {"stor m()", 0b00100001},
    {"load m()", 0b00000001},
    {"load -m()", 0b00000010},
    {"load |m()|", 0b00000011},
    {"load -|m()|", 0b00000100},
    {"jump m(,0:19)", 0b00001101},
    {"jump m(,20:39)", 0b00001110},
    {"jump +m(,0:19)", 0b00001111},
    {"jump +m(,20:39)", 0b00010000},
    {"add m()", 0b00000101},
    {"add |m()|", 0b00000111},
    {"sub m()", 0b00000110},
    {"sub |m()|", 0b00001000},
    {"mul m()", 0b00001011},
    {"div m()", 0b00001100},
    {"lsh", 0b00010100},
    {"rsh", 0b00010101},
    {"stor m(,8:19)", 0b00010010},
    {"stor m(,28:39)", 0b00010011},
    {"exit", 0b11111111}
};
// função para converter a operação para sua representação decimal
void convert_to_decimal(char *line)
{
    // separa a operação e o endereço de memória
    char *op;
    op = strtok(line, ";");
    if (op == NULL)
    {
        return;
    }

    char *address;
    address = strtok(NULL, ";");
    if (address == NULL)
    {
        address = "0";
    }

    u_int32_t op_numeric = 0;

    // busca a operação no array de ops
    for (int i = 0; i < sizeof(ops) / sizeof(ops[0]); i++)
    {
        if (strcmp(ops[i].name, op) == 0)
        {
            op_numeric = ops[i].opcode;
            break;
        }
    }
    if (op_numeric == 0)
    {
        printf("Operação inválida: %s\n", op);
        exit(EXIT_FAILURE);
    }

    // converte o endereço de memória para um inteiro e concatena-o à operação
    op_numeric <<= 12;
    int address_numeric = 0;
    address_numeric = atoi(address);
    op_numeric |= address_numeric;

    sprintf(line, "%" PRIu32, op_numeric);
}

// função para unir as duas operações em um único inteiro de 64 bits
u_int64_t join_ops(char *line1, char *line2)
{
    // converte as operações para ulong int
    unsigned long int op1 = 0;
    op1 = strtoul(line1, NULL, 10);
    unsigned long int op2 = 0;
    op2 = strtoul(line2, NULL, 10);

    u_int64_t op_final = 0;

    // concatena as duas operações em um único inteiro de 64 bits
    op_final |= op1;
    op_final <<= 20;
    op_final |= op2;

    return op_final;
}

int is_operation(char *line)
{
    if (isdigit((unsigned char)line[0]))
    {
        return 0;
    }
    return 1;
}

void write_memory(void *memory, const char *input_file)
{
    FILE *file;
    char line[MAX_LINE_LENGTH];
    char last_op[MAX_LINE_LENGTH];
    last_op[0] = '\0';
    int memory_address = 0;

    // Abre o arquivo de entrada para leitura
    if ((file = fopen(input_file, "r")) == NULL)
    {
        perror("Erro ao abrir arquivo de entrada");
        exit(EXIT_FAILURE);
    }

    // Lê e processa as linhas do arquivo de entrada

    while (fgets(line, MAX_LINE_LENGTH, file) != NULL)
    {
        // ignora comentários
        if (line[0] == '/' && line[1] == '/')
        {
            continue;
        }

        // ignora comentários de bloco
        if (line[0] == '/' && line[1] == '*')
        {
            while (!(strstr(line, "*/")))
            {
                fgets(line, MAX_LINE_LENGTH, file);
            }
            continue;
        }

        // Se a linha for uma operação, verifica se há uma operação na variável last_op
        if (is_operation(line))
        {
            // Se já houver uma operação na variável last_op, escreve ambas as operações no arquivo de saída
            if (last_op[0] != '\0')
            {
                remove_memory_address(last_op);
                remove_memory_address(line);

                convert_to_decimal(last_op);
                convert_to_decimal(line);

                u_int64_t op_final = join_ops(last_op, line);

                memory_write(memory_address, op_final, memory);
                memory_address++;

                last_op[0] = '\0';
            }
            // Se não houver uma operação na variável last_op, armazena a operação atual em last_op
            else
            {
                strcpy(last_op, line);
            }
        }
        else
        {
            // Se a linha não for uma operação, verifica se há uma operação na variável last_op
            if (last_op[0] != '\0')
            {
                remove_memory_address(last_op);
                convert_to_decimal(last_op);
                u_int64_t op_final = join_ops(last_op, "0");
                memory_write(memory_address, op_final, memory);
                memory_address++;
                last_op[0] = '\0';
            }

            // Converte o valor em linha para um inteiro de 64 bits e escreve no arquivo de saída
            int64_t value = 0;
            value = strtoull(line, NULL, 10);
            memory_write(memory_address, value, memory);
            memory_address++;
        }
    }

    // Se houver uma operação na variável last_op, escreve no arquivo de saída
    if (last_op[0] != '\0')
    {
        remove_memory_address(last_op);
        convert_to_decimal(last_op);
        u_int64_t op_final = join_ops(last_op, "0");
        memory_write(memory_address, op_final, memory);
        memory_address++;
    }

    // Fecha o arquivo
    fclose(file);
}

// função para separar a linha em operação e número
// exemplo: "load: 2\n" -> operation = load, number = 2
void separate_line(char *line, char *operation, int *number)
{
    const char delimiters[] = ":\n";

    char *token = strtok(line, delimiters);
    strcpy(operation, token);

    token = strtok(NULL, delimiters);
    *number = atoi(token);
}

// função para obter o número de ciclos para cada operação do arquivo de entrada
void get_op_cycles(int *cycles, const char *input_file)
{
    FILE *file;
    char line[50];
    char operation[20];
    int number;
    int i = 0;

    // Abre o arquivo de entrada para leitura
    if ((file = fopen(input_file, "r")) == NULL)
    {
        perror("Erro ao abrir arquivo de entrada");
        exit(EXIT_FAILURE);
    }

    // Lê e processa as linhas do arquivo de entrada
    fgets(line, 50, file);
    while (fgets(line, 50, file) && !(strstr(line, "*/")))
    {
        separate_line(line, operation, &number);

        if (strcmp(operation, "load") == 0)
        {
            cycles[1] = number;
        }
        else if (strcmp(operation, "load-m") == 0)
        {
            cycles[2] = number;
        }
        else if (strcmp(operation, "load|m") == 0)
        {
            cycles[3] = number;
        }
        else if (strcmp(operation, "load-|m") == 0)
        {
            cycles[4] = number;
        }
        else if (strcmp(operation, "addm") == 0)
        {
            cycles[5] = number;
        }
        else if (strcmp(operation, "subm") == 0)
        {
            cycles[6] = number;
        }
        else if (strcmp(operation, "add|m") == 0)
        {
            cycles[7] = number;
        }
        else if (strcmp(operation, "sub|m") == 0)
        {
            cycles[8] = number;
        }
        else if (strcmp(operation, "loadmm") == 0)
        {
            cycles[9] = number;
        }
        else if (strcmp(operation, "loadm") == 0)
        {
            cycles[10] = number;
        }
        else if (strcmp(operation, "mulm") == 0)
        {
            cycles[11] = number;
        }
        else if (strcmp(operation, "divm") == 0)
        {
            cycles[12] = number;
        }
        else if (strcmp(operation, "jumpm") == 0)
        {
            cycles[13] = number;
            cycles[14] = number;
        }
        else if (strcmp(operation, "jump+m") == 0)
        {
            cycles[15] = number;
            cycles[16] = number;
        }
        else if (strcmp(operation, "storm") == 0)
        {
            cycles[18] = number;
            cycles[19] = number;
        }
        else if (strcmp(operation, "lsh") == 0)
        {
            cycles[20] = number;
        }
        else if (strcmp(operation, "rsh") == 0)
        {
            cycles[21] = number;
        }
        else if (strcmp(operation, "stor") == 0)
        {
            cycles[33] = number;
        }
    }

    // Fecha o arquivo
    fclose(file);
}

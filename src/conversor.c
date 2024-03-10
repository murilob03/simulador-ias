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
    char *name;
    int opcode;
};

// function to extract the memory address from an operation
void extract_address(char *line, char *address)
{
    // move the pointer to the start of the memory address
    while (*line != '(')
    {
        line++;
    }
    line++;

    // copy the memory address to the address variable
    while (*line != ')' && *line != ',')
    {
        *address = *line;
        address++;
        line++;
    }

    // add null terminator to the end of the address variable
    *address = '\0';
}

// function to remove memory addresses from inside the operations
// output format: operation;memory_address;
// example: STOR M(123) -> STOR M();123;
// example: LOAD MQ,M(1234,8:19) -> LOAD MQ,M(,8:19);1234;
// example: LOAD MQ -> LOAD MQ;
void remove_memory_address(char *line)
{
    regex_t regex;
    regmatch_t match;

    // Compile the regular expression pattern to match memory addresses (M(X))
    if (regcomp(&regex, "M\\([^)]*\\)", REG_EXTENDED) != 0)
    {
        fprintf(stderr, "Error compiling regular expression\n");
        exit(EXIT_FAILURE);
    }

    // Use regexec to find and replace memory addresses in the line
    if (regexec(&regex, line, 1, &match, 0) == 0)
    {
        char address[10] = {'\0'};
        extract_address(line, address);

        // Use memmove to replace the matched memory address with "M()"
        memmove(&line[match.rm_so + 2], &line[match.rm_so + 2 + strlen(address)], strlen(&line[match.rm_so + strlen(address)]) + 1);

        // concatenate the memory address to the end of the line separeted by a semicolon
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

    // Free the compiled regular expression
    regfree(&regex);
}

struct operation ops[] = {
    {"LOAD MQ", 0b00001010},
    {"LOAD MQ,M()", 0b00001001},
    {"STOR M()", 0b00100001},
    {"LOAD M()", 0b00000001},
    {"LOAD -M()", 0b00000010},
    {"LOAD |M()|", 0b00000011},
    {"LOAD -|M()|", 0b00000100},
    {"JUMP M(,0:19)", 0b00001101},
    {"JUMP M(,20:39)", 0b00001110},
    {"JUMP +M(,0:19)", 0b00001111},
    {"JUMP +M(,20:39)", 0b00010000},
    {"ADD M()", 0b00000101},
    {"ADD |M()|", 0b00000111},
    {"SUB M()", 0b00000110},
    {"SUB |M()|", 0b00001000},
    {"MUL M()", 0b00001011},
    {"DIV M()", 0b00001100},
    {"LSH", 0b00010100},
    {"RSH", 0b00010101},
    {"STOR M(,8:19)", 0b00010010},
    {"STOR M(,28:39)", 0b00010011},
    {"HALT", 0b11111111},
};
// function to convert the operation to its decimal representation
void convert_to_decimal(char *line)
{
    // separate the operation and the memory address
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

    // search for the operation in the ops array
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
        printf("Invalid operation: %s\n", op);
        exit(EXIT_FAILURE);
    }

    // convert the memory address to an integer and concatenate it to the operation
    op_numeric <<= 12;
    int address_numeric = 0;
    address_numeric = atoi(address);
    op_numeric |= address_numeric;

    sprintf(line, "%" PRIu32, op_numeric);
}

// function to join the two operations into a single 64-bit integer
u_int64_t join_ops(char *line1, char *line2)
{
    // convert the operations to unsigned long integers
    unsigned long int op1 = 0;
    op1 = strtoul(line1, NULL, 10);
    unsigned long int op2 = 0;
    op2 = strtoul(line2, NULL, 10);

    u_int64_t op_final = 0;

    // concatenate the two operations into a single 64-bit integer
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

    // Open input file for reading
    if ((file = fopen(input_file, "r")) == NULL)
    {
        perror("Error opening input file");
        exit(EXIT_FAILURE);
    }

    // Read and process lines from the input file
    while (fgets(line, MAX_LINE_LENGTH, file) != NULL)
    {
        // ignore comments
        if (line[0] == '/' && line[1] == '/')
        {
            continue;
        }

        // ignore block comments
        if (line[0] == '/' && line[1] == '*')
        {
            while (!(strstr(line, "*/")))
            {
                fgets(line, MAX_LINE_LENGTH, file);
            }
            continue;
        }

        // If the line is an operation, check if there is an operation in the last_op variable
        if (is_operation(line))
        {
            // If there is already an operation in the last_op variable, write both operations to the output file
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
            // If there is no operation in the last_op variable, store the current operation in last_op
            else
            {
                strcpy(last_op, line);
            }
        }
        else
        {
            // If the line is not an operation, check if there is an operation in the last_op variable
            if (last_op[0] != '\0')
            {
                remove_memory_address(last_op);
                convert_to_decimal(last_op);
                u_int64_t op_final = join_ops(last_op, "0");
                memory_write(memory_address, op_final, memory);
                memory_address++;
                last_op[0] = '\0';
            }

            // Convert the value in line for a 64-bit integer and write to the output file
            int64_t value = 0;
            value = strtoull(line, NULL, 10);
            memory_write(memory_address, value, memory);
            memory_address++;
        }
    }

    // If there is an operation in the last_op variable, write it to the output file
    if (last_op[0] != '\0')
    {
        remove_memory_address(last_op);
        convert_to_decimal(last_op);
        u_int64_t op_final = join_ops(last_op, "0");
        memory_write(memory_address, op_final, memory);
        memory_address++;
    }

    // Close file
    fclose(file);
}

// function to separate the line into the operation and the number
// example: "load: 2\n" -> operation = load, number = 2
void separate_line(char *line, char *operation, int *number)
{
    const char delimiters[] = ":\n";

    char *token = strtok(line, delimiters);
    strcpy(operation, token);

    token = strtok(NULL, delimiters);
    *number = atoi(token);
}

// function to get the number of cycles for each operation from the input file
void get_op_cycles(int *cycles, const char *input_file)
{
    FILE *file;
    char line[50];
    char operation[20];
    int number;
    int i = 0;

    // Open input file for reading
    if ((file = fopen(input_file, "r")) == NULL)
    {
        perror("Error opening input file");
        exit(EXIT_FAILURE);
    }

    // Read and process lines from the input file
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

    // Close file
    fclose(file);
}

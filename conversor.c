#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <inttypes.h>

#define MAX_LINE_LENGTH 1000

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

int main()
{
    FILE *input_file, *output_file;
    char line1[MAX_LINE_LENGTH];
    char line2[MAX_LINE_LENGTH];

    // Open input file for reading
    if ((input_file = fopen("input.txt", "r")) == NULL)
    {
        perror("Error opening input file");
        exit(EXIT_FAILURE);
    }

    // Open output file for writing
    if ((output_file = fopen("output.txt", "w")) == NULL)
    {
        perror("Error opening output file");
        fclose(input_file);
        exit(EXIT_FAILURE);
    }

    // Read lines from the input file two by two, remove memory addresses (when necessary), and write to the output file
    int i = 0;
    while (fgets(line1, MAX_LINE_LENGTH, input_file) != NULL)
    {
        fgets(line2, MAX_LINE_LENGTH, input_file);
        if (i > 499)
        {
            remove_memory_address(line1);
            remove_memory_address(line2);

            convert_to_decimal(line1);
            convert_to_decimal(line2);

            u_int64_t op_final = join_ops(line1, line2);

            fprintf(output_file, "%" PRIu64 "\n", op_final);
        }
        else
        {
            fputs(line1, output_file);
            fputs(line2, output_file);
        }
        i += 2;
    }

    // Close files
    fclose(input_file);
    fclose(output_file);

    return 0;
}

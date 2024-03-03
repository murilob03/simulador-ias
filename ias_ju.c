#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#define TAM_MAX_STR 50
#define tamanho 4096
char buffer[TAM_MAX_STR];

int pc = 0;
int ck = 0;
int ibr = 0;
long long int mar = 0;
int ir = 0;
long long int mbr = 0;
int ac = 0;
int mq = 0;
long long int memoria[tamanho];
int tempo[21];
int count = 0;
int op1 = 0;
int op2 = 0;
int op3 = 0;
int op4 = 0;
int flag_ini = 0;
int flag_parar = 0;
int flag_dir = 0;
long long int aux_mudanca = 0;
int imp = 0;
int flag_pula = 0;

void tempo_instrucoes(FILE *arq);
void separar_string_numero(char *entrada, char *string, int *numero);
void valores_memoria(FILE *arq);
void removeEspacos(char *str);
void extrairInformacoes(char *instrucao, char *operacao, int *endereco, int *esq, int *dir);
void processador();
void busca();
void barramento();
void ula(int sig, int lado);
void uc();
void imprimir_memoria(FILE *saida);
char custom_strtoll(const char *str, long long int *result);

int main(int argc, char *argv[])
{
    FILE *arq;
    FILE *saida;
    char *nome_entrada;
    char *end_inicial;
    char *arg_p = "-p";
    char *arg_i = "-i";

    int k = 0; // inicializa memoria com zero.
    for (k = 0; k < 4096; k++)
    {
        memoria[k] = 0;
    }

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], arg_p) == 0 && i + 1 < argc)
        {
            nome_entrada = argv[i + 1];
        }
        else if (strcmp(argv[i], arg_i) == 0 && i + 1 < argc)
        {
            end_inicial = argv[i + 1];
        }
    }
    pc = atoi(end_inicial);
    printf("end_pc: %d", pc);
    saida = fopen("saida.ias.out", "w+");
    arq = fopen(nome_entrada, "r");
    if (arq == NULL)
    {
        printf("ERRO ao abrir o arquivo de entrada\n");
        exit(1);
    }
    rewind(arq);
    fgets(buffer, TAM_MAX_STR, arq);
    size_t inputLength = strlen(buffer); // tirar o \n
    if (buffer[inputLength - 1] == '\n')
    {
        buffer[inputLength - 1] = '\0';
    }
    if (strcmp(buffer, "/*") == 0)
    {
        tempo_instrucoes(arq);
    }
    for (int i = 0; i < 21; i++)
    {
        printf("\n%i", tempo[i]);
    }
    valores_memoria(arq);
    fclose(arq);
    printf("pc:%d\n", pc);
    processador();
    imprimir_memoria(saida);
}

void tempo_instrucoes(FILE *arq)
{
    char string[50];
    int numero;

    for (int i = 0; i < 21; i++)
    { // preenche o vetor com zeros inicialmente
        tempo[i] = 0;
    }

    while ((fgets(buffer, TAM_MAX_STR, arq) != NULL) && (strcmp(buffer, "*/\n") != 0))
    {
        size_t inputLength = strlen(buffer); // tirar o \n
        if (buffer[inputLength - 1] == '\n')
        {
            buffer[inputLength - 1] = '\0';
        }
        separar_string_numero(buffer, string, &numero);
        printf("\nbuffer atual: %s", buffer);
        if (strcmp(string, "loadm") == 0)
        {
            tempo[0] = numero;
        }
        else if (strcmp(string, "loadmm") == 0)
        {
            tempo[1] = numero;
        }
        else if (strcmp(string, "stor") == 0)
        {
            tempo[2] = numero;
        }
        else if (strcmp(string, "load") == 0)
        {
            tempo[3] = numero;
        }
        else if (strcmp(string, "load-m") == 0)
        {
            tempo[4] = numero;
        }
        else if (strcmp(string, "load|m") == 0)
        {
            tempo[5] = numero;
        }
        else if (strcmp(string, "load-|m") == 0)
        {
            tempo[6] = numero;
        }
        else if (strcmp(string, "jumpm") == 0)
        {
            tempo[7] = numero;
            tempo[8] = numero;
        }
        else if (strcmp(string, "jump+") == 0)
        {
            tempo[9] = numero;
            tempo[10] = numero;
        }
        else if (strcmp(string, "addm") == 0)
        {
            tempo[11] = numero;
        }
        else if (strcmp(string, "add|m") == 0)
        {
            tempo[12] = numero;
        }
        else if (strcmp(string, "subm") == 0)
        {
            tempo[13] = numero;
        }
        else if (strcmp(string, "sub|m") == 0)
        {
            tempo[14] = numero;
        }
        else if (strcmp(string, "mulm") == 0)
        {
            tempo[15] = numero;
        }
        else if (strcmp(string, "divm") == 0)
        {
            tempo[16] = numero;
        }
        else if (strcmp(string, "lsh") == 0)
        {
            tempo[17] = numero;
        }
        else if (strcmp(string, "rsh") == 0)
        {
            tempo[18] = numero;
        }
        else if (strcmp(string, "storm") == 0)
        {
            tempo[19] = numero;
            tempo[20] = numero;
        }
    }
    printf("buffer ultimo: %s", buffer);
}

void separar_string_numero(char *entrada, char *string, int *numero)
{
    // Usa sscanf para separar a string do número
    sscanf(entrada, "%[^:]: %d", string, numero);
}

void valores_memoria(FILE *arq)
{
    int i = 0;
    int m = 0;
    char operacao[20];
    int dir = 0;
    int esq = 0;
    int endereco = 0;
    int instrucao1 = 0;
    int instrucao2 = 0;
    long long int instrucao = 0;
    int op = 0;
    while (i < tamanho && fgets(buffer, TAM_MAX_STR, arq) != NULL)
    {
        removeEspacos(buffer);
        size_t inputLength = strlen(buffer); // tirar o \n
        if (buffer[inputLength - 1] == '\n')
        {
            buffer[inputLength - 1] = '\0';
        }
        printf("\nbuffer atual:%s\n", buffer);
        long long int value;
        char status = custom_strtoll(buffer, &value);

        // Se a linha não é um número, para de ler
        if (status == 'E')
        {
            printf("entrou aqui no break\n");
            break;
        }
        memoria[i] = value;
        i++;
    }
    int k = i;
    printf("Começo instrução");
    do
    {
        size_t inputLength = strlen(buffer); // tirar o \n
        if (buffer[inputLength - 1] == '\n')
        {
            buffer[inputLength - 1] = '\0';
        }
        for (int j = 0; j < strlen(buffer); ++j) // letras minusculas
        {
            buffer[j] = tolower(buffer[j]);
        }
        removeEspacos(buffer);
        printf("buffer:%s", buffer);
        extrairInformacoes(buffer, operacao, &endereco, &esq, &dir);
        printf("\nInstrucao: %s", operacao);
        printf("\nEndereco: %i\n", endereco);
        if (strcmp(buffer, "loadmq") == 0)
        {
            printf("caso 1\n");
            op = 10;
            endereco = 0;
        }
        else if (strcmp(operacao, "loadmq,m(") == 0)
        {
            printf("caso 2\n");
            op = 9;
        }

        else if ((strcmp(operacao, "storm(") == 0 && (esq != 8) && (esq != 28)))
        {
            printf("caso 3\n");
            op = 33;
        }
        else if (strcmp(operacao, "load-m(") == 0)
        {
            printf("caso 4\n");
            op = 2;
        }
        else if (strcmp(operacao, "load-|m(") == 0)
        {
            printf("caso 5\n");
            op = 4;
        }
        else if (strcmp(operacao, "loadm(") == 0)
        {
            printf("caso 6\n");
            op = 1;
        }
        else if (strcmp(operacao, "load|m(") == 0)
        {
            printf("caso 7\n");
            op = 3;
        }
        else if ((strcmp(operacao, "jump+m(") == 0 && (esq == 0)))
        {
            printf("caso 8\n");
            op = 15;
        }
        else if ((strcmp(operacao, "jump+m(") == 0 && (esq == 20)))
        {
            printf("caso 9\n");
            op = 16;
            esq = 0, dir = 0;
        }
        else if ((strcmp(operacao, "jumpm(") == 0 && (esq == 0)))
        {
            printf("caso 10\n");
            op = 13;
            esq = 0, dir = 0;
        }
        else if ((strcmp(operacao, "jumpm(") == 0 && (esq == 20)))
        {
            printf("caso 11\n");
            op = 14;
            esq = 0, dir = 0;
        }
        else if (strcmp(operacao, "addm(") == 0)
        {
            printf("caso 12\n");
            op = 5;
        }
        else if (strcmp(operacao, "add|m(") == 0)
        {
            printf("caso 13\n");
            op = 7;
        }
        else if (strcmp(operacao, "subm(") == 0)
        {
            printf("caso 14\n");
            op = 6;
        }
        else if (strcmp(operacao, "sub|m(") == 0)
        {
            printf("caso 15\n");
            op = 8;
        }
        else if (strcmp(operacao, "mulm(") == 0)
        {
            printf("caso 16\n");
            op = 11;
        }
        else if (strcmp(operacao, "divm(") == 0)
        {
            printf("caso 17\n");
            op = 12;
        }
        else if (strcmp(buffer, "lsh") == 0)
        {
            printf("caso 18\n");
            op = 20;
            endereco = 0;
        }
        else if (strcmp(buffer, "rsh") == 0)
        {
            printf("caso 19\n");
            op = 21;
            endereco = 0;
        }
        else if ((((strcmp(operacao, "storm(")) == 0) && (esq == 8)))
        {
            printf("caso 20\n");
            op = 18;
            esq = 0, dir = 0;
        }
        else if ((((strcmp(operacao, "storm(")) == 0) && (esq == 28)))
        {
            printf("caso 21\n");
            op = 19;
            esq = 0, dir = 0;
        }
        else if (strcmp(buffer, "exit") == 0)
        {
            printf("caso 22\n");
            op = 255;
            endereco = 0;
            fseek(arq, 0, SEEK_END);
        }
        if (m % 2 == 0)
        {
            instrucao1 = op << 12;
            instrucao1 = instrucao1 | endereco;
            printf("inst1: %d\n", instrucao1);
            instrucao = instrucao1;
            instrucao = instrucao << 20;
            memoria[i] = instrucao;
        }
        else if ((m % 2 != 0))
        {
            instrucao2 = op << 12;
            instrucao2 = instrucao2 | endereco;
            printf("inst2:%d\n", instrucao2);
            instrucao = instrucao | instrucao2;
            printf("instfinal:%llu\n", instrucao);
            memoria[i] = instrucao;
            i++;
        }
        m++;
    } while (fgets(buffer, TAM_MAX_STR, arq));

    for (int j = 0; j < i; j++)
    {
        printf("vetor[%d] = %llu\n", j, memoria[j]);
    }
}

void removeEspacos(char *str)
{
    char *i = str;
    char *j = str;

    while (*j != 0)
    {
        *i = *j++;
        if (!isspace(*i))
            i++;
    }
    *i = 0;
}

void extrairInformacoes(char *instrucao, char *operacao, int *endereco, int *esq, int *dir)
{
    // Tentar extrair o comando, o valor e a hora:minuto
    if (sscanf(instrucao, "%29[^0-9]%d,%d:%d", operacao, endereco, esq, dir) == 4)
    {
    }
    else if (sscanf(instrucao, "%19[^0-9]%d", operacao, endereco) == 2)
    {
    }
}

void processador()
{
    uc();
}

// busca, decodificação, busca dos operandos, execução e escrita dos resultados.
void escrita_resultados()
{
    printf("escr_Resul\n");
    if ((ir == 10) || (ir == 1) || (ir == 2) || (ir == 3) || (ir == 4) || (ir == 5) || (ir == 7) || (ir == 6) || (ir == 8) || (ir == 20) || (ir == 21))
    {
        ac = op2;
    }
    else if (ir == 9)
    {
        mq = op2;
    }
    else if (ir == 33)
    {
        memoria[mbr] = op2;
    }
    else if (ir == 13) // duvida jump incodicional inst 1
    {
        pc = op2;
        flag_parar = 0;
        if (count % 2 != 0) // para decodificação ocorrer certo.
        {
            count++;
        }
    }
    else if (ir == 14) // duvida jump incodicional inst 2
    {
        pc = op2;
        flag_parar = 0;
        flag_dir = 1;
        if (count % 2 != 0)
        {
            count++;
        }
    }
    else if (ir == 15) // duvida jump condicional inst1
    {
        pc = op2;
        flag_parar = 0;
        if ((count % 2 != 0) && (flag_pula == 1))
        {
            count++;
            flag_pula = 0;
        }
    }
    else if (ir == 16) // duvida jump condicional inst2
    {
        if (op2 != pc)
        {
            flag_dir = 1;
        }
        pc = op2;
        flag_parar = 0;
        if ((count % 2 != 0) && (flag_pula == 1))
        {
            count++;
        }
    }
    else if (ir == 12)
    {
        ac = op3;
        mq = op4;
    }
    else if ((ir == 18) || (ir == 19)) // duvida mudança end
    {
        memoria[op1] = aux_mudanca;
    }
    else if (ir == 11)
    {
        ac = op2;
        mq = op2;
    }
}

void execucao()
{
    printf("Entrou exe\n");
    int m = 1;
    while (m <= ck)
    {
        printf("Entrou aqui na execucao");
        if (ir == 10)
        {
            op1 = mq;
            ula(1, 0);
        }
        else if (ir == 9)
        {
            op1 = mbr;
            ula(1, 0);
        }
        else if (ir == 33)
        {
            printf("IR = 33\n");
            op1 = ac;
            ula(1, 0);
        }
        else if (ir == 1)
        {
            op1 = mbr;
            ula(1, 0);
        }
        else if (ir == 2)
        {
            printf("IR = 2\n");
            op1 = -mbr;
            ula(1, 0);
        }
        else if (ir == 3)
        {
            printf("Entrou no ir = 3\n");
            op1 = llabs(mbr);
            ula(1, 0);
        }
        else if (ir == 4)
        {
            op1 = -llabs(mbr);
            ula(1, 0);
        }
        else if (ir == 13) // duvida jump incodicional
        {
            op1 = mbr;
            ula(1, 0);
        }
        else if (ir == 14) // duvida jump incodicional
        {
            op1 = mbr;
            ula(1, 0);
        }
        else if (ir == 15) // duvida jump condicional
        {
            op1 = mbr;
            ula(2, 0);
        }
        else if (ir == 16) // duvida jump condicional
        {
            op1 = mbr;
            ula(2, 0);
        }
        else if (ir == 5)
        {
            op1 = mbr;
            op2 = ac;
            ula(3, 0);
        }
        else if (ir == 7)
        {
            op1 = llabs(mbr);
            op2 = ac;
            ula(3, 0);
        }
        else if (ir == 6)
        {
            op1 = mbr;
            op2 = ac;
            ula(4, 0);
        }
        else if (ir == 8)
        {
            op1 = llabs(mbr);
            op2 = ac;
            ula(4, 0);
        }
        else if (ir == 11)
        {
            op1 = mbr;
            op2 = mq;
            ula(5, 0);
        }
        else if (ir == 12)
        {
            op1 = ac;
            op2 = mbr;
            ula(6, 0);
        }
        else if (ir == 20)
        {
            op1 = ac;
            ula(7, 0);
        }
        else if (ir == 21)
        {
            op1 = ac;
            ula(8, 0);
        }
        else if (ir == 18) // duvida mudança end
        {
            op1 = mbr;
            ula(9, 1);
        }
        else if (ir == 19) // duvida mudança end
        {
            op1 = mbr;
            ula(9, 2);
        }
        m++;
    }
}
void busca_operandos()
{
    printf("Entrou busc_op\n");
    if ((ir == 10) || (ir == 9) || (ir == 1) || (ir == 2) || (ir == 3) || (ir == 4) || (ir == 5) || (ir == 7) || (ir == 6) || (ir == 8) || (ir == 11) || (ir == 12) || (ir == 20) || (ir == 21))
    {
        mbr = memoria[mar];
    }
    else if ((ir == 13) || (ir == 14) || (ir == 15) || (ir == 16) || (ir == 14) || (ir == 18) || (ir == 19) || (ir == 33))
    {
        mbr = mar;
    }
}
void decodificacao()
{
    printf("Entrou deco\n");
    long long int aux;
    long long int masc1 = 0xFFFFF00000; // hexadecimal
    long long int masc2 = 1048575;
    int op = 0;
    int end = 0;
    int masc_op = 1044480;
    int masc_end = 4095;
    printf("count:%d\n", count);
    if ((count % 2 == 0) && (flag_ini != 0)) // se for par quer dizer que é necessário buscar o par de instrucoes na memoria
    {
        printf("Entrou deco1\n");
        long long int instrucao_1 = 0;
        long long int instrucao_2 = 0;
        printf("memoria:%llu\n", memoria[pc]);
        printf("valor mbr:%llu\n", mbr);
        aux = mbr;
        printf("aux:%llu\n", aux);
        instrucao_1 = aux;
        instrucao_1 = instrucao_1 & masc1;
        instrucao_1 = instrucao_1 >> 20;
        printf("inst1_depois: %llu\n", instrucao_1);
        instrucao_2 = aux;
        instrucao_2 = instrucao_2 & masc2;
        printf("inst2_depois:%llu\n", instrucao_2);
        if (flag_dir == 0)
        {
            ibr = instrucao_2;
            op = instrucao_1;
            op = op & masc_op;
            op = op >> 12;
            printf("opcode:%d\n", op);
            end = instrucao_1;
            end = end & masc_end;
            printf("end:%d\n", end);
        }
        else
        {
            op = instrucao_2;
            op = op & masc_op;
            op = op >> 12;
            printf("opcode:%d\n", op);
            end = instrucao_2;
            end = end & masc_end;
            printf("end:%d\n", end);
            flag_dir = 0;
            count++; // para que n entre na parte do ibr
        }
        ir = op;
        mar = end;
        count++;
        pc = pc + 1;
    }
    else if ((count % 2 != 0) && (ir != 0) && (flag_dir == 0))
    { // se for impar, quer dizer que já tem uma instrucao em ibr
        op = ibr;
        op = ibr & masc_op;
        op = op >> 12;
        printf("opcode:%d\n", op);
        end = ibr;
        end = end & masc_end;
        printf("end:%d\n", end);
        ir = op;
        mar = end;
        count++;
    }
    if ((ir == 13) || (ir == 14) || (ir == 15) || (ir == 16))
    {
        flag_parar = 1;
    }
    if (ir == 10)
    {
        ck = tempo[0];
    }
    else if (ir == 9)
    {
        ck = tempo[1];
    }
    else if (ir == 33)
    {
        ck = tempo[2];
    }
    else if (ir == 1)
    {
        ck = tempo[3];
    }
    else if (ir == 2)
    {
        ck = tempo[4];
    }
    else if (ir == 3)
    {
        ck = tempo[5];
    }
    else if (ir == 4)
    {
        ck = tempo[6];
    }
    else if (ir == 13)
    {
        ck = tempo[7];
    }
    else if (ir == 14)
    {
        ck = tempo[8];
    }
    else if (ir == 15)
    {
        ck = tempo[9];
    }
    else if (ir == 16)
    {
        ck = tempo[10];
    }
    else if (ir == 5)
    {
        ck = tempo[11];
    }
    else if (ir == 7)
    {
        ck = tempo[12];
    }
    else if (ir == 6)
    {
        ck = tempo[13];
    }
    else if (ir == 8)
    {
        ck = tempo[14];
    }
    else if (ir == 11)
    {
        ck = tempo[15];
    }
    else if (ir == 12)
    {
        ck = tempo[16];
    }
    else if (ir == 20)
    {
        ck = tempo[17];
    }
    else if (ir == 21)
    {
        ck = tempo[18];
    }
    else if (ir == 18)
    {
        ck = tempo[19];
    }
    else if (ir == 19)
    {
        ck = tempo[20];
    }
}

void busca()
{
    flag_ini = 1;
    if ((count % 2 == 0) && (flag_parar == 0))
    {
        printf("Entrou busca\n");
        mar = pc;
        barramento();
    }
}

void barramento()
{
    printf("Entrou barramento\n");
    long long int bar_dados = 0;
    long long int bar_end = 0;
    bar_end = mar;
    bar_dados = memoria[mar];
    mbr = bar_dados;
    printf("mbr_barramento: %llu\n", mbr);
}
void ula(int sig, int lado)
{
    if (sig == 1) // transferencia de dados e salto incondicional
    {
        op2 = op1;
    }
    else if (sig == 2)
    { // salto codicional
        if (ac >= 0)
        {
            op2 = op1;
            flag_pula = 1;
        }
        else
        {
            op2 = pc;
        }
    }
    else if (sig == 3)
    { // aritmeticas
        op2 = op2 + op1;
    }
    else if (sig == 4)
    {
        op2 = op2 - op1;
    }
    else if (sig == 5) // mul : DUVIDA, como assim colocar bit menos significativo em MQ?
    {
        op2 = op2 * op1;
    }
    else if (sig == 6)
    { // div
        op4 = 0;
        op3 = op1;
        while (op3 >= op2)
        {
            op3 -= op2;
            op4++;
        }
    }
    else if (sig == 7)
    { // lsh
        op2 = op1 * 2;
    }
    else if (sig == 8)
    { // rsh
        op2 = op1 / 2;
        op3 = op1 % 2;
    }
    else if (sig == 9)
    { // mudança de endereco esq
        long long int aux = 0;
        long long int instrucao_1 = 0;
        long long int instrucao_2 = 0;
        long long int masc1 = 0xFFFFF00000; // hexadecimal
        long long int masc2 = 1048575;
        int op = 0;
        int end = 0;
        int masc_op = 1044480;
        int masc_end = 4095;
        aux = memoria[op1];
        instrucao_1 = aux;
        instrucao_1 = instrucao_1 & masc1;
        instrucao_1 = instrucao_1 >> 20;
        printf("inst1_depois: %llu\n", instrucao_1);
        instrucao_2 = aux;
        instrucao_2 = instrucao_2 & masc2;
        printf("inst2_depois:%llu\n", instrucao_2);
        if (lado == 1)
        { // instrucao esq
            op = instrucao_1;
            op = op & masc_op;
            op = op >> 12;
            printf("opcode:%d\n", op);
            end = ac;
            printf("end:%d\n", end);
            instrucao_1 = op << 12;
            instrucao_1 = instrucao_1 | end;
            printf("inst1: %d\n", instrucao_1);
            printf("inst2:%d\n", instrucao_2);
        }
        else if (lado == 2)
        { // instrucao dir
            op = instrucao_2;
            op = op & masc_op;
            op = op >> 12;
            printf("opcode:%d\n", op);
            end = ac;
            printf("end:%d\n", end);
            instrucao_2 = op << 12;
            instrucao_2 = instrucao_2 | end;
            printf("inst1: %d\n", instrucao_1);
            printf("inst2:%d\n", instrucao_2);
        }

        aux_mudanca = instrucao_1;
        aux_mudanca = aux_mudanca << 20;
        aux_mudanca = aux_mudanca | instrucao_2;
        printf("instfinal:%llu\n", aux_mudanca);
    }
}
void uc()
{
    while (ir != 255)
    {
        printf("pc_while:%d\n", pc);
        // escrita_resultados();
        // execucao();
        // busca_operandos();
        // decodificacao();
        busca();
        decodificacao();
        busca_operandos();
        execucao();
        escrita_resultados();
    }
}

void imprimir_memoria(FILE *saida)
{
    for (int i = 0; i < 70; i++)
    {
        if (memoria[i] < 0)
        {
            fprintf(saida, "%lld\n", memoria[i]);
            printf("Memoria[%i]:-%lld\n", i, -memoria[i]); // Imprime o número negativo com o sinal "-"
        }
        else
        {
            fprintf(saida, "%lld\n", memoria[i]);
            printf("Memoria[%i]:%lld\n", i, memoria[i]); // Imprime o número positivo normalmente
        }
    }
}

char custom_strtoll(const char *str, long long int *result)
{
    *result = 0;
    int is_negative = 0; // 0 para falso, 1 para verdadeiro

    if (*str == '-')
    {
        is_negative = 1;
        str++;
    }

    while (*str != '\0')
    {
        if (*str < '0' || *str > '9')
        {
            // Se encontrar um caractere não numérico, retorna 'E' para indicar um erro
            return 'E';
        }
        *result = *result * 10 + (*str - '0');
        str++;
    }

    if (is_negative)
    {
        *result = -*result;
    }

    return 'S'; // Retorna 'S' para indicar sucesso
}
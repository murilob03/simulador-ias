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
int ck2 = 0;
int ck3 = 0;
int ibr = 0;
long long int mar = 0;
long long int mar2 = 0;
int ir = 0;
int ir2 = 0;
int ir3 = 0;
int ir4 = 0;
long long int mbr = 0;
long long int mbr2 = 0;
long long int mbr3 = 0;
long long int mbr4 = 0;
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
int flag_pular = 0;
int flag_pular2 = 0;
int flag_continua = 0;
int flag_parar2 = 0; // storm

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

    for (int i = 1; i < argc; i++) // parametros de entrada
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
    pc = atoi(end_inicial); // transforma o valor de pc em número
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
    if (strcmp(buffer, "/*") == 0) // inicio para ler o tempo que cada instrucao permanece na execucao
    {
        tempo_instrucoes(arq); // coloca o tempo de cada instrução em um vetor
    }
    valores_memoria(arq); // coloca os valores na memoria
    fclose(arq);          // já utilizou o arq, agora realizara as operação a partir da memória
    processador();
    imprimir_memoria(saida);
    fclose(saida);
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
        long long int value;
        char status = custom_strtoll(buffer, &value);

        // Se a linha não é um número, para de ler
        if (status == 'E')
        {
            break;
        }
        memoria[i] = value;
        i++;
    }
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
        extrairInformacoes(buffer, operacao, &endereco, &esq, &dir);
        if (strcmp(buffer, "loadmq") == 0)
        {
            op = 10;
            endereco = 0;
        }
        else if (strcmp(operacao, "loadmq,m(") == 0)
        {
            op = 9;
        }

        else if ((strcmp(operacao, "storm(") == 0 && (esq != 8) && (esq != 28)))
        {
            op = 33;
        }
        else if (strcmp(operacao, "load-m(") == 0)
        {
            op = 2;
        }
        else if (strcmp(operacao, "load-|m(") == 0)
        {
            op = 4;
        }
        else if (strcmp(operacao, "loadm(") == 0)
        {
            op = 1;
        }
        else if (strcmp(operacao, "load|m(") == 0)
        {
            op = 3;
        }
        else if ((strcmp(operacao, "jump+m(") == 0 && (esq == 0)))
        {
            op = 15;
        }
        else if ((strcmp(operacao, "jump+m(") == 0 && (esq == 20)))
        {
            op = 16;
            esq = 0, dir = 0;
        }
        else if ((strcmp(operacao, "jumpm(") == 0 && (esq == 0)))
        {
            op = 13;
            esq = 0, dir = 0;
        }
        else if ((strcmp(operacao, "jumpm(") == 0 && (esq == 20)))
        {
            op = 14;
            esq = 0, dir = 0;
        }
        else if (strcmp(operacao, "addm(") == 0)
        {
            op = 5;
        }
        else if (strcmp(operacao, "add|m(") == 0)
        {
            op = 7;
        }
        else if (strcmp(operacao, "subm(") == 0)
        {
            op = 6;
        }
        else if (strcmp(operacao, "sub|m(") == 0)
        {
            op = 8;
        }
        else if (strcmp(operacao, "mulm(") == 0)
        {
            op = 11;
        }
        else if (strcmp(operacao, "divm(") == 0)
        {
            op = 12;
        }
        else if (strcmp(buffer, "lsh") == 0)
        {
            op = 20;
            endereco = 0;
        }
        else if (strcmp(buffer, "rsh") == 0)
        {
            op = 21;
            endereco = 0;
        }
        else if ((((strcmp(operacao, "storm(")) == 0) && (esq == 8)))
        {
            op = 18;
            esq = 0, dir = 0;
        }
        else if ((((strcmp(operacao, "storm(")) == 0) && (esq == 28)))
        {
            op = 19;
            esq = 0, dir = 0;
        }
        else if (strcmp(buffer, "exit") == 0)
        {
            op = 255;
            endereco = 0;
            fseek(arq, 0, SEEK_END);
        }
        if (m % 2 == 0)
        {
            instrucao1 = op << 12;
            instrucao1 = instrucao1 | endereco;
            instrucao = instrucao1;
            instrucao = instrucao << 20;
            memoria[i] = instrucao;
        }
        else if ((m % 2 != 0))
        {
            instrucao2 = op << 12;
            instrucao2 = instrucao2 | endereco;
            instrucao = instrucao | instrucao2;
            memoria[i] = instrucao;
            i++;
        }
        m++;
    } while (fgets(buffer, TAM_MAX_STR, arq));
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
    ir4 = ir3;
    mbr4 = mbr3;

    if ((ir4 == 10) || (ir4 == 1) || (ir4 == 2) || (ir4 == 3) || (ir4 == 4) || (ir4 == 5) || (ir4 == 7) || (ir4 == 6) || (ir4 == 8) || (ir4 == 20) || (ir4 == 21))
    {
        ac = op2;
    }
    else if (ir4 == 9)
    {
        mq = op2;
    }
    else if (ir4 == 33)
    {
        memoria[mbr4] = op2;
    }
    else if (ir4 == 13)
    {
        pc = op2;
        flag_parar = 0;
        if (count % 2 != 0)
        {
            count++;
        }
    }
    else if (ir4 == 14)
    {
        pc = op2;
        flag_parar = 0;
        flag_dir = 1;
        if (count % 2 != 0)
        {
            count++;
        }
    }
    else if (ir4 == 15)
    {
        pc = op2;
        flag_parar = 0;

        if (flag_pular == 1)
        {
            if (count % 2 != 0)
            {
                count++;
            }
            flag_pular = 0;
        }
    }
    else if (ir4 == 16)
    {
        if (op2 != pc)
        {
            flag_dir = 1;
        }
        if (flag_pular == 1)
        {
            if (count % 2 != 0)
            {
                count++;
            }
            flag_pular = 0;
        }
        pc = op2;
        flag_parar = 0;
    }
    else if (ir4 == 12)
    {
        ac = op3;
        mq = op4;
    }
    else if ((ir4 == 18) || (ir4 == 19))
    {
        memoria[op1] = aux_mudanca;
        flag_parar2 = 0;
    }
    else if (ir4 == 11)
    {
        ac = op2;
        mq = op2;
    }
}

void execucao()
{
    ck3 = ck2;
    ir3 = ir2;
    mbr3 = mbr2;

    int m = 1;
    while (m <= ck3)
    {

        if (ir3 == 10)
        {
            op1 = mq;
            ula(1, 0);
        }
        else if (ir3 == 9)
        {
            op1 = mbr3;
            ula(1, 0);
        }
        else if (ir3 == 33)
        {

            op1 = ac;
            ula(1, 0);
        }
        else if (ir3 == 1)
        {
            op1 = mbr3;
            ula(1, 0);
        }
        else if (ir3 == 2)
        {

            op1 = -mbr3;
            ula(1, 0);
        }
        else if (ir3 == 3)
        {

            op1 = llabs(mbr3);
            ula(1, 0);
        }
        else if (ir3 == 4)
        {
            op1 = -llabs(mbr3);
            ula(1, 0);
        }
        else if (ir3 == 13)
        {
            op1 = mbr3;
            ula(10, 0);
        }
        else if (ir3 == 14)
        {
            op1 = mbr3;
            ula(10, 0);
        }
        else if (ir3 == 15)
        {
            op1 = mbr3;
            ula(2, 0);
        }
        else if (ir3 == 16)
        {
            op1 = mbr3;
            ula(2, 0);
        }
        else if (ir3 == 5)
        {
            op1 = mbr3;
            op2 = ac;
            ula(3, 0);
        }
        else if (ir3 == 7)
        {
            op1 = llabs(mbr3);
            op2 = ac;
            ula(3, 0);
        }
        else if (ir3 == 6)
        {
            op1 = mbr3;
            op2 = ac;
            ula(4, 0);
        }
        else if (ir3 == 8)
        {
            op1 = llabs(mbr);
            op2 = ac;
            ula(4, 0);
        }
        else if (ir3 == 11)
        {
            op1 = mbr3;
            op2 = mq;
            ula(5, 0);
        }
        else if (ir3 == 12)
        {
            op1 = ac;
            op2 = mbr3;
            ula(6, 0);
        }
        else if (ir3 == 20)
        {
            op1 = ac;
            ula(7, 0);
        }
        else if (ir3 == 21)
        {
            op1 = ac;
            ula(8, 0);
        }
        else if (ir3 == 18)
        {
            op1 = mbr3;
            ula(9, 1);
        }
        else if (ir3 == 19)
        {
            op1 = mbr3;
            ula(9, 2);
        }
        m++;
    }
}
void busca_operandos()
{
    ck2 = ck;
    ir2 = ir;

    if ((ir2 == 10) || (ir2 == 9) || (ir2 == 1) || (ir2 == 2) || (ir2 == 3) || (ir2 == 4) || (ir2 == 5) || (ir2 == 7) || (ir2 == 6) || (ir2 == 8) || (ir2 == 11) || (ir2 == 12) || (ir2 == 20) || (ir2 == 21))
    {
        mbr2 = memoria[mar2];
    }
    else if ((ir2 == 13) || (ir2 == 14) || (ir2 == 15) || (ir2 == 16) || (ir2 == 14) || (ir2 == 18) || (ir2 == 19) || (ir2 == 33))
    {
        mbr2 = mar2;
    }
}
void decodificacao()
{

    long long int aux;
    long long int masc1 = 0xFFFFF00000; // hexadecimal
    long long int masc2 = 1048575;
    int op = 0;
    int end = 0;
    int masc_op = 1044480;
    int masc_end = 4095;

    if (flag_parar == 1)
    {
        ir = 0;
        mar2 = 0;
    }
    else if (flag_parar2 == 1)
    {
        ir = 0;
        mar2 = 0;
    }
    else if (flag_pular2 == 1)
    {
        ir = 0;
        mar2 = 0;
        flag_pular2 = 0;
    }
    else if (flag_continua == 1)
    { // para n decodificar uma instrução desnecessária para continuar as intruções.
        ir = 0;
        mar2 = 0;
        flag_continua = 0;
    }
    else if ((count % 2 == 0) && (flag_ini != 0)) // se for par quer dizer que é necessário buscar o par de instrucoes na memoria
    {

        long long int instrucao_1 = 0;
        long long int instrucao_2 = 0;
        aux = mbr;
        instrucao_1 = aux;
        instrucao_1 = instrucao_1 & masc1;
        instrucao_1 = instrucao_1 >> 20;
        instrucao_2 = aux;
        instrucao_2 = instrucao_2 & masc2;

        if (flag_dir == 0)
        {
            ibr = instrucao_2;
            op = instrucao_1;
            op = op & masc_op;
            op = op >> 12;
            end = instrucao_1;
            end = end & masc_end;
        }
        else
        {
            op = instrucao_2;
            op = op & masc_op;
            op = op >> 12;
            end = instrucao_2;
            end = end & masc_end;
            flag_dir = 0;
            count++; // para que n entre na parte do ibr
        }
        ir = op;
        mar2 = end;
        count++;
        pc = pc + 1;
    }
    else if ((count % 2 != 0) && (flag_dir == 0))
    { // se for impar, quer dizer que já tem uma instrucao em ibr
        op = ibr;
        op = ibr & masc_op;
        op = op >> 12;
        end = ibr;
        end = end & masc_end;
        ir = op;
        mar2 = end;
        count++;
    }
    if ((ir == 13) || (ir == 14) || (ir == 15) || (ir == 16))
    {
        flag_parar = 1;
    }
    if ((ir == 18) || (ir == 19))
    {
        flag_parar2 = 1;
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

    if ((count % 2 == 0) && (flag_parar == 0) && (flag_parar2 == 0))
    {
        mar = pc;
        barramento();
    }
}

void barramento()
{
    long long int bar_dados = 0;
    long long int bar_end = 0;
    bar_end = mar;
    bar_dados = memoria[mar];
    mbr = bar_dados;
}

void ula(int sig, int lado)
{
    if (sig == 1)
    {
        op2 = op1;
    }
    else if (sig == 2)
    {
        if (ac >= 0)
        {
            op2 = op1;
            flag_pular = 1;
            flag_pular2 = 1;
        }
        else
        {
            op2 = pc;
            flag_continua = 1;
        }
    }
    else if (sig == 3)
    {
        op2 = op2 + op1;
    }
    else if (sig == 4)
    {
        op2 = op2 - op1;
    }
    else if (sig == 5)
    {
        op2 = op2 * op1;
    }
    else if (sig == 6)
    {
        op4 = 0;
        op3 = op1;
        while (op3 >= op2)
        {
            op3 -= op2;
            op4++;
        }
    }
    else if (sig == 7)
    {
        op2 = op1 * 2;
    }
    else if (sig == 8)
    {
        op2 = op1 / 2;
        op3 = op1 % 2;
    }
    else if (sig == 9)
    {
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

        instrucao_2 = aux;
        instrucao_2 = instrucao_2 & masc2;

        if (lado == 1)
        {
            op = instrucao_1;
            op = op & masc_op;
            op = op >> 12;

            end = ac;

            instrucao_1 = op << 12;
            instrucao_1 = instrucao_1 | end;
        }
        else if (lado == 2)
        {
            op = instrucao_2;
            op = op & masc_op;
            op = op >> 12;

            end = ac;

            instrucao_2 = op << 12;
            instrucao_2 = instrucao_2 | end;
        }

        aux_mudanca = instrucao_1;
        aux_mudanca = aux_mudanca << 20;
        aux_mudanca = aux_mudanca | instrucao_2;

        flag_continua = 1;
    }
    else if (sig == 10)
    {
        op2 = op1;
        flag_pular2 = 1;
    }
}

void uc()
{
    while (ir4 != 255)
    {
        escrita_resultados();
        execucao();
        busca_operandos();
        decodificacao();
        busca();
    }
}

void imprimir_memoria(FILE *saida)
{
    for (int i = 0; i < 200; i++) // mudar para 4096
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
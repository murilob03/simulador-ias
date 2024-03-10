#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "conversor.c"
#include "memory.h"

int main()
{
    // test of get_op_cycles
    int op_cycles[33];
    op_cycles[0] = 1;  // NOP
    op_cycles[1] = 1;  // LOAD M(X)
    op_cycles[1] = 1;  // LOAD M(X)
    op_cycles[1] = 1;  // LOAD M(X)
    op_cycles[2] = 1;  // LOAD -M(X)
    op_cycles[3] = 1;  // LOAD |M(X)|
    op_cycles[4] = 1;  // LOAD -|M(X)|
    op_cycles[5] = 1;  // ADD M(X)
    op_cycles[6] = 1;  // SUB M(X)
    op_cycles[7] = 1;  // ADD |M(X)|
    op_cycles[8] = 1;  // SUB |M(X)|
    op_cycles[9] = 1;  // LOAD MQ,M(X)
    op_cycles[10] = 1; // LOAD MQ
    op_cycles[11] = 1; // MUL M(X)
    op_cycles[12] = 1; // DIV M(X)
    op_cycles[13] = 1; // JUMP M(X,0:19)
    op_cycles[14] = 1; // JUMP M(X,20:39)
    op_cycles[15] = 1; // JUMP +M(X,0:19)
    op_cycles[16] = 1; // JUMP +M(X,20:39)
    op_cycles[18] = 1; // STOR M(X,8:19)
    op_cycles[19] = 1; // STOR M(X,28:39)
    op_cycles[20] = 1; // LSH
    op_cycles[21] = 1; // RSH
    op_cycles[33] = 1; // STOR M(X)

    get_op_cycles("input.txt", op_cycles);

    // print op_cycles
    printf("\n");
    for (int i = 0; i < 22; i++)
    {
        printf("op_cycles[%d] = %d\n", i, op_cycles[i]);
    }
    printf("op_cycles[33] = %d\n", op_cycles[33]);
}
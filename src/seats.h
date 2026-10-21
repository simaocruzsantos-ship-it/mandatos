/* Calculo de mandatos: metodos de D'Hondt e Sainte-Lague.
   Logica pura (sem dependencias da gint) para poder ser testada no PC. */
#ifndef SEATS_H
#define SEATS_H

#include <stdint.h>

#define MAX_PARTIES 12
#define MAX_DIVS    12
#define MAX_VOTES   9999999u

typedef enum {
    METHOD_DHONDT = 0,       /* divisores 1, 2, 3, 4, 5, ... */
    METHOD_SAINTE_LAGUE = 1  /* divisores 1, 3, 5, 7, 9, ... */
} method_t;

typedef struct {
    /* entradas */
    method_t method;
    int parties;                    /* 2..MAX_PARTIES */
    int divs;                       /* 1..MAX_DIVS */
    int seats;                      /* 0 = so mostrar a tabela */
    uint32_t votes[MAX_PARTIES];

    /* saidas */
    uint32_t divisor[MAX_DIVS];
    uint32_t q10[MAX_PARTIES][MAX_DIVS]; /* quociente x10 (so para mostrar) */
    uint8_t sel[MAX_PARTIES][MAX_DIVS];  /* 1 = quociente que deu mandato */
    int seats_won[MAX_PARTIES];
    int allocated;                  /* mandatos efetivamente atribuidos */
    int tie;                        /* 1 = empate no ultimo mandato */
    int cap_hit;                    /* 1 = um partido usou todos os divisores */
} result_t;

/* k-esimo divisor (k comeca em 1) */
uint32_t seats_divisor(method_t m, int k);

/* preenche divisor/q10/sel/seats_won/... a partir das entradas */
void seats_compute(result_t *r);

#endif

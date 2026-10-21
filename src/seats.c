#include "seats.h"
#include <string.h>

uint32_t seats_divisor(method_t m, int k)
{
    return (m == METHOD_DHONDT) ? (uint32_t)k : (uint32_t)(2 * k - 1);
}

/* Compara votes[p1]/div[d1] com votes[p2]/div[d2] sem divisoes
   (multiplicacao cruzada, cabe em 32 bits: 9 999 999 * 23 < 2^32). */
static int cmp_cell(const result_t *r, int p1, int d1, int p2, int d2)
{
    uint32_t a = r->votes[p1] * r->divisor[d2];
    uint32_t b = r->votes[p2] * r->divisor[d1];
    return (a > b) - (a < b);
}

/* 1 se a celula (p,d) ganha a (bp,bd); em empate fica o partido mais votado,
   e em empate total fica o que ja la estava (indice mais baixo). */
static int beats(const result_t *r, int p, int d, int bp, int bd)
{
    int c = cmp_cell(r, p, d, bp, bd);
    if(c != 0) return c > 0;
    return r->votes[p] > r->votes[bp];
}

/* melhor celula ainda nao selecionada; devolve 0 se nao houver nenhuma */
static int best_free(const result_t *r, int *bp, int *bd)
{
    *bp = -1;
    *bd = -1;
    for(int p = 0; p < r->parties; p++)
        for(int d = 0; d < r->divs; d++) {
            if(r->sel[p][d]) continue;
            if(*bp < 0 || beats(r, p, d, *bp, *bd)) {
                *bp = p;
                *bd = d;
            }
        }
    return *bp >= 0;
}

void seats_compute(result_t *r)
{
    memset(r->sel, 0, sizeof r->sel);
    memset(r->seats_won, 0, sizeof r->seats_won);
    memset(r->q10, 0, sizeof r->q10);
    r->allocated = 0;
    r->tie = 0;
    r->cap_hit = 0;

    for(int d = 0; d < r->divs; d++)
        r->divisor[d] = seats_divisor(r->method, d + 1);

    for(int p = 0; p < r->parties; p++)
        for(int d = 0; d < r->divs; d++)
            r->q10[p][d] = r->votes[p] * 10u / r->divisor[d];

    int lp = -1, ld = -1;
    for(int s = 0; s < r->seats; s++) {
        int bp, bd;
        if(!best_free(r, &bp, &bd)) break;
        r->sel[bp][bd] = 1;
        r->seats_won[bp]++;
        r->allocated++;
        lp = bp;
        ld = bd;
    }

    if(lp >= 0) {
        int bp, bd;
        if(best_free(r, &bp, &bd) && cmp_cell(r, bp, bd, lp, ld) == 0)
            r->tie = 1;
    }

    /* se algum partido gastou todos os divisores, pode merecer mais
       mandatos do que a tabela consegue mostrar */
    if(r->seats > 0)
        for(int p = 0; p < r->parties; p++)
            if(r->seats_won[p] == r->divs) r->cap_hit = 1;
}

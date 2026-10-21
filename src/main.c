/* Mandatos - D'Hondt e Sainte-Lague para a Casio fx-CG50 (gint) */
#include <gint/display.h>
#include <gint/keyboard.h>
#include <stdint.h>
#include <string.h>
#include "seats.h"

#define LABELS "ABCDEFGHIJKL"

/* ---------- utilitarios de texto (sem printf) ---------- */

static int u32_str(uint32_t v, char *out)
{
    char tmp[12];
    int n = 0, i = 0;
    do {
        tmp[n++] = (char)('0' + v % 10);
        v /= 10;
    } while(v);
    while(n) out[i++] = tmp[--n];
    out[i] = 0;
    return i;
}

/* quociente x10 -> "1234.5" */
static void q10_str(uint32_t q, char *out)
{
    int n = u32_str(q / 10, out);
    out[n] = '.';
    out[n + 1] = (char)('0' + q % 10);
    out[n + 2] = 0;
}

static void cat(char *dst, const char *src)
{
    strcat(dst, src);
}

static void cat_u32(char *dst, uint32_t v)
{
    char t[12];
    u32_str(v, t);
    strcat(dst, t);
}

static int digit_of(int key)
{
    switch(key) {
    case KEY_0: return 0;
    case KEY_1: return 1;
    case KEY_2: return 2;
    case KEY_3: return 3;
    case KEY_4: return 4;
    case KEY_5: return 5;
    case KEY_6: return 6;
    case KEY_7: return 7;
    case KEY_8: return 8;
    case KEY_9: return 9;
    }
    return -1;
}

/* ---------- desenho comum ---------- */

static void header(const char *title)
{
    drect(0, 0, DWIDTH - 1, 15, C_BLACK);
    dtext(4, 3, C_WHITE, title);
}

static void footer(const char *txt)
{
    dtext(4, DHEIGHT - 13, C_BLACK, txt);
}

/* ---------- ecra: escolher metodo ---------- */

/* devolve 1 = escolhido, 0 = sair da aplicacao */
static int ask_method(method_t *m)
{
    int sel = (int)*m;
    for(;;) {
        dclear(C_WHITE);
        header("Mandatos - escolhe o metodo");

        const char *names[2] = {"1  D'Hondt", "2  Sainte-Lague"};
        const char *divs[2] = {"divisores 1, 2, 3, 4, 5, ...",
                               "divisores 1, 3, 5, 7, 9, ..."};
        for(int i = 0; i < 2; i++) {
            int y = 40 + i * 50;
            if(i == sel) drect(10, y - 4, DWIDTH - 11, y + 32, C_BLACK);
            int fg = (i == sel) ? C_WHITE : C_BLACK;
            dtext(20, y, fg, names[i]);
            dtext(20, y + 14, fg, divs[i]);
        }
        footer("1/2 ou setas + EXE      EXIT: sair");
        dupdate();

        int k = getkey().key;
        if(k == KEY_1) sel = 0;
        else if(k == KEY_2) sel = 1;
        else if(k == KEY_UP || k == KEY_DOWN) sel = 1 - sel;
        else if(k == KEY_EXE) {
            *m = (method_t)sel;
            return 1;
        }
        else if(k == KEY_EXIT) return 0;
    }
}

/* ---------- ecra: pedir um numero ---------- */

/* devolve 1 = valor lido, 0 = voltar atras (EXIT) */
static int ask_number(const char *title, const char *prompt, const char *hint,
                      uint32_t lo, uint32_t hi, uint32_t init, uint32_t *out)
{
    char buf[12];
    char limit[12];
    int maxlen = u32_str(hi, limit);
    int len = u32_str(init, buf);
    int fresh = 1;
    char msg[48] = "";

    for(;;) {
        dclear(C_WHITE);
        header(title);
        dtext(10, 34, C_BLACK, prompt);
        dtext(10, 50, C_BLACK, hint);

        drect(10, 76, DWIDTH - 11, 104, C_BLACK);
        drect(12, 78, DWIDTH - 13, 102, C_WHITE);
        dtext(18, 84, C_BLACK, buf);

        if(msg[0]) dtext(10, 116, C_RED, msg);
        footer("EXE: ok    DEL: apagar    EXIT: voltar");
        dupdate();

        int k = getkey().key;
        int dg = digit_of(k);

        if(dg >= 0) {
            if(fresh) {
                len = 0;
                buf[0] = 0;
                fresh = 0;
            }
            if(len < maxlen) {
                buf[len++] = (char)('0' + dg);
                buf[len] = 0;
            }
            msg[0] = 0;
        }
        else if(k == KEY_DEL) {
            fresh = 0;
            if(len > 0) buf[--len] = 0;
            msg[0] = 0;
        }
        else if(k == KEY_EXIT) return 0;
        else if(k == KEY_EXE) {
            uint32_t v = 0;
            for(int i = 0; i < len; i++) v = v * 10 + (uint32_t)(buf[i] - '0');
            if(len == 0 || v < lo || v > hi) {
                msg[0] = 0;
                cat(msg, "Valor entre ");
                cat_u32(msg, lo);
                cat(msg, " e ");
                cat_u32(msg, hi);
            }
            else {
                *out = v;
                return 1;
            }
        }
    }
}

/* ---------- ecra: tabela e resumo ---------- */

#define LABEL_W 22
#define CELL_W  58
#define ROW_H   15
#define TOP_Y   36

static void draw_table(const result_t *r, int row0, int col0)
{
    int vis_cols = (DWIDTH - LABEL_W) / CELL_W;
    int vis_rows = (DHEIGHT - TOP_Y - 18) / ROW_H;

    /* cabecalho: divisores */
    dtext(2, 20, C_BLACK, "d");
    for(int c = 0; c < vis_cols && col0 + c < r->divs; c++) {
        char s[12];
        u32_str(r->divisor[col0 + c], s);
        dtext(LABEL_W + c * CELL_W + 4, 20, C_BLACK, s);
    }

    for(int rr = 0; rr < vis_rows && row0 + rr < r->parties; rr++) {
        int p = row0 + rr;
        int y = TOP_Y + rr * ROW_H;
        char lab[2] = {LABELS[p], 0};
        dtext(6, y + 2, C_BLACK, lab);

        for(int c = 0; c < vis_cols && col0 + c < r->divs; c++) {
            int d = col0 + c;
            int x = LABEL_W + c * CELL_W;
            char s[16];
            q10_str(r->q10[p][d], s);
            if(r->sel[p][d]) {
                drect(x, y, x + CELL_W - 2, y + ROW_H - 2, C_BLACK);
                dtext(x + 3, y + 2, C_WHITE, s);
            }
            else {
                dtext(x + 3, y + 2, C_BLACK, s);
            }
        }
    }

    /* indicadores de scroll */
    if(col0 > 0) dtext(DWIDTH - 40, 4, C_WHITE, "<");
    if(col0 + vis_cols < r->divs) dtext(DWIDTH - 28, 4, C_WHITE, ">");
    if(row0 > 0) dtext(DWIDTH - 16, 4, C_WHITE, "^");
    if(row0 + vis_rows < r->parties) dtext(DWIDTH - 8, 4, C_WHITE, "v");
}

static void draw_summary(const result_t *r)
{
    dtext(10, 22, C_BLACK, "Partido");
    dtext(90, 22, C_BLACK, "Votos");
    dtext(220, 22, C_BLACK, "Mandatos");

    for(int p = 0; p < r->parties; p++) {
        int y = 36 + p * 14;
        char lab[2] = {LABELS[p], 0};
        char v[12], m[12];
        u32_str(r->votes[p], v);
        u32_str((uint32_t)r->seats_won[p], m);
        dtext(10, y, C_BLACK, lab);
        dtext(90, y, C_BLACK, v);
        dtext(220, y, C_BLACK, m);
    }
}

/* devolve 1 = nova simulacao (F1), 0 = voltar atras (EXIT) */
static int show_result(const result_t *r)
{
    int row0 = 0, col0 = 0, summary = 0;
    int vis_cols = (DWIDTH - LABEL_W) / CELL_W;
    int vis_rows = (DHEIGHT - TOP_Y - 18) / ROW_H;

    for(;;) {
        char title[48] = "";
        cat(title, r->method == METHOD_DHONDT ? "D'Hondt" : "Sainte-Lague");
        if(r->seats > 0) {
            cat(title, " - ");
            cat_u32(title, (uint32_t)r->seats);
            cat(title, " mandatos");
        }

        dclear(C_WHITE);
        header(title);
        if(summary) draw_summary(r);
        else draw_table(r, row0, col0);

        if(r->seats > 0 && r->cap_hit)
            dtext(4, DHEIGHT - 26, C_RED, "ATENCAO: aumenta os divisores!");
        else if(r->seats > 0 && r->tie)
            dtext(4, DHEIGHT - 26, C_RED, "Empate no ultimo mandato!");
        else if(r->seats > r->allocated)
            dtext(4, DHEIGHT - 26, C_RED, "Mais mandatos do que celulas!");

        footer(summary ? "EXE: tabela  F1: nova  EXIT: voltar"
                       : "EXE: resumo  F1: nova  EXIT: voltar");
        dupdate();

        int k = getkey().key;
        if(k == KEY_EXIT) return 0;
        if(k == KEY_F1) return 1;
        if(k == KEY_EXE && r->seats > 0) summary = !summary;
        if(!summary) {
            if(k == KEY_RIGHT && col0 + vis_cols < r->divs) col0++;
            if(k == KEY_LEFT && col0 > 0) col0--;
            if(k == KEY_DOWN && row0 + vis_rows < r->parties) row0++;
            if(k == KEY_UP && row0 > 0) row0--;
        }
    }
}

/* ---------- programa principal ---------- */

int main(void)
{
    static result_t r;
    int step = 0;
    int idx = 0; /* partido atual no passo dos votos */

    r.method = METHOD_DHONDT;
    r.parties = 5;
    r.divs = 5;
    r.seats = 0;

    for(;;) {
        uint32_t v;

        if(step == 0) {
            if(!ask_method(&r.method)) return 1;
            step = 1;
        }
        else if(step == 1) {
            if(!ask_number("Numero de partidos", "Quantos partidos?",
                           "(de 2 a 12)", 2, MAX_PARTIES,
                           (uint32_t)r.parties, &v)) {
                step = 0;
                continue;
            }
            r.parties = (int)v;
            idx = 0;
            step = 2;
        }
        else if(step == 2) {
            char prompt[40] = "Votos do partido ";
            char lab[2] = {LABELS[idx], 0};
            cat(prompt, lab);
            if(!ask_number("Votos", prompt, "(0 a 9999999)", 0, MAX_VOTES,
                           r.votes[idx], &v)) {
                if(idx == 0) step = 1;
                else idx--;
                continue;
            }
            r.votes[idx] = v;
            if(++idx >= r.parties) step = 3;
        }
        else if(step == 3) {
            const char *hint = (r.method == METHOD_DHONDT)
                ? "(ex.: 5 -> divisores 1 2 3 4 5)"
                : "(ex.: 5 -> divisores 1 3 5 7 9)";
            if(!ask_number("Divisores", "Quantos divisores?", hint, 1, MAX_DIVS,
                           (uint32_t)r.divs, &v)) {
                idx = r.parties - 1;
                step = 2;
                continue;
            }
            r.divs = (int)v;
            step = 4;
        }
        else if(step == 4) {
            if(!ask_number("Mandatos", "Quantos mandatos a distribuir?",
                           "(0 = so mostrar a tabela)", 0, 999,
                           (uint32_t)r.seats, &v)) {
                step = 3;
                continue;
            }
            r.seats = (int)v;
            seats_compute(&r);
            step = 5;
        }
        else {
            if(show_result(&r)) {
                step = 0;
            }
            else step = 4;
        }
    }
}

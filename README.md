# Mandatos (fx-CG50)

Add-in para a Casio fx-CG50 que calcula a tabela de quocientes e a distribuição de
mandatos pelos métodos de **D'Hondt** (divisores 1, 2, 3, 4, 5, …) e
**Sainte-Laguë** (divisores 1, 3, 5, 7, 9, …).

## Como usar na calculadora

1. Escolher o método (1 ou 2, ou setas + EXE).
2. Número de partidos (2 a 12).
3. Votos de cada partido (A, B, C, …; até 9 999 999).
4. Número de divisores (1 a 12). Ex.: 5 → D'Hondt 1 2 3 4 5; Sainte-Laguë 1 3 5 7 9.
5. Número de mandatos (0 = só mostrar a tabela).

Na tabela, as células a preto são os quocientes que deram mandato. Setas para
deslocar, EXE alterna entre tabela e resumo, F1 começa de novo, EXIT volta atrás.
Aparece um aviso se um partido gastar todos os divisores (aumenta os divisores)
ou se houver empate no último mandato.

## Compilar (precisa do fxSDK + gint)

    fxsdk build-cg

O ficheiro `Mandatos.g3a` fica em `build-cg/`. Copia-o para a calculadora
(modo USB "Storage") e abre-o no menu principal.

## Testes da lógica no PC

    gcc -Isrc test/t.c src/seats.c -o t_seats

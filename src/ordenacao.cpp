#include "ordenacao.hpp"

#include <algorithm>
#include <utility>

const char* nomePivo(EstrategiaPivo p) {
    switch (p) {
        case EstrategiaPivo::PRIMEIRO: return "primeiro";
        case EstrategiaPivo::MEIO:     return "meio";
        case EstrategiaPivo::MEDIANA3: return "mediana3";
    }
    return "?";
}

// ---------------------------------------------------------------------------
// Insertion Sort
// ---------------------------------------------------------------------------

void insertionSort(std::vector<int>& v, int esq, int dir, Contadores& c) {
    for (int i = esq + 1; i <= dir; ++i) {
        const int chave = v[i];
        int j = i - 1;
        // Desloca para a direita todos os elementos maiores que 'chave'.
        while (j >= esq) {
            ++c.comparacoes;
            if (v[j] <= chave) break;
            v[j + 1] = v[j];
            ++c.trocas;
            --j;
        }
        if (j + 1 != i) {
            v[j + 1] = chave;
        }
    }
}

void insertionSort(std::vector<int>& v, Contadores& c) {
    if (v.size() < 2) return;
    insertionSort(v, 0, static_cast<int>(v.size()) - 1, c);
}

// ---------------------------------------------------------------------------
// Escolha do pivo
// ---------------------------------------------------------------------------

/**
 * Mediana de tres valores, com contagem de comparacoes (2 no melhor caso,
 * 3 no pior). Recebe copias: nao reordena o vetor, apenas devolve o valor
 * mediano, que sera usado como pivo pela particao de Hoare.
 */
static int mediana3(int a, int b, int d, Contadores& c) {
    ++c.comparacoes;
    if (a > b) std::swap(a, b);          // garante a <= b
    ++c.comparacoes;
    if (b > d) {                         // caso a <= b e d < b: mediana e max(a,d)
        ++c.comparacoes;
        return (a > d) ? a : d;
    }
    return b;                            // caso a <= b <= d
}

static int escolherPivo(const std::vector<int>& v, int esq, int dir,
                        EstrategiaPivo p, Contadores& c) {
    const int meio = esq + (dir - esq) / 2;   // evita overflow de (esq+dir)/2
    switch (p) {
        case EstrategiaPivo::PRIMEIRO: return v[esq];
        case EstrategiaPivo::MEIO:     return v[meio];
        case EstrategiaPivo::MEDIANA3: return mediana3(v[esq], v[meio], v[dir], c);
    }
    return v[meio];
}

// ---------------------------------------------------------------------------
// Particao de Hoare (ZIVIANI, 2018)
// ---------------------------------------------------------------------------

/**
 * Particiona v[esq..dir] em torno do VALOR 'pivo', devolvendo em i e j os
 * indices de cruzamento. Ao final vale: v[esq..j] <= pivo <= v[i..dir], com
 * j < i. Os elementos em (j, i) ja estao na posicao final.
 *
 * A terminacao dos lacos internos e garantida porque 'pivo' e sempre um valor
 * presente em v[esq..dir], funcionando como sentinela natural.
 */
static void particao(std::vector<int>& v, int esq, int dir,
                     int& i, int& j, int pivo, Contadores& c) {
    i = esq;
    j = dir;
    do {
        while (true) {
            ++c.comparacoes;
            if (v[i] >= pivo) break;
            ++i;
        }
        while (true) {
            ++c.comparacoes;
            if (v[j] <= pivo) break;
            --j;
        }
        if (i <= j) {
            std::swap(v[i], v[j]);
            ++c.trocas;
            ++i;
            --j;
        }
    } while (i <= j);
}

// ---------------------------------------------------------------------------
// Motor generico
// ---------------------------------------------------------------------------

static void ordena(std::vector<int>& v, int esq, int dir,
                   Contadores& c, int M, EstrategiaPivo p) {
    if (esq >= dir) return;

    // Interrompe a particao para subvetores com MENOS de M elementos
    // (redacao literal do enunciado). Com M <= 1 nenhum subvetor nao trivial
    // e interrompido, e o algoritmo degenera na versao recursiva pura.
    if (dir - esq + 1 < M) {
        insertionSort(v, esq, dir, c);
        return;
    }

    int i, j;
    const int pivo = escolherPivo(v, esq, dir, p, c);
    particao(v, esq, dir, i, j, pivo, c);
    ordena(v, esq, j, c, M, p);
    ordena(v, i, dir, c, M, p);
}

void quicksort(std::vector<int>& v, Contadores& c, int M, EstrategiaPivo p) {
    if (v.size() < 2) return;
    ordena(v, 0, static_cast<int>(v.size()) - 1, c, M, p);
}

// ---------------------------------------------------------------------------
// As tres versoes pedidas
// ---------------------------------------------------------------------------

void quicksortRecursivo(std::vector<int>& v, Contadores& c, EstrategiaPivo p) {
    quicksort(v, c, 1, p);
}

void quicksortHibrido(std::vector<int>& v, Contadores& c, int M) {
    quicksort(v, c, M, EstrategiaPivo::MEIO);
}

void quicksortHibridoMediana3(std::vector<int>& v, Contadores& c, int M) {
    quicksort(v, c, M, EstrategiaPivo::MEDIANA3);
}

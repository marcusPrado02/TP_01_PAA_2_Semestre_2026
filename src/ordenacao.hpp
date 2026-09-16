#ifndef ORDENACAO_HPP
#define ORDENACAO_HPP

#include <vector>
#include "contadores.hpp"

/**
 * Estrategia de escolha do pivo.
 *
 *  PRIMEIRO : pivo = v[esq]. Escolha "inadequada": sobre vetor ja ordenado
 *             (ou inversamente ordenado) produz particoes de tamanho 0 e n-1,
 *             que e exatamente o pior caso O(n^2) do Quicksort. Usada apenas
 *             no experimento que forca o pior caso (item 2c do enunciado).
 *  MEIO     : pivo = elemento central do subvetor. Escolha classica de ZIVIANI
 *             (2018); e a usada nas versoes (a) e (b) do trabalho.
 *  MEDIANA3 : pivo = mediana entre v[esq], v[meio] e v[dir]. Versao (c).
 */
enum class EstrategiaPivo { PRIMEIRO, MEIO, MEDIANA3 };

const char* nomePivo(EstrategiaPivo p);

// ---------------------------------------------------------------------------
// Insertion Sort
// ---------------------------------------------------------------------------

/** Ordena o subvetor v[esq..dir] por insercao. */
void insertionSort(std::vector<int>& v, int esq, int dir, Contadores& c);

/** Ordena o vetor inteiro por insercao (usado como baseline nos testes). */
void insertionSort(std::vector<int>& v, Contadores& c);

// ---------------------------------------------------------------------------
// Versoes do Quicksort exigidas pelo enunciado
// ---------------------------------------------------------------------------

/**
 * (a) Quicksort recursivo puro: recursao ate subvetores de tamanho 1.
 * O parametro de pivo permite reutilizar esta mesma versao no experimento
 * de pior caso, trocando apenas a estrategia de escolha do pivo.
 */
void quicksortRecursivo(std::vector<int>& v, Contadores& c,
                        EstrategiaPivo p = EstrategiaPivo::MEIO);

/**
 * (b) Quicksort hibrido: a particao e interrompida para subvetores com MENOS
 * de M elementos, que sao entao ordenados por Insertion Sort.
 * M = 1 equivale a versao (a) (nenhum subvetor tem menos de 1 elemento).
 */
void quicksortHibrido(std::vector<int>& v, Contadores& c, int M);

/**
 * (c) Quicksort hibrido com pivo por mediana-de-tres.
 */
void quicksortHibridoMediana3(std::vector<int>& v, Contadores& c, int M);

// ---------------------------------------------------------------------------
// Motor generico (exposto para uso do driver de experimentos)
// ---------------------------------------------------------------------------

/**
 * Motor unico das tres versoes. As versoes acima sao apenas atalhos com
 * (M, EstrategiaPivo) fixos. Manter um unico motor garante que as tres
 * variantes contem comparacoes e trocas exatamente da mesma maneira - condicao
 * necessaria para que a comparacao experimental seja valida.
 */
void quicksort(std::vector<int>& v, Contadores& c, int M, EstrategiaPivo p);

#endif // ORDENACAO_HPP

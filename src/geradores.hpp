#ifndef GERADORES_HPP
#define GERADORES_HPP

#include <string>
#include <vector>

/**
 * Tipos de massa de teste exigidos pelo item 2 do enunciado.
 *
 *  ALEATORIO      : chaves uniformes em [0, 10n) - caso medio.
 *  ORDENADO       : 0,1,2,...,n-1 - melhor caso para pivo central,
 *                   pior caso para pivo = primeiro elemento.
 *  INVERSO        : n-1,...,1,0.
 *  REPETIDOS      : apenas NUM_VALORES_REPETIDOS chaves distintas, sorteadas
 *                   uniformemente - estressa o tratamento de chaves iguais.
 *  QUASE_ORDENADO : vetor ordenado com ~1% dos elementos trocados de posicao -
 *                   caso comum na pratica, favoravel ao Insertion Sort.
 */
enum class TipoMassa { ALEATORIO, ORDENADO, INVERSO, REPETIDOS, QUASE_ORDENADO };

/** Numero de chaves distintas na massa REPETIDOS. */
constexpr int NUM_VALORES_REPETIDOS = 10;

/** Todas as massas, na ordem em que aparecem nos relatorios. */
const std::vector<TipoMassa>& todasAsMassas();

const char* nomeMassa(TipoMassa t);

/**
 * Gera uma massa de teste deterministica: mesma (tipo, n, semente) sempre
 * produz o mesmo vetor. Isso garante que todas as versoes do algoritmo sejam
 * medidas sobre EXATAMENTE os mesmos dados (requisito adicional (c)).
 */
std::vector<int> gerarMassa(TipoMassa t, int n, unsigned semente);

#endif // GERADORES_HPP

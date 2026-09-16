#ifndef CONTADORES_HPP
#define CONTADORES_HPP

#include <cstdint>

/**
 * Contadores de operacoes elementares, atualizados pelos algoritmos de
 * ordenacao (requisito adicional (b) do enunciado).
 *
 * Convencoes de contagem adotadas neste trabalho (documentadas tambem no
 * README e no relatorio, para que os numeros sejam interpretaveis):
 *
 *  - comparacoes: toda comparacao entre CHAVES do vetor (elemento x pivo,
 *    elemento x elemento). Comparacoes de indices (i <= j, j >= esq) NAO sao
 *    contadas, pois sao controle de laco e nao dependem dos dados.
 *
 *  - trocas: toda MOVIMENTACAO de chave dentro do vetor.
 *      * no Quicksort, cada swap da particao conta como 1 troca;
 *      * no Insertion Sort, cada deslocamento v[j+1] = v[j] conta como 1 troca.
 *    Essa escolha mantem as duas grandezas na mesma unidade ("quantas vezes um
 *    elemento mudou de lugar"), permitindo somar os dois algoritmos nas versoes
 *    hibridas sem distorcer a comparacao.
 */
struct Contadores {
    std::uint64_t comparacoes = 0;
    std::uint64_t trocas = 0;

    void zerar() {
        comparacoes = 0;
        trocas = 0;
    }
};

#endif // CONTADORES_HPP

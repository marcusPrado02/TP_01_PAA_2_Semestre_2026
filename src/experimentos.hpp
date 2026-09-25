#ifndef EXPERIMENTOS_HPP
#define EXPERIMENTOS_HPP

#include <cstdint>
#include <string>
#include <vector>

#include "geradores.hpp"
#include "ordenacao.hpp"

/** Uma configuracao de algoritmo a ser medida. */
struct Algoritmo {
    std::string nome;     // rotulo usado nos CSVs e no relatorio
    int M;                // corte para Insertion Sort (1 = sem corte)
    EstrategiaPivo pivo;
};

/** Resultado de UMA execucao (uma repeticao sobre uma massa). */
struct Medida {
    double tempo_ms = 0.0;
    std::uint64_t comparacoes = 0;
    std::uint64_t trocas = 0;
};

/** Executa o algoritmo sobre uma COPIA de 'entrada' e devolve as metricas. */
Medida executar(const Algoritmo& alg, const std::vector<int>& entrada);

/**
 * Estatisticas agregadas sobre varias repeticoes.
 *
 * Alem da media exigida pelo enunciado, guardamos MEDIANA e MINIMO porque o
 * ruido de medicao (escalonador do SO, escalonamento de frequencia da CPU)
 * so pode ACRESCENTAR tempo, nunca subtrair: a media e puxada por outliers,
 * enquanto mediana e minimo sao estimadores mais fieis do custo real.
 * A analise do relatorio usa a mediana; a media e o desvio ficam registrados
 * para documentar a dispersao das medidas.
 */
struct Resumo {
    double tempo_medio_ms = 0.0;
    double tempo_mediana_ms = 0.0;
    double tempo_min_ms = 0.0;
    double tempo_desvio_ms = 0.0;
    /**
     * Desvio absoluto mediano (MAD), robusto a outliers: 1.4826 * mediana(|x -
     * mediana|). O desvio-padrao classico e dominado por um unico pico de
     * sistema (preempcao, interrupcao) e chega a superestimar a dispersao real
     * em uma ordem de grandeza; o MAD estima a dispersao tipica. Ex.: em n=1000
     * uma unica medida de 0,0154 ms entre 50 de 0,0058 ms eleva o desvio
     * classico a 23%, enquanto o MAD permanece em ~0,1%.
     */
    double tempo_mad_ms = 0.0;
    double comparacoes_medias = 0.0;
    double trocas_medias = 0.0;
    int repeticoes = 0;
};

Resumo resumir(std::vector<Medida> medidas);

/**
 * Leva a CPU ao regime permanente de frequencia antes de qualquer medicao.
 *
 * MOTIVACAO (documentada no relatorio): em testes preliminares a MESMA carga
 * mediu 16,2 ms na primeira execucao do processo e 5,3 ms apos alguns segundos
 * de atividade - variacao de 3x causada pelo governor de frequencia da CPU, nao
 * pelo algoritmo. Sem esta fase, o M "otimo" medido era artefato de aquecimento.
 * Repetir e tirar media NAO corrige esse vies, por ser sistematico e nao
 * aleatorio; e preciso descartar a fase transitoria.
 */
void aquecerCPU();

// --- Rotinas de experimento (cada uma grava seus proprios CSVs) -------------

/** Verifica a corretude das tres versoes contra std::sort. */
bool validar();

/** Busca empirica do melhor M (item (b) do enunciado). */
void calibrarM(const std::string& dirSaida);

/** Bateria principal: 3 versoes x 5 massas x varios n. */
void experimentosPrincipais(const std::string& dirSaida, int M);

/** Experimento que forca explicitamente o pior caso (item 2c). */
void experimentoPiorCaso(const std::string& dirSaida, int M);

#endif // EXPERIMENTOS_HPP

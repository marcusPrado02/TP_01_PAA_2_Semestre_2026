/**
 * Trabalho Pratico de PAA - Estudo comparativo de implementacoes do Quicksort
 * PUC Minas - Prof. Walisson Ferreira de Carvalho
 *
 * Fonte: exercicio 15, cap. 4 de ZIVIANI, N. Projeto de Algoritmos, 3. ed.
 *
 * Uso:
 *   ./bin/quicksort validar             verifica a corretude das tres versoes
 *   ./bin/quicksort calibrar            busca empirica do melhor M
 *   ./bin/quicksort experimentos [M]    bateria principal (padrao: M = 40)
 *   ./bin/quicksort pior-caso [M]       experimento de pior caso forcado
 *   ./bin/quicksort tudo [M]            executa tudo, na ordem acima
 */

#include <cstdlib>
#include <iostream>
#include <string>

#include "experimentos.hpp"

namespace {

constexpr int M_PADRAO = 40;   // valor obtido pela calibracao empirica (ver README)
const std::string DIR_SAIDA = "resultados";

void uso(const char* prog) {
    std::cout <<
        "Uso: " << prog << " <comando> [M]\n\n"
        "Comandos:\n"
        "  validar         verifica a corretude das tres versoes contra std::sort\n"
        "  calibrar        busca empirica do melhor valor de M\n"
        "  experimentos    bateria principal: 3 versoes x 5 massas x 4 tamanhos\n"
        "  pior-caso       forca o pior caso do Quicksort (pivo inadequado)\n"
        "  tudo            executa todos os comandos acima\n\n"
        "  [M]  corte para o Insertion Sort nas versoes hibridas (padrao: "
        << M_PADRAO << ")\n\n"
        "Os resultados sao gravados em CSV no diretorio '" << DIR_SAIDA << "/'.\n";
}

} // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        uso(argv[0]);
        return 1;
    }

    const std::string comando = argv[1];
    int M = M_PADRAO;
    if (argc >= 3) {
        M = std::atoi(argv[2]);
        if (M < 1) {
            std::cerr << "ERRO: M deve ser >= 1 (recebido: " << argv[2] << ").\n";
            return 1;
        }
    }

    if (comando == "validar") {
        return validar() ? 0 : 1;
    }
    if (comando == "calibrar") {
        aquecerCPU();
        calibrarM(DIR_SAIDA);
        return 0;
    }
    if (comando == "experimentos") {
        aquecerCPU();
        experimentosPrincipais(DIR_SAIDA, M);
        return 0;
    }
    if (comando == "pior-caso") {
        aquecerCPU();
        experimentoPiorCaso(DIR_SAIDA, M);
        return 0;
    }
    if (comando == "tudo") {
        if (!validar()) return 1;
        aquecerCPU();
        calibrarM(DIR_SAIDA);
        experimentosPrincipais(DIR_SAIDA, M);
        experimentoPiorCaso(DIR_SAIDA, M);
        std::cout << "Concluido. CSVs em '" << DIR_SAIDA << "/'.\n";
        return 0;
    }

    std::cerr << "Comando desconhecido: " << comando << "\n\n";
    uso(argv[0]);
    return 1;
}

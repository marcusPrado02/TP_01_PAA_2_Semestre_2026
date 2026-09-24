#include "experimentos.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <numeric>
#include <utility>

namespace {

using Relogio = std::chrono::steady_clock;

/** Semente base fixa: torna toda a bateria de testes reproduzivel. */
constexpr unsigned SEMENTE_BASE = 20262026u;

/** Semente da repeticao r para uma massa de tamanho n. */
unsigned semente(int n, int repeticao) {
    return SEMENTE_BASE + static_cast<unsigned>(n) * 1000u
           + static_cast<unsigned>(repeticao);
}

/** Numero de repeticoes: mais repeticoes para entradas pequenas (mais ruido). */
int repeticoesPara(int n) {
    if (n <= 10000)  return 10;
    if (n <= 100000) return 5;
    return 3;
}

std::ofstream abrirCsv(const std::string& caminho, const std::string& cabecalho) {
    std::ofstream out(caminho);
    if (!out) {
        std::cerr << "ERRO: nao foi possivel escrever em " << caminho << "\n";
        std::exit(1);
    }
    // Precisao alta para nao perder algarismos em contagens grandes (ex.:
    // 1250074998 viraria 1.25007e+09 sem isso) nem no tempo.
    out << std::setprecision(12);
    out << cabecalho << "\n";
    std::cout << "  -> " << caminho << "\n";
    return out;
}

} // namespace

// ---------------------------------------------------------------------------

Medida executar(const Algoritmo& alg, const std::vector<int>& entrada) {
    std::vector<int> v = entrada;           // copia: a geracao nao entra no tempo
    Contadores c;

    const auto t0 = Relogio::now();
    quicksort(v, c, alg.M, alg.pivo);
    const auto t1 = Relogio::now();

    Medida m;
    m.tempo_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
    m.comparacoes = c.comparacoes;
    m.trocas = c.trocas;

    // Barreira de sanidade: um algoritmo que nao ordena invalida a medicao.
    if (!std::is_sorted(v.begin(), v.end())) {
        std::cerr << "ERRO: " << alg.nome << " nao ordenou o vetor!\n";
        std::exit(1);
    }
    return m;
}

Resumo resumir(std::vector<Medida> medidas) {
    Resumo r;
    if (medidas.empty()) return r;
    r.repeticoes = static_cast<int>(medidas.size());

    for (const Medida& m : medidas) {
        r.tempo_medio_ms      += m.tempo_ms;
        r.comparacoes_medias  += static_cast<double>(m.comparacoes);
        r.trocas_medias       += static_cast<double>(m.trocas);
    }
    r.tempo_medio_ms     /= r.repeticoes;
    r.comparacoes_medias /= r.repeticoes;
    r.trocas_medias      /= r.repeticoes;

    double soma2 = 0.0;
    for (const Medida& m : medidas) {
        const double d = m.tempo_ms - r.tempo_medio_ms;
        soma2 += d * d;
    }
    r.tempo_desvio_ms = (r.repeticoes > 1)
        ? std::sqrt(soma2 / (r.repeticoes - 1))   // desvio amostral
        : 0.0;

    std::sort(medidas.begin(), medidas.end(),
              [](const Medida& a, const Medida& b) { return a.tempo_ms < b.tempo_ms; });
    const std::size_t k = medidas.size();
    r.tempo_min_ms = medidas.front().tempo_ms;
    r.tempo_mediana_ms = (k % 2 == 1)
        ? medidas[k / 2].tempo_ms
        : 0.5 * (medidas[k / 2 - 1].tempo_ms + medidas[k / 2].tempo_ms);
    return r;
}

void aquecerCPU() {
    std::cout << "Aquecendo a CPU (regime permanente de frequencia)..." << std::flush;
    const std::vector<int> v = gerarMassa(TipoMassa::ALEATORIO, 200000, 7u);
    const auto inicio = Relogio::now();
    std::uint64_t descartavel = 0;
    while (std::chrono::duration<double>(Relogio::now() - inicio).count() < 2.0) {
        std::vector<int> copia = v;
        Contadores c;
        quicksort(copia, c, 20, EstrategiaPivo::MEIO);
        descartavel += c.comparacoes;
    }
    // Impede que o otimizador elimine o laco de aquecimento.
    if (descartavel == 0) std::cerr << "";
    std::cout << " ok\n\n";
}

/**
 * Mede uma configuracao com 'reps' repeticoes, descartando execucoes de
 * aquecimento (cache de dados e preditor de saltos) que nao entram na media.
 */
static std::vector<Medida> medirRepeticoes(const Algoritmo& alg,
                                           const std::vector<std::vector<int>>& entradas) {
    // Aquecimento local: 1 execucao descartada por configuracao.
    if (!entradas.empty()) {
        (void) executar(alg, entradas.front());
    }
    std::vector<Medida> medidas;
    medidas.reserve(entradas.size());
    for (const std::vector<int>& e : entradas) {
        medidas.push_back(executar(alg, e));
    }
    return medidas;
}

// ---------------------------------------------------------------------------
// Validacao de corretude
// ---------------------------------------------------------------------------

bool validar() {
    std::cout << "== Validacao de corretude ==\n";

    const std::vector<Algoritmo> algs = {
        {"quicksort_recursivo",  1,  EstrategiaPivo::MEIO},
        {"quicksort_hibrido",    20, EstrategiaPivo::MEIO},
        {"quicksort_mediana3",   20, EstrategiaPivo::MEDIANA3},
        {"quicksort_pivo_prim",  1,  EstrategiaPivo::PRIMEIRO},
    };

    bool ok = true;
    int casos = 0;

    // Casos degenerados (vetor vazio, 1 elemento, todos iguais, 2 elementos).
    const std::vector<std::vector<int>> degenerados = {
        {}, {42}, {2, 1}, {1, 2}, {7, 7, 7, 7, 7}, {3, 1, 3, 1, 3},
    };

    for (const Algoritmo& alg : algs) {
        for (std::vector<int> v : degenerados) {
            std::vector<int> esperado = v;
            std::sort(esperado.begin(), esperado.end());
            Contadores c;
            quicksort(v, c, alg.M, alg.pivo);
            ++casos;
            if (v != esperado) {
                std::cerr << "  FALHA (degenerado) em " << alg.nome << "\n";
                ok = false;
            }
        }

        // Casos aleatorios de todos os tipos de massa e tamanhos variados.
        for (TipoMassa t : todasAsMassas()) {
            for (int n : {1, 2, 3, 5, 17, 100, 999, 5000}) {
                for (int rep = 0; rep < 3; ++rep) {
                    std::vector<int> v = gerarMassa(t, n, semente(n, rep));
                    std::vector<int> esperado = v;
                    std::sort(esperado.begin(), esperado.end());
                    Contadores c;
                    quicksort(v, c, alg.M, alg.pivo);
                    ++casos;
                    if (v != esperado) {
                        std::cerr << "  FALHA em " << alg.nome
                                  << " massa=" << nomeMassa(t)
                                  << " n=" << n << " rep=" << rep << "\n";
                        ok = false;
                    }
                }
            }
        }
    }

    // Insertion Sort isolado (usado pelas versoes hibridas).
    for (int n : {0, 1, 2, 50, 1000}) {
        std::vector<int> v = gerarMassa(TipoMassa::ALEATORIO, n, semente(n, 0));
        std::vector<int> esperado = v;
        std::sort(esperado.begin(), esperado.end());
        Contadores c;
        insertionSort(v, c);
        ++casos;
        if (v != esperado) {
            std::cerr << "  FALHA no insertionSort com n=" << n << "\n";
            ok = false;
        }
    }

    std::cout << (ok ? "  OK: " : "  ERRO: ") << casos << " casos verificados.\n\n";
    return ok;
}

// ---------------------------------------------------------------------------
// Calibracao empirica de M
// ---------------------------------------------------------------------------

void calibrarM(const std::string& dirSaida) {
    std::cout << "== Calibracao empirica de M ==\n";

    // O enunciado sugere vetores de 1000 elementos; incluimos tambem 10^4 e
    // 10^5 para verificar se o M otimo depende do tamanho da entrada.
    const std::vector<int> tamanhos = {1000, 10000, 100000};
    const std::vector<int> valoresM = {
        1, 2, 3, 4, 5, 6, 8, 10, 12, 15, 20, 25, 30, 40, 50, 70, 100, 150, 200
    };
    // Massas usadas na calibracao: as tres mais representativas.
    const std::vector<TipoMassa> massas = {
        TipoMassa::ALEATORIO, TipoMassa::ORDENADO, TipoMassa::REPETIDOS
    };
    // Repeticoes adaptativas: entradas pequenas sao ordenadas em dezenas de
    // microssegundos e por isso sofrem muito mais com transientes do SO. O
    // numero de repeticoes cresce quando n diminui, mantendo o tempo total
    // medido por celula na casa das centenas de milissegundos.
    auto repsPara = [](int n) {
        const int r = 1000000 / std::max(n, 1);
        return std::min(200, std::max(15, r));
    };

    // Tolerancia do criterio de "platô": o M recomendado e o MENOR M cujo tempo
    // fica dentro de TOLERANCIA do melhor tempo observado. Motivo: perto do
    // otimo a curva e praticamente plana, e o argmin puro passa a escolher com
    // base em ruido (em execucoes preliminares ele oscilou entre 12 e 100 sem
    // diferenca real de desempenho). Preferir o menor M dentro da faixa otima
    // e reproduzivel e mantem o Insertion Sort restrito a subvetores pequenos.
    constexpr double TOLERANCIA = 1.02;   // 2% acima do melhor tempo

    std::ofstream csv = abrirCsv(dirSaida + "/calibracao_m.csv",
        "pivo,massa,n,M,tempo_mediana_ms,tempo_medio_ms,tempo_min_ms,tempo_desvio_ms,"
        "comparacoes_medias,trocas_medias,repeticoes");

    // Melhor M por (pivo, n). Guardamos DOIS criterios:
    //  - tempo (mediana somada sobre as massas): o que interessa na pratica,
    //    porem sujeito a ruido de medicao;
    //  - comparacoes (deterministico): reproduzivel, serve de controle. Se os
    //    dois criterios discordarem muito, a medicao de tempo esta contaminada.
    struct Melhor {
        int M_tempo = 0;  double tempo = std::numeric_limits<double>::max();
        int M_comp  = 0;  double comps = std::numeric_limits<double>::max();
    };

    for (EstrategiaPivo pivo : {EstrategiaPivo::MEIO, EstrategiaPivo::MEDIANA3}) {
        std::cout << "  pivo = " << nomePivo(pivo) << "\n";
        for (int n : tamanhos) {
            const int REPS = repsPara(n);
            Melhor melhor;
            std::vector<std::pair<int, double>> curva;   // (M, tempo somado)
            for (int M : valoresM) {
                double tempoTotal = 0.0;
                double compsTotal = 0.0;
                for (TipoMassa t : massas) {
                    std::vector<std::vector<int>> entradas;
                    entradas.reserve(REPS);
                    for (int rep = 0; rep < REPS; ++rep) {
                        entradas.push_back(gerarMassa(t, n, semente(n, rep)));
                    }
                    const Resumo r = resumir(medirRepeticoes({"calib", M, pivo}, entradas));
                    csv << nomePivo(pivo) << ',' << nomeMassa(t) << ',' << n << ',' << M
                        << ',' << r.tempo_mediana_ms << ',' << r.tempo_medio_ms
                        << ',' << r.tempo_min_ms << ',' << r.tempo_desvio_ms
                        << ',' << r.comparacoes_medias << ',' << r.trocas_medias
                        << ',' << r.repeticoes << '\n';
                    // Criterio de selecao: MEDIANA (robusta a ruido do SO).
                    tempoTotal += r.tempo_mediana_ms;
                    compsTotal += r.comparacoes_medias;
                }
                if (tempoTotal < melhor.tempo) {
                    melhor.tempo = tempoTotal;
                    melhor.M_tempo = M;
                }
                if (compsTotal < melhor.comps) {
                    melhor.comps = compsTotal;
                    melhor.M_comp = M;
                }
                curva.emplace_back(M, tempoTotal);
            }

            // Menor M dentro da faixa de TOLERANCIA em torno do melhor tempo.
            int M_recomendado = melhor.M_tempo;
            for (const auto& [M, tempo] : curva) {
                if (tempo <= melhor.tempo * TOLERANCIA) { M_recomendado = M; break; }
            }

            std::printf("    n = %7d (%3d reps) -> M recomendado = %3d | "
                        "argmin tempo = %3d (%.3f ms) | argmin comparacoes = %3d\n",
                        n, REPS, M_recomendado, melhor.M_tempo, melhor.tempo,
                        melhor.M_comp);
        }
    }
    csv.close();
    std::cout << "\n";
}

// ---------------------------------------------------------------------------
// Bateria principal
// ---------------------------------------------------------------------------

void experimentosPrincipais(const std::string& dirSaida, int M) {
    std::cout << "== Experimentos principais (M = " << M << ") ==\n";

    const std::vector<int> tamanhos = {1000, 10000, 100000, 500000};

    const std::vector<Algoritmo> algs = {
        {"quicksort_recursivo", 1, EstrategiaPivo::MEIO},
        {"quicksort_hibrido",   M, EstrategiaPivo::MEIO},
        {"quicksort_mediana3",  M, EstrategiaPivo::MEDIANA3},
    };

    std::ofstream bruto = abrirCsv(dirSaida + "/experimentos_bruto.csv",
        "algoritmo,M,pivo,massa,n,repeticao,tempo_ms,comparacoes,trocas");
    std::ofstream resumo = abrirCsv(dirSaida + "/experimentos_resumo.csv",
        "algoritmo,M,pivo,massa,n,repeticoes,tempo_mediana_ms,tempo_medio_ms,"
        "tempo_min_ms,tempo_desvio_ms,comparacoes_medias,trocas_medias");

    for (TipoMassa t : todasAsMassas()) {
        std::printf("\n  massa = %s\n", nomeMassa(t));
        std::printf("  %-22s %8s %7s %14s %12s %10s %16s %16s\n",
                    "algoritmo", "n", "reps", "mediana(ms)", "media(ms)", "desvio",
                    "comparacoes", "trocas");
        for (int n : tamanhos) {
            const int reps = repeticoesPara(n);

            // Todas as repeticoes sao pre-geradas para que os tres algoritmos
            // recebam exatamente as mesmas entradas.
            std::vector<std::vector<int>> entradas;
            entradas.reserve(reps);
            for (int rep = 0; rep < reps; ++rep) {
                entradas.push_back(gerarMassa(t, n, semente(n, rep)));
            }

            for (const Algoritmo& alg : algs) {
                const std::vector<Medida> medidas = medirRepeticoes(alg, entradas);
                for (int rep = 0; rep < reps; ++rep) {
                    const Medida& m = medidas[rep];
                    bruto << alg.nome << ',' << alg.M << ',' << nomePivo(alg.pivo) << ','
                          << nomeMassa(t) << ',' << n << ',' << rep << ','
                          << m.tempo_ms << ',' << m.comparacoes << ',' << m.trocas << '\n';
                }
                const Resumo r = resumir(medidas);
                resumo << alg.nome << ',' << alg.M << ',' << nomePivo(alg.pivo) << ','
                       << nomeMassa(t) << ',' << n << ',' << r.repeticoes << ','
                       << r.tempo_mediana_ms << ',' << r.tempo_medio_ms << ','
                       << r.tempo_min_ms << ',' << r.tempo_desvio_ms << ','
                       << r.comparacoes_medias << ',' << r.trocas_medias << '\n';
                std::printf("  %-22s %8d %7d %14.3f %12.3f %10.3f %16.0f %16.0f\n",
                            alg.nome.c_str(), n, r.repeticoes, r.tempo_mediana_ms,
                            r.tempo_medio_ms, r.tempo_desvio_ms,
                            r.comparacoes_medias, r.trocas_medias);
            }
        }
    }
    std::cout << "\n";
}

// ---------------------------------------------------------------------------
// Pior caso forcado
// ---------------------------------------------------------------------------

void experimentoPiorCaso(const std::string& dirSaida, int M) {
    std::cout << "== Pior caso forcado (pivo = primeiro elemento) ==\n";
    std::cout << "  Entrada ja ordenada + pivo inadequado => particoes de "
                 "tamanho 0 e n-1 => Theta(n^2).\n";

    // Tamanhos limitados: com pivo = primeiro elemento sobre vetor ordenado a
    // profundidade da recursao e n, e um n maior estoura a pilha (ver README).
    const std::vector<int> tamanhos = {1000, 2000, 5000, 10000, 20000, 50000};

    const std::vector<Algoritmo> algs = {
        {"quicksort_pivo_primeiro", 1, EstrategiaPivo::PRIMEIRO},  // pior caso
        {"quicksort_recursivo",     1, EstrategiaPivo::MEIO},      // controle
        {"quicksort_mediana3",      M, EstrategiaPivo::MEDIANA3},  // controle
    };
    const std::vector<TipoMassa> massas = {TipoMassa::ORDENADO, TipoMassa::INVERSO};

    std::ofstream csv = abrirCsv(dirSaida + "/pior_caso.csv",
        "algoritmo,M,pivo,massa,n,repeticoes,tempo_mediana_ms,tempo_medio_ms,"
        "tempo_min_ms,tempo_desvio_ms,comparacoes_medias,trocas_medias,"
        "comparacoes_sobre_n2");

    for (TipoMassa t : massas) {
        std::printf("\n  massa = %s\n", nomeMassa(t));
        std::printf("  %-26s %8s %14s %16s %12s\n",
                    "algoritmo", "n", "mediana(ms)", "comparacoes", "comp/n^2");
        for (int n : tamanhos) {
            const int reps = 5;
            const std::vector<int> entrada = gerarMassa(t, n, semente(n, 0));
            // O pior caso e deterministico: a mesma entrada e reutilizada em
            // todas as repeticoes (a variacao medida e apenas de medicao).
            const std::vector<std::vector<int>> entradas(reps, entrada);
            for (const Algoritmo& alg : algs) {
                const Resumo r = resumir(medirRepeticoes(alg, entradas));
                const double razao = r.comparacoes_medias
                                   / (static_cast<double>(n) * static_cast<double>(n));
                csv << alg.nome << ',' << alg.M << ',' << nomePivo(alg.pivo) << ','
                    << nomeMassa(t) << ',' << n << ',' << r.repeticoes << ','
                    << r.tempo_mediana_ms << ',' << r.tempo_medio_ms << ','
                    << r.tempo_min_ms << ',' << r.tempo_desvio_ms << ','
                    << r.comparacoes_medias << ',' << r.trocas_medias << ','
                    << razao << '\n';
                std::printf("  %-26s %8d %14.3f %16.0f %12.4f\n",
                            alg.nome.c_str(), n, r.tempo_mediana_ms,
                            r.comparacoes_medias, razao);
            }
        }
    }
    std::cout << "\n";
}

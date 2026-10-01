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
    if (n <= 1000)   return 50;   // n pequeno: tempo ~5 us, exige mais reps
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

    // Desvio absoluto mediano (MAD), robusto a outliers.
    std::vector<double> desvios;
    desvios.reserve(k);
    for (const Medida& m : medidas) {
        desvios.push_back(std::abs(m.tempo_ms - r.tempo_mediana_ms));
    }
    std::sort(desvios.begin(), desvios.end());
    const double mad = (k % 2 == 1)
        ? desvios[k / 2]
        : 0.5 * (desvios[k / 2 - 1] + desvios[k / 2]);
    r.tempo_mad_ms = 1.4826 * mad;   // consistente com o desvio-padrao sob normalidade
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

/**
 * Mede varios algoritmos sobre as mesmas entradas em regime de rodizio
 * (round-robin): a cada repeticao executa todos os algoritmos, um apos o outro.
 *
 * Motivo: medir todos os A, depois todos os B e por fim todos os C faz com que
 * qualquer deriva lenta da maquina (temperatura, frequencia, carga) penalize
 * sistematicamente as medidas mais tardias. O rodizio distribui essa deriva
 * igualmente entre as versoes, de modo que a comparacao entre elas seja justa.
 * Retorna um vetor de medidas por algoritmo, na mesma ordem de 'algs'.
 */
static std::vector<std::vector<Medida>> medirIntercalado(
        const std::vector<Algoritmo>& algs,
        const std::vector<std::vector<int>>& entradas) {
    // Aquecimento local: 1 execucao descartada por algoritmo.
    if (!entradas.empty()) {
        for (const Algoritmo& alg : algs) {
            (void) executar(alg, entradas.front());
        }
    }
    std::vector<std::vector<Medida>> medidas(algs.size());
    for (std::vector<Medida>& m : medidas) m.reserve(entradas.size());

    for (const std::vector<int>& e : entradas) {
        for (std::size_t i = 0; i < algs.size(); ++i) {
            medidas[i].push_back(executar(algs[i], e));
        }
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
        "tempo_mad_ms,comparacoes_medias,trocas_medias,repeticoes");

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
                        << ',' << r.tempo_mad_ms
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
        "tempo_min_ms,tempo_desvio_ms,tempo_mad_ms,comparacoes_medias,trocas_medias");

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

            // Mede os tres algoritmos em rodizio sobre as mesmas entradas, para
            // que eventual deriva lenta da maquina nao penalize uma versao.
            const std::vector<std::vector<Medida>> todas =
                medirIntercalado(algs, entradas);

            for (std::size_t ia = 0; ia < algs.size(); ++ia) {
                const Algoritmo& alg = algs[ia];
                const std::vector<Medida>& medidas = todas[ia];
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
                       << r.tempo_mad_ms << ','
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
        "tempo_min_ms,tempo_desvio_ms,tempo_mad_ms,comparacoes_medias,trocas_medias,"
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
                    << r.tempo_mad_ms << ','
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

// ---------------------------------------------------------------------------
// Escolha dos tamanhos de entrada
// ---------------------------------------------------------------------------

void escolherTamanhos(const std::string& dirSaida, int M) {
    std::cout << "== Escolha dos tamanhos de entrada (n) ==\n";
    std::cout << "  Criterios: (1) piso de ruido; (2) regime assintotico; "
                 "(3) teto pratico.\n\n";

    // Resolucao efetiva do relogio: menor delta nao-nulo entre duas leituras.
    // Serve para expressar cada tempo em "ticks" e julgar se a medicao tem
    // folga suficiente sobre a granularidade do relogio.
    double resolucao_ms = 1e9;
    for (int i = 0; i < 2000; ++i) {
        const auto a = Relogio::now();
        const auto b = Relogio::now();
        const double d = std::chrono::duration<double, std::milli>(b - a).count();
        if (d > 0.0 && d < resolucao_ms) resolucao_ms = d;
    }
    std::printf("  Resolucao do relogio: %.4f us\n", resolucao_ms * 1000.0);
    std::printf("  Constante teorica do caso medio (recursivo puro): %.4f\n\n",
                2.0 * std::log(2.0));

    // Faixa varrida: do muito pequeno (para achar o piso) ao teto pratico.
    const std::vector<int> tamanhos = {
        100, 200, 500, 1000, 2000, 5000, 10000,
        20000, 50000, 100000, 200000, 500000
    };
    const int reps = 30;
    const int sementes = 4;

    std::ofstream csv = abrirCsv(dirSaida + "/escolha_n.csv",
        "n,tempo_mediana_ms,ticks_relogio,cv_mad_pct,"
        "comparacoes_sobre_nlogn,razao_sobre_teoria,memoria_entrada_kb");

    std::printf("  %9s %13s %9s %9s %14s %9s\n",
                "n", "mediana(ms)", "ticks", "CV_MAD%", "comp/(n log2 n)", "vs teo");
    for (int n : tamanhos) {
        std::vector<Medida> medidas;
        std::vector<double> razoes;
        for (int s = 0; s < sementes; ++s) {
            const auto entrada = gerarMassa(TipoMassa::ALEATORIO, n,
                                            semente(n, s));
            // Aquecimento local por (n, semente).
            for (int w = 0; w < 2; ++w) {
                std::vector<int> v = entrada;
                Contadores c;
                quicksort(v, c, 1, EstrategiaPivo::MEIO);
            }
            // A razao assintotica e medida no recursivo puro (M=1): a versao
            // hibrida tem o Insertion Sort dominando em n pequeno e deslocaria
            // a razao. O tempo, porem, e medido na versao hibrida (M), que e a
            // usada de fato na bateria principal.
            {
                std::vector<int> v = entrada;
                Contadores c;
                quicksort(v, c, 1, EstrategiaPivo::MEIO);
                razoes.push_back(static_cast<double>(c.comparacoes)
                                 / (n * std::log2(static_cast<double>(n))));
            }
            for (int r = 0; r < reps; ++r) {
                std::vector<int> v = entrada;
                Contadores c;
                const auto t0 = Relogio::now();
                quicksort(v, c, M, EstrategiaPivo::MEIO);
                const auto t1 = Relogio::now();
                Medida m;
                m.tempo_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
                m.comparacoes = c.comparacoes;
                m.trocas = c.trocas;
                medidas.push_back(m);
            }
        }

        const Resumo r = resumir(medidas);
        double razao = 0.0;
        for (double x : razoes) razao += x;
        razao /= razoes.size();
        const double teoria = 2.0 * std::log(2.0);
        const double ticks = r.tempo_mediana_ms / resolucao_ms;
        const double cv_mad = (r.tempo_mediana_ms > 0.0)
            ? 100.0 * r.tempo_mad_ms / r.tempo_mediana_ms : 0.0;
        const double memoria_kb = static_cast<double>(n) * sizeof(int) / 1024.0;

        csv << n << ',' << r.tempo_mediana_ms << ',' << ticks << ','
            << cv_mad << ',' << razao << ',' << razao / teoria << ','
            << memoria_kb << '\n';
        std::printf("  %9d %13.4f %9.0f %9.2f %14.4f %9.3f\n",
                    n, r.tempo_mediana_ms, ticks, cv_mad, razao, razao / teoria);
    }
    std::cout << "\n  Leitura: abaixo de n~1000 a medicao dura poucos ticks do\n"
                 "  relogio e a dispersao cresce (piso de ruido). A razao\n"
                 "  comp/(n log2 n) estabiliza muito antes, indicando que o teto\n"
                 "  ja esta no regime assintotico. Ver a secao de metodologia.\n\n";
}

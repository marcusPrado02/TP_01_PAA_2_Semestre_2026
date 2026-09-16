#include "geradores.hpp"

#include <algorithm>
#include <numeric>
#include <random>

const std::vector<TipoMassa>& todasAsMassas() {
    static const std::vector<TipoMassa> massas = {
        TipoMassa::ALEATORIO,
        TipoMassa::ORDENADO,
        TipoMassa::INVERSO,
        TipoMassa::REPETIDOS,
        TipoMassa::QUASE_ORDENADO,
    };
    return massas;
}

const char* nomeMassa(TipoMassa t) {
    switch (t) {
        case TipoMassa::ALEATORIO:      return "aleatorio";
        case TipoMassa::ORDENADO:       return "ordenado";
        case TipoMassa::INVERSO:        return "inverso";
        case TipoMassa::REPETIDOS:      return "repetidos";
        case TipoMassa::QUASE_ORDENADO: return "quase_ordenado";
    }
    return "?";
}

std::vector<int> gerarMassa(TipoMassa t, int n, unsigned semente) {
    std::vector<int> v(static_cast<std::size_t>(n));
    std::mt19937 rng(semente);

    switch (t) {
        case TipoMassa::ALEATORIO: {
            std::uniform_int_distribution<int> dist(0, 10 * n);
            for (int& x : v) x = dist(rng);
            break;
        }
        case TipoMassa::ORDENADO: {
            std::iota(v.begin(), v.end(), 0);
            break;
        }
        case TipoMassa::INVERSO: {
            std::iota(v.rbegin(), v.rend(), 0);
            break;
        }
        case TipoMassa::REPETIDOS: {
            std::uniform_int_distribution<int> dist(0, NUM_VALORES_REPETIDOS - 1);
            for (int& x : v) x = dist(rng);
            break;
        }
        case TipoMassa::QUASE_ORDENADO: {
            std::iota(v.begin(), v.end(), 0);
            const int perturbacoes = std::max(1, n / 100);   // ~1% de desordem
            std::uniform_int_distribution<int> dist(0, n - 1);
            for (int k = 0; k < perturbacoes; ++k) {
                std::swap(v[dist(rng)], v[dist(rng)]);
            }
            break;
        }
    }
    return v;
}

#!/usr/bin/env python3
"""
Gera os graficos do relatorio a partir dos CSVs produzidos por ./bin/quicksort.

Uso:  python3 scripts/graficos.py        (a partir da raiz do projeto)
Saida: graficos/*.png  +  graficos/tabelas.tex (tabelas prontas para o Overleaf)
"""

from pathlib import Path

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import pandas as pd

RAIZ = Path(__file__).resolve().parent.parent
ENTRADA = RAIZ / "resultados"
SAIDA = RAIZ / "graficos"

# Rotulos legiveis e estilo fixo por algoritmo (mesma cor em todos os graficos).
# A mediana-de-tres usa linha tracejada porque suas curvas praticamente coincidem
# com as do hibrido simples em varias massas; o tracejado evita que uma serie
# cubra a outra (overdraw) e torna as duas distinguiveis.
ESTILO = {
    "quicksort_recursivo":     ("Quicksort recursivo",        "#1f77b4", "o", "-"),
    "quicksort_hibrido":       ("Quicksort hibrido",          "#d62728", "s", "-"),
    "quicksort_mediana3":      ("Hibrido + mediana-de-tres",  "#2ca02c", "^", "--"),
    "quicksort_pivo_primeiro": ("Pivo = primeiro (pior caso)", "#9467bd", "D", ":"),
}
NOME_MASSA = {
    "aleatorio": "Aleatorio",
    "ordenado": "Ordenado",
    "inverso": "Inversamente ordenado",
    "repetidos": "Muitos elementos repetidos",
    "quase_ordenado": "Quase ordenado",
}
# Cor fixa por massa, usada nos graficos de calibracao (o eixo x e M, nao n).
COR_MASSA = {
    "aleatorio": "#1f77b4",
    "ordenado": "#2ca02c",
    "repetidos": "#d62728",
    "inverso": "#9467bd",
    "quase_ordenado": "#ff7f0e",
}


def salvar(fig, nome):
    SAIDA.mkdir(exist_ok=True)
    caminho = SAIDA / nome
    fig.savefig(caminho, dpi=160, bbox_inches="tight")
    plt.close(fig)
    print(f"  -> {caminho.relative_to(RAIZ)}")


def plotar_series(ax, dados, coluna, log=True):
    """Desenha uma curva por algoritmo, em funcao de n."""
    for alg, grupo in dados.groupby("algoritmo", sort=False):
        if alg not in ESTILO:
            continue
        rotulo, cor, marca, traco = ESTILO[alg]
        grupo = grupo.sort_values("n")
        ax.plot(grupo["n"], grupo[coluna], label=rotulo, color=cor,
                marker=marca, linestyle=traco, linewidth=1.6, markersize=5)
    if log:
        ax.set_xscale("log")
        ax.set_yscale("log")
    ax.set_xlabel("n (numero de elementos)")
    ax.grid(True, which="both", alpha=0.25, linewidth=0.5)


# ---------------------------------------------------------------------------
# 1. Calibracao de M
# ---------------------------------------------------------------------------
def grafico_calibracao():
    arq = ENTRADA / "calibracao_m.csv"
    if not arq.exists():
        print("  (calibracao_m.csv ausente - execute './bin/quicksort calibrar')")
        return
    df = pd.read_csv(arq)

    for metrica, coluna, unidade in [
        ("tempo", "tempo_mediana_ms", "tempo mediano (ms)"),
        ("comparacoes", "comparacoes_medias", "comparacoes medias"),
    ]:
        tamanhos = sorted(df["n"].unique())
        fig, eixos = plt.subplots(1, len(tamanhos), figsize=(5 * len(tamanhos), 4))
        if len(tamanhos) == 1:
            eixos = [eixos]
        for ax, n in zip(eixos, tamanhos):
            sub = df[(df["n"] == n) & (df["pivo"] == "meio")]
            for massa, grupo in sub.groupby("massa"):
                grupo = grupo.sort_values("M")
                ax.plot(grupo["M"], grupo[coluna], marker="o", markersize=4,
                        linewidth=1.5, color=COR_MASSA.get(massa),
                        label=NOME_MASSA.get(massa, massa))
            ax.set_title(f"n = {n}")
            ax.set_xlabel("M (corte para Insertion Sort)")
            ax.set_ylabel(unidade)
            ax.set_xscale("log")
            ax.grid(True, which="both", alpha=0.25, linewidth=0.5)
            ax.legend(fontsize=8)
        fig.suptitle(f"Calibracao empirica de M - {unidade} (pivo central)")
        salvar(fig, f"calibracao_m_{metrica}.png")


# ---------------------------------------------------------------------------
# 2. Experimentos principais
# ---------------------------------------------------------------------------
def graficos_principais():
    arq = ENTRADA / "experimentos_resumo.csv"
    if not arq.exists():
        print("  (experimentos_resumo.csv ausente - execute './bin/quicksort experimentos')")
        return
    df = pd.read_csv(arq)
    massas = [m for m in NOME_MASSA if m in set(df["massa"])]

    for metrica, coluna, unidade in [
        ("tempo", "tempo_mediana_ms", "tempo mediano (ms)"),
        ("comparacoes", "comparacoes_medias", "comparacoes"),
        ("trocas", "trocas_medias", "trocas / movimentacoes"),
    ]:
        fig, eixos = plt.subplots(1, len(massas), figsize=(4.2 * len(massas), 3.8),
                                  sharey=True)
        if len(massas) == 1:
            eixos = [eixos]
        for ax, massa in zip(eixos, massas):
            plotar_series(ax, df[df["massa"] == massa], coluna)
            ax.set_title(NOME_MASSA[massa], fontsize=10)
        eixos[0].set_ylabel(unidade)
        eixos[-1].legend(fontsize=8)
        fig.suptitle(f"Comparacao das tres versoes do Quicksort - {unidade}")
        salvar(fig, f"principal_{metrica}.png")

    # Ganho relativo das versoes hibridas sobre a recursiva pura.
    # Uma faceta por massa, eixo x = n (log): le a tendencia ao longo de n sem
    # os 20 rotulos rotacionados de um eixo categorico.
    base = df[df["algoritmo"] == "quicksort_recursivo"].set_index(["massa", "n"])
    fig, eixos = plt.subplots(1, len(massas), figsize=(4.2 * len(massas), 3.8),
                              sharey=True)
    if len(massas) == 1:
        eixos = [eixos]
    for ax, massa in zip(eixos, massas):
        base_m = df[(df["algoritmo"] == "quicksort_recursivo") & (df["massa"] == massa)]
        base_m = base_m.set_index("n")["tempo_mediana_ms"]
        for alg in ["quicksort_hibrido", "quicksort_mediana3"]:
            sub = df[(df["algoritmo"] == alg) & (df["massa"] == massa)].sort_values("n")
            ganho = (1 - sub["tempo_mediana_ms"].values / base_m.loc[sub["n"]].values) * 100
            rotulo, cor, marca, traco = ESTILO[alg]
            ax.plot(sub["n"], ganho, marker=marca, color=cor, linestyle=traco,
                    linewidth=1.5, markersize=5, label=rotulo)
        ax.axhline(0, color="black", linewidth=0.8)
        ax.set_xscale("log")
        ax.set_title(NOME_MASSA[massa], fontsize=10)
        ax.grid(True, which="both", alpha=0.25, linewidth=0.5)
    eixos[0].set_ylabel("reducao de tempo vs. recursivo puro (%)")
    eixos[-1].legend(fontsize=8)
    fig.suptitle("Ganho das versoes hibridas sobre o Quicksort recursivo")
    salvar(fig, "ganho_relativo.png")


# ---------------------------------------------------------------------------
# 3. Pior caso
# ---------------------------------------------------------------------------
def graficos_pior_caso():
    arq = ENTRADA / "pior_caso.csv"
    if not arq.exists():
        print("  (pior_caso.csv ausente - execute './bin/quicksort pior-caso')")
        return
    df = pd.read_csv(arq)
    sub = df[df["massa"] == "ordenado"]

    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(11, 4.2))
    plotar_series(ax1, sub, "comparacoes_medias")
    ax1.set_ylabel("comparacoes")
    ax1.set_title("Pior caso: comparacoes (vetor ordenado)")
    ax1.legend(fontsize=8)

    # comparacoes / n^2 constante  =>  crescimento quadratico confirmado.
    for alg, grupo in sub.groupby("algoritmo", sort=False):
        if alg not in ESTILO:
            continue
        rotulo, cor, marca, traco = ESTILO[alg]
        grupo = grupo.sort_values("n")
        ax2.plot(grupo["n"], grupo["comparacoes_sobre_n2"], label=rotulo,
                 color=cor, marker=marca, linestyle=traco, markersize=5)
    ax2.axhline(0.5, color="grey", linestyle=":", linewidth=1.2,
                label="limite teorico n$^2$/2")
    ax2.set_xscale("log")
    ax2.set_yscale("log")
    ax2.set_xlabel("n (numero de elementos)")
    ax2.set_ylabel("comparacoes / n$^2$")
    ax2.set_title("Razao comparacoes/n$^2$ (constante => $\\Theta(n^2)$)")
    ax2.grid(True, which="both", alpha=0.25, linewidth=0.5)
    ax2.legend(fontsize=8)
    salvar(fig, "pior_caso.png")


# ---------------------------------------------------------------------------
# 4. Tabelas LaTeX prontas para o relatorio
# ---------------------------------------------------------------------------
def tabelas_latex():
    arq = ENTRADA / "experimentos_resumo.csv"
    if not arq.exists():
        return
    df = pd.read_csv(arq)
    SAIDA.mkdir(exist_ok=True)
    destino = SAIDA / "tabelas.tex"

    with destino.open("w") as f:
        f.write("% Gerado automaticamente por scripts/graficos.py\n")
        for massa in NOME_MASSA:
            sub = df[df["massa"] == massa]
            if sub.empty:
                continue
            f.write("\n\\begin{table}[H]\n\\centering\n")
            f.write(f"\\caption{{Resultados para massa {NOME_MASSA[massa]}.}}\n")
            f.write(f"\\label{{tab:{massa}}}\n")
            f.write("\\begin{tabular}{lrrrr}\n\\hline\n")
            f.write("Algoritmo & $n$ & Tempo (ms) & Comparacoes & Trocas \\\\\n\\hline\n")
            for _, r in sub.iterrows():
                rotulo = ESTILO.get(r["algoritmo"], (r["algoritmo"],))[0]
                f.write(f"{rotulo} & {int(r['n'])} & {r['tempo_mediana_ms']:.3f} & "
                        f"{r['comparacoes_medias']:.0f} & {r['trocas_medias']:.0f} \\\\\n")
            f.write("\\hline\n\\end{tabular}\n\\end{table}\n")
    print(f"  -> {destino.relative_to(RAIZ)}")


if __name__ == "__main__":
    print("Gerando graficos e tabelas:")
    grafico_calibracao()
    graficos_principais()
    graficos_pior_caso()
    tabelas_latex()
    print("Concluido.")

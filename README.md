# TP 01 — PAA — Estudo comparativo de implementações do Quicksort

Trabalho Prático da disciplina de **Projeto e Análise de Algoritmos** (PUC Minas,
Prof. Walisson Ferreira de Carvalho), 2º semestre de 2026.

Baseado no **exercício 15 do capítulo 4** de ZIVIANI, N. *Projeto de Algoritmos:
com implementações em Pascal e C*. 3. ed. São Paulo: Cengage Learning, 2018.

O objetivo é comparar experimentalmente três versões do Quicksort:

| # | Versão | Implementação |
|---|--------|---------------|
| a | Quicksort recursivo | `quicksortRecursivo()` — recursão até subvetores unitários |
| b | Quicksort híbrido | `quicksortHibrido()` — partição interrompida para subvetores com menos de `M` elementos, ordenados por Insertion Sort |
| c | Híbrido + mediana-de-três | `quicksortHibridoMediana3()` — igual à (b), com pivô = mediana de `v[esq]`, `v[meio]`, `v[dir]` |

---

## Compilação e execução

Requer apenas um compilador C++17 (testado com g++ 13.3) e `make`.

```bash
make                      # compila em bin/quicksort
make validar              # verifica a corretude das três versões
make calibrar             # busca empírica do melhor M
make experimentos         # bateria principal
make pior-caso            # experimento de pior caso forçado
make tudo                 # executa tudo (≈ 20 s)
make graficos             # gera gráficos e tabelas LaTeX (requer matplotlib/pandas)
make limpar
```

O parâmetro `M` pode ser sobrescrito: `make experimentos M=25`.

Os resultados são gravados em CSV no diretório `resultados/`; os gráficos e as
tabelas prontas para o Overleaf, em `graficos/`.

---

## Organização do código

```
src/
  contadores.hpp    struct Contadores (comparações e trocas)
  ordenacao.hpp/cpp Insertion Sort, partição de Hoare e as três versões do Quicksort
  geradores.hpp/cpp geração determinística das massas de teste
  experimentos.hpp/cpp  medição, agregação estatística e as rotinas de experimento
  main.cpp          interface de linha de comando
scripts/graficos.py gráficos (matplotlib) e tabelas LaTeX a partir dos CSVs
resultados/         saídas em CSV
graficos/           figuras .png e tabelas.tex
```

### Um único motor para as três versões

As três versões diferem em apenas dois eixos: **onde a recursão para** (`M`) e
**como o pivô é escolhido** (`EstrategiaPivo`). Por isso existe um único motor,
`quicksort(v, contadores, M, pivo)`; as três funções pedidas pelo enunciado são
atalhos com esses parâmetros fixos.

Isso não é apenas economia de código: garante que as três variantes contem
comparações e trocas **exatamente da mesma forma**, condição necessária para que
a comparação experimental seja válida. Se cada versão tivesse seu próprio laço,
qualquer diferença de contagem seria indistinguível de uma diferença de
desempenho.

### Partição

Usa-se a partição de **Hoare** na forma apresentada por Ziviani (dois ponteiros
que caminham em sentidos opostos até se cruzarem, com o pivô tratado como
*valor*, não como índice). A terminação dos laços internos é garantida porque o
valor do pivô é sempre retirado do próprio subvetor, funcionando como sentinela.

Essa escolha importa para o teste de **muitos elementos repetidos**: com todos os
elementos iguais, os dois ponteiros avançam simetricamente e a partição fica
balanceada. A partição de Lomuto, na mesma entrada, degeneraria para O(n²).

---

## Convenções de contagem

Documentadas em `src/contadores.hpp` e repetidas aqui porque afetam a leitura de
todos os resultados:

- **Comparações**: toda comparação entre *chaves* do vetor. Comparações de
  índices (`i <= j`, `j >= esq`) não são contadas — são controle de laço e não
  dependem dos dados.
- **Trocas**: toda *movimentação* de chave dentro do vetor. No Quicksort, cada
  `swap` da partição conta 1; no Insertion Sort, cada deslocamento
  `v[j+1] = v[j]` conta 1. Assim as duas grandezas ficam na mesma unidade
  ("quantas vezes um elemento mudou de lugar") e podem ser somadas nas versões
  híbridas sem distorção.

---

## Massas de teste

Geradas de forma **determinística** a partir de `(tipo, n, semente)`: todas as
versões são medidas sobre exatamente os mesmos dados, e qualquer execução é
reproduzível.

| Massa | Descrição |
|-------|-----------|
| `aleatorio` | chaves uniformes em `[0, 10n)` — caso médio |
| `ordenado` | `0, 1, …, n-1` — melhor caso para pivô central; **pior caso** para pivô = primeiro |
| `inverso` | `n-1, …, 1, 0` |
| `repetidos` | apenas 10 chaves distintas — estressa o tratamento de chaves iguais |
| `quase_ordenado` | ordenado com ~1% dos elementos trocados de posição |

Tamanhos: `n ∈ {1000, 10000, 100000, 500000}`.

---

## Metodologia de medição

O enunciado exige medir o tempo de relógio e repetir as execuções para obter
médias confiáveis. Duas decisões adicionais foram necessárias e estão
documentadas aqui porque **alteram as conclusões**:

### 1. Fase de aquecimento descartada

Em testes preliminares, a **mesma carga** mediu **16,2 ms** na primeira execução
do processo e **5,3 ms** poucos segundos depois — variação de 3× causada pelo
governador de frequência da CPU, não pelo algoritmo. Sem tratar isso, a
calibração indicava `M = 100` como ótimo, enquanto o número de comparações
(métrica determinística) apontava `M ≈ 20`. O "ótimo" era artefato de
aquecimento.

Repetir e tirar média **não** corrige esse viés: ele é sistemático, não
aleatório. Por isso o programa executa `aquecerCPU()` antes de qualquer medição
e descarta uma execução por configuração.

### 2. Mediana em vez de média

O ruído de medição (escalonador do SO, migração entre núcleos) só pode
*acrescentar* tempo, nunca subtrair. A média é puxada por outliers; a mediana e
o mínimo são estimadores mais fiéis do custo real. Os CSVs registram
**mediana, média, mínimo e desvio-padrão** — a análise usa a mediana, e a média
e o desvio ficam disponíveis para documentar a dispersão.

---

## Calibração empírica de M

`./bin/quicksort calibrar` varre `M ∈ {1, 2, …, 200}` para `n ∈ {1000, 10000,
100000}` sobre três massas, com repetições adaptativas (mais repetições para `n`
pequeno, onde os transientes pesam mais).

O critério de recomendação **não** é o `argmin` puro do tempo: perto do ótimo a
curva é praticamente plana, e o `argmin` passa a escolher com base em ruído (em
execuções preliminares oscilou entre 12 e 100 sem diferença real). O critério
adotado é **o menor M cujo tempo fica dentro de 2% do melhor tempo observado** —
reproduzível, e mantém o Insertion Sort restrito a subvetores pequenos.

Resultado (reproduzível entre execuções):

- **M recomendado ≈ 30–40**, estável para os três tamanhos e para as duas
  estratégias de pivô;
- `argmin` por comparações: **M ≈ 20–25**;
- o platô de bom desempenho se estende de ~20 a ~70.

O valor adotado como padrão no código é **M = 40**.

Observe que esse valor é maior que os 5–25 usualmente citados na literatura
clássica. Isso é esperado em hardware moderno: o Insertion Sort sobre blocos
pequenos tem localidade de cache e previsibilidade de desvios excelentes, e as
chaves aqui são `int` (movimentação barata), o que desloca o ponto de equilíbrio
para cima.

---

## Pior caso

`./bin/quicksort pior-caso` força explicitamente o pior caso combinando **vetor
já ordenado** com **escolha inadequada de pivô** (`v[esq]`). A partição resulta
em subvetores de tamanho 0 e n−1, e a recursão tem profundidade n.

A razão `comparações / n²` converge para **exatamente 0,5000**, confirmando
numericamente o limite teórico de n²/2 comparações:

| n | comparações | comparações / n² | tempo (ms) |
|---|-------------|------------------|------------|
| 1 000 | 501 498 | 0,5015 | 0,14 |
| 10 000 | 50 014 998 | 0,5001 | 13,2 |
| 50 000 | 1 250 074 998 | 0,5000 | 334,4 |

Para comparação, na mesma entrada de n = 50 000 a versão com mediana-de-três faz
**600 010** comparações (0,198 ms) — cerca de **2 000×** menos comparações e
**1 700×** mais rápida.

> **Limite de tamanho.** Os tamanhos do experimento de pior caso vão até
> n = 50 000 porque a profundidade da recursão é n; com a pilha padrão de 8 MB,
> valores maiores causariam estouro de pilha. Essa é justamente a fragilidade
> que as versões (b) e (c) corrigem.

---

## Resultados principais (resumo)

Máquina de teste: g++ 13.3, `-O2`, Linux x86-64.

- As versões **híbridas reduzem o tempo entre 16% e 45%** (mediana 31%) frente
  ao recursivo puro, de forma consistente em todas as massas e tamanhos.
- Comparando **mediana-de-três contra híbrido simples com o mesmo M** (única
  forma de isolar o efeito do pivô), a mediana-de-três reduz as comparações em
  **4% a 9% em dados aleatórios**, mas fica **~1% pior** em vetores ordenados,
  inversos e quase-ordenados.
- Esse resultado é explicado pela teoria: num vetor ordenado o elemento central
  **já é a mediana exata** do subvetor, então a mediana-de-três escolhe o mesmo
  pivô e ainda gasta 2–3 comparações extras por chamada. Em dados aleatórios o
  elemento central é um elemento qualquer, e a mediana-de-três estima melhor a
  mediana real.
- O ganho decisivo da mediana-de-três **não é sobre o pivô central**, e sim
  sobre pivôs de **posição fixa** (primeiro/último elemento): no experimento de
  pior caso a diferença chega a **2 000×** em número de comparações.

> **Atenção ao comparar versões.** Confrontar `mediana3 (M=40)` com
> `recursivo (M=1)` mistura duas variáveis (o corte e o pivô) e superestima o
> efeito da mediana-de-três. Para isolar o pivô é preciso manter M igual dos
> dois lados — é justamente por isso que existe um motor único parametrizado.

Os números completos estão em `resultados/experimentos_resumo.csv` e nas figuras
de `graficos/`.

---

## Arquivos de saída

| Arquivo | Conteúdo |
|---------|----------|
| `resultados/calibracao_m.csv` | varredura de M (tempo, comparações, trocas) |
| `resultados/experimentos_bruto.csv` | uma linha por repetição individual |
| `resultados/experimentos_resumo.csv` | estatísticas agregadas por configuração |
| `resultados/pior_caso.csv` | pior caso forçado, com a razão comparações/n² |
| `graficos/*.png` | figuras para o relatório |
| `graficos/tabelas.tex` | tabelas prontas para colar no Overleaf |

---

## Referências

ZIVIANI, N. **Projeto de Algoritmos: com implementações em Pascal e C**. 3. ed.
São Paulo: Cengage Learning, 2018.

CORMEN, T. H. et al. **Introduction to Algorithms**. 4. ed. Cambridge: MIT
Press, 2022.

HOARE, C. A. R. Quicksort. **The Computer Journal**, v. 5, n. 1, p. 10–16, 1962.

SEDGEWICK, R. Implementing Quicksort programs. **Communications of the ACM**,
v. 21, n. 10, p. 847–857, 1978.

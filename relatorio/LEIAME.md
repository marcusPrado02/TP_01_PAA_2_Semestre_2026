# Relatório — TP Quicksort (LaTeX / ABNT PUC Minas)

## Estrutura

```
Projeto.tex              documento principal (identificação, pacotes, includes)
pre-texto/resumo.tex     resumo + palavras-chave
pre-texto/abstract.tex   abstract + keywords
pre-texto/lista-siglas.tex
texto/introducao.tex     Cap. 1 — Introdução
texto/referencial.tex    Cap. 2 — Referencial Teórico
texto/metodologia.tex    Cap. 3 — Metodologia
texto/resultados.tex     Cap. 4 — Resultados e Análise
texto/conclusao.tex      Cap. 5 — Conclusão
pos-texto/apendiceA.tex  Apêndice — código-fonte
Referencias.bib          16 referências (10 clássicas + 6 recentes)
imagem/                  figuras geradas por scripts/graficos.py
```

## Compilar

**Localmente:**
```bash
./compilar.sh
```

**No Overleaf:** suba o ZIP e defina `Projeto.tex` como documento principal
(Menu → Main document). O compilador deve ser **pdfLaTeX**.

## Regenerar as figuras

As figuras em `imagem/` são geradas a partir dos CSVs do programa em C++:

```bash
cd ..            # raiz do projeto
make tudo        # roda os experimentos, gera resultados/*.csv
make graficos    # gera graficos/*.png
cp graficos/*.png relatorio/imagem/
```

## Pendências antes da entrega

- [ ] Acrescentar os demais integrantes do grupo em `\autor{}` (Projeto.tex, linha ~170)
- [ ] Conferir o nome do curso em `\instituicao{}` (atualmente "Engenharia de Software")
- [ ] Ajustar `\setcounter{page}{1}` se a contagem de páginas pré-textuais mudar

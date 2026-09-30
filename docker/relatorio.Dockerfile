# Etapa LaTeX: compila o relatorio (relatorio/compilar.sh) sem instalar TeX no host.
# Base enxuta + pacotes via apt em vez da imagem oficial texlive (~5 GB), que nao
# caberia com folga no disco desta maquina.
FROM debian:bookworm-slim

# Colecao de pacotes usados por relatorio/Projeto.tex: abnt (classe/sty locais no
# repo), acronym, multirow, geometry, xy, algorithm2e, algorithmic, listings,
# setspace, rotating, color, array, longtable, colortbl, ifthen, caption, paralist,
# subfig, footmisc, amsmath/amssymb, url, float, placeins, graphicx.
RUN apt-get update && apt-get install -y --no-install-recommends \
        texlive-latex-base \
        texlive-latex-recommended \
        texlive-latex-extra \
        texlive-plain-generic \
        texlive-science \
        texlive-pictures \
        texlive-fonts-recommended \
        texlive-lang-portuguese \
        lmodern \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# O script faz cd para o proprio diretorio, roda pdflatex/bibtex e imprime o PDF.
ENTRYPOINT ["bash", "relatorio/compilar.sh"]

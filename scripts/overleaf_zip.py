#!/usr/bin/env python3
"""
Empacota o relatorio em um ZIP pronto para upload no Overleaf.

O diretorio relatorio/ e autocontido (classe ABNT, .sty/.bst vendorizados,
Referencias.bib e as figuras em imagem/), entao o ZIP gerado compila no
Overleaf sem nenhuma configuracao alem de definir Projeto.tex como documento
principal e o compilador pdfLaTeX.

Artefatos intermediarios do LaTeX (.aux, .log, .toc, ...) ficam de fora.

Uso:  python3 scripts/overleaf_zip.py        (a partir da raiz do projeto)
Saida: relatorio-overleaf.zip
"""

import sys
import zipfile
from pathlib import Path

RAIZ = Path(__file__).resolve().parent.parent
ORIGEM = RAIZ / "relatorio"
DESTINO = RAIZ / "relatorio-overleaf.zip"

# Mesmas extensoes ignoradas por relatorio/.gitignore.
EXCLUIR = (
    ".aux", ".bbl", ".blg", ".log", ".out", ".toc", ".lof", ".lot",
    ".loq", ".synctex.gz", ".fls", ".fdb_latexmk",
)


def main() -> int:
    if not (ORIGEM / "Projeto.tex").is_file():
        print(f"ERRO: {ORIGEM / 'Projeto.tex'} nao encontrado.", file=sys.stderr)
        return 1

    arquivos = []
    with zipfile.ZipFile(DESTINO, "w", zipfile.ZIP_DEFLATED) as z:
        for caminho in sorted(ORIGEM.rglob("*")):
            if caminho.is_dir() or caminho.name.endswith(EXCLUIR):
                continue
            z.write(caminho, caminho.relative_to(ORIGEM))
            arquivos.append(caminho)

    tamanho_mb = DESTINO.stat().st_size / 1e6
    print(f"OK: {DESTINO.name} ({len(arquivos)} arquivos, {tamanho_mb:.1f} MB)")
    print("Overleaf: New Project -> Upload Project; Main document = Projeto.tex; pdfLaTeX.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

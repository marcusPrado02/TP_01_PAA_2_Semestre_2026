# Etapa Python: gera graficos e tabelas LaTeX a partir dos CSVs de resultados/.
# Versoes fixadas para reprodutibilidade das figuras.
FROM python:3.12-slim

WORKDIR /app

RUN pip install --no-cache-dir matplotlib==3.9.2 pandas==2.2.3

# O usuario do host pode nao ter HOME gravavel dentro do container; aponta o
# cache do matplotlib para /tmp para evitar o aviso na importacao.
ENV MPLCONFIGDIR=/tmp/matplotlib

# ENTRYPOINT fixo no script de graficos; o repositorio e montado via bind mount.
ENTRYPOINT ["python3", "scripts/graficos.py"]

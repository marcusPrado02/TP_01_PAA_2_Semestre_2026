# Empacota o relatorio em um ZIP pronto para upload no Overleaf.
# Usa python:3.12-slim (sem matplotlib/pandas: o script so usa a stdlib).
FROM python:3.12-slim

WORKDIR /app

# ENTRYPOINT fixo no script; o repositorio e montado via bind mount.
ENTRYPOINT ["python3", "scripts/overleaf_zip.py"]

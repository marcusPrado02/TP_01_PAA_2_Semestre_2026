# Etapa C++: compila e executa os experimentos (validar/calibrar/experimentos/pior-caso).
# Nao copia o codigo: o repositorio e montado via bind mount em /app pelo compose,
# para que os CSVs caiam no diretorio resultados/ do host.
FROM gcc:13

WORKDIR /app

# ENTRYPOINT fixo em make: os argumentos passados ao container viram alvos do
# Makefile, ex.: `docker compose run --rm experimentos tudo` -> `make tudo`.
ENTRYPOINT ["make"]
CMD ["tudo"]

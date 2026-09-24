# Trabalho Pratico de PAA - Quicksort
CXX      ?= g++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -pedantic
SRCDIR    = src
BINDIR    = bin
ALVO      = $(BINDIR)/quicksort
FONTES    = $(wildcard $(SRCDIR)/*.cpp)
OBJETOS   = $(FONTES:$(SRCDIR)/%.cpp=$(BINDIR)/%.o)

.PHONY: all validar calibrar experimentos pior-caso tudo graficos overleaf-zip limpar \
        docker-build docker-tudo docker-validar docker-calibrar \
        docker-experimentos docker-pior-caso docker-graficos docker-relatorio \
        docker-overleaf docker-up

all: $(ALVO)

$(ALVO): $(OBJETOS) | $(BINDIR)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(BINDIR)/%.o: $(SRCDIR)/%.cpp | $(BINDIR)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(BINDIR):
	mkdir -p $(BINDIR)

resultados:
	mkdir -p resultados

validar: $(ALVO)
	./$(ALVO) validar

calibrar: $(ALVO) resultados
	./$(ALVO) calibrar

experimentos: $(ALVO) resultados
	./$(ALVO) experimentos $(M)

pior-caso: $(ALVO) resultados
	./$(ALVO) pior-caso $(M)

tudo: $(ALVO) resultados
	./$(ALVO) tudo $(M)

graficos:
	python3 scripts/graficos.py

# Empacota o relatorio em relatorio-overleaf.zip para upload no Overleaf.
overleaf-zip:
	python3 scripts/overleaf_zip.py

limpar:
	rm -rf $(BINDIR)

# ---------------------------------------------------------------------------
# Alvos com Docker (nao requerem g++/make/python/matplotlib no host)
# ---------------------------------------------------------------------------

# Constroi as duas imagens (experimentos em C++ e graficos em Python).
docker-build:
	docker compose build

# Executa a bateria completa dentro do container e gera os graficos em seguida.
docker-tudo:
	docker compose run --rm experimentos tudo M=$(M)
	docker compose run --rm --no-deps graficos

# Etapas individuais da parte C++.
docker-validar:
	docker compose run --rm experimentos validar

docker-calibrar:
	docker compose run --rm experimentos calibrar

docker-experimentos:
	docker compose run --rm experimentos experimentos M=$(M)

docker-pior-caso:
	docker compose run --rm experimentos pior-caso M=$(M)

# Gera apenas os graficos (requer resultados/*.csv ja existentes).
docker-graficos:
	docker compose run --rm --no-deps graficos

# Compila o relatorio (relatorio/Projeto.pdf) com pdflatex + bibtex.
docker-relatorio:
	docker compose run --rm --no-deps relatorio

# Gera o ZIP para upload no Overleaf (relatorio-overleaf.zip).
docker-overleaf:
	docker compose run --rm --no-deps overleaf

# Sobe os dois servicos em sequencia, respeitando o depends_on.
docker-up:
	docker compose up --abort-on-container-exit

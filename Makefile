# Trabalho Pratico de PAA - Quicksort
CXX      ?= g++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -pedantic
SRCDIR    = src
BINDIR    = bin
ALVO      = $(BINDIR)/quicksort
FONTES    = $(wildcard $(SRCDIR)/*.cpp)
OBJETOS   = $(FONTES:$(SRCDIR)/%.cpp=$(BINDIR)/%.o)

.PHONY: all validar calibrar experimentos pior-caso tudo graficos limpar

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

limpar:
	rm -rf $(BINDIR)

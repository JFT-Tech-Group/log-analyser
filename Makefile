# Makefile pour log-analyser
# Structure attendue :
#  include/*.h
#  src/*.c

# -- outils & flags
CC      := gcc
CFLAGS  := -std=gnu11 -Wall -Wextra -Wshadow -g -Iinclude
LDFLAGS :=
LDLIBS := -lsystemd -lncursesw

# -- répertoires et cibles
SRCDIR := src
OBJDIR := build
BINDIR := bin
TARGET := log-analyser

# -- fichiers sources / objets / dépendances
SRCS := $(wildcard $(SRCDIR)/*.c)
OBJS := $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRCS))
DEPS := $(OBJS:.o=.d)

.PHONY: all clean rebuild run

# cible par défaut
all: $(BINDIR)/$(TARGET)

# linkage : créer le binaire dans bin/
$(BINDIR)/$(TARGET): $(OBJS) | $(BINDIR)
	$(CC) $(LDFLAGS) $^ -o $@ $(LDLIBS)

# règle de compilation : génère .o et .d (dépendances)
# -MMD -MP -MF ... : crée un fichier de dépendances .d pour chaque .o
$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -MMD -MP -MF $(@:.o=.d) -c $< -o $@

# créer les répertoires si nécessaire
$(OBJDIR) $(BINDIR):
	mkdir -p $@

# inclure automatiquement les fichiers .d générés (silencieusement si manquants)
-include $(DEPS)

# utilitaires
clean:
	rm -rf $(OBJDIR) $(BINDIR)

rebuild: clean all

run: all
	./$(BINDIR)/$(TARGET)

# debug/help : afficher une variable (ex: make print-CFLAGS)
print-%:
	@echo '$*=$($*)'
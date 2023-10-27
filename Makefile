# Couleurs pour l'affichage
RED = \033[31m
GREEN = \033[32m
YELLOW = \033[33m
YELLOW = \033[33m
BOLD = \033[1m
NC = \033[0m # Pas de couleur

# Compilateur
CC = gcc

# Dossiers
SRV_DIR = Serveur
CLI_DIR = Client

# Sources et objets
SRV_SOURCES = $(SRV_DIR)/server2.c $(SRV_DIR)/awale.c
CLI_SOURCE = $(CLI_DIR)/client2.c
SRV_OBJECTS = $(SRV_SOURCES:.c=.o)
CLI_OBJECT = $(CLI_SOURCE:.c=.o)

# Exécutables
SRV_BIN = server_bin
CLI_BIN = client_bin

all: $(SRV_BIN) $(CLI_BIN)

$(SRV_BIN): $(SRV_OBJECTS)
	@echo "$(YELLOW)Génération de $@...$(NC)"
	$(CC) -o $@ $^
	@echo "$(GREEN)$(BOLD)$@ a été généré avec succès !$(NC)\r\n"

$(CLI_BIN): $(CLI_OBJECT)
	@echo "$(YELLOW)Génération de $@...$(NC)"
	$(CC) -o $@ $^
	@echo "$(GREEN)$(BOLD)$@ a été généré avec succès !$(NC)\r\n"

%.o: %.c
	@echo "$(YELLOW)Compilation de $<...$(NC)"
	$(CC) -c $< -o $@

clean:
	@echo "$(RED)Suppression des fichiers objets et des exécutables...$(NC)"
	rm -f $(SRV_DIR)/*.o $(CLI_DIR)/*.o $(SRV_BIN) $(CLI_BIN)

.PHONY: all clean

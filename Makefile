# Nome do executável
TARGET = run_n_splash

# Arquivos-fonte do projeto
SRC = run_n_splash.c Square.c Joystick.c

# Flags do Allegro
ALLEGRO_FLAGS = $(shell pkg-config allegro-5 allegro_font-5 allegro_main-5 allegro_primitives-5 allegro_image-5 --libs --cflags)

# Compilador
CC = gcc

# Regra padrão: compilar
all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(SRC) -o $(TARGET) $(ALLEGRO_FLAGS)

# Regra para executar o programa
run: all
	./$(TARGET)

# Limpeza
clean:
	rm -f $(TARGET)

.PHONY: all clean run

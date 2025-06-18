# Nome do executável final
TARGET = run_n_splash

# Lista de arquivos-fonte do projeto
SRCS = run_n_splash.c player.c enemy.c boss.c projetil.c utils.c Square.c Joystick.c

# Flags do compilador para warnings e padrão C99
CFLAGS = -std=c99

# Bibliotecas do Allegro (ajuste conforme sua instalação se necessário)
ALLEGRO_LIBS = -lallegro -lallegro_image -lallegro_font -lallegro_ttf -lallegro_primitives -lallegro_audio -lallegro_acodec -lm

# Regra padrão: compilar tudo
all: $(TARGET)

# Regra para linkar o executável
$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET) $(ALLEGRO_LIBS)

# Limpa arquivos objeto e executável
clean:
	rm -f $(TARGET) *.o

# Rodar o programa (opcional)
run: all
	./$(TARGET)
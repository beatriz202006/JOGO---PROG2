# JOGO---PROG2
gcc helloword.c Square.c Joystick.c -o helloword $(pkg-config allegro-5 allegro_font-5 allegro_main-5 allegro_primitives-5 --libs --cflags)
./helloword


gcc helloword.c Square.c Joystick.c -o helloword -lallegro -lallegro_font -lallegro_primitives -lallegro_image
./helloword

gcc helloword.c Square.c Joystick.c -o helloword $(pkg-config allegro-5 allegro_font-5 allegro_main-5 allegro_primitives-5 allegro_image-5 --libs --cflags)

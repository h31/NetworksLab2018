gcc -pthread -Wall -Werror -o server server.c response.c -lm
gcc -pthread -Wall -Werror -o client client.c line.c number.c input.c request.c response_applier.c -lm


Compilation instructions:
clang -std=c17 -Wall -Wextra -Werror src/main.c -o bin/main.exe -L.lib -lglfw3 -luser32 -lgdi32 -lshell32
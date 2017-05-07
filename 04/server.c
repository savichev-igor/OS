#include <errno.h>  // errno
#include <string.h>  // strerror
#include <stdio.h>  // input and output
#include <stdlib.h>  // random
#include <stdbool.h>  // boolean
#include <unistd.h>  // sleep
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

void print_error(char *info, bool exit_flag) {
    printf("%s: %s\n", strerror(errno), info);

    if (exit_flag) {
        exit(1);
    }
}

const int h = 30;
const int w = 30;

unsigned univ[h][w];

char shared_file[] = "shared.txt";

#define for_x for (int x = 0; x < w; x++)
#define for_y for (int y = 0; y < h; y++)
#define for_xy for_x for_y

/**
 * Выводит на экран текущую конфигурацию.
 */
void show() {
    for_y {
        for_x {
            printf(univ[y][x] ? "\033[07m  \033[m" : "  ");
        }

        printf("\n");
    }

    fflush(stdout);
}

/**
 * Делает один шаг развития.
 */
void evolve() {
    unsigned new[h][w];

    for_y {
        for_x {
            int n = 0;

            for (int y1 = y - 1; y1 <= y + 1; y1++) {
                for (int x1 = x - 1; x1 <= x + 1; x1++) {
                    if (univ[(y1 + h) % h][(x1 + w) % w]) {
                        n++;
                    }
                }
            }

            if (univ[y][x]) {
                n--;
            }

            new[y][x] = (unsigned) (n == 3 || (n == 2 && univ[y][x]));
        }
    }

    for_y {
        for_x {
            univ[y][x] = new[y][x];
        }
    }
}

/**
 * Инициализация игры.
 */
void init_game() {
    for_xy univ[y][x] = rand() < RAND_MAX / 10 ? 1 : 0;
}

/**
 * Цикл игры с записью промежуточного состояния в файл.
 */
void game() {
    init_game();

    int counter = 0;

    while (true) {
        FILE *fp = fopen(shared_file, "wa");

        for_y {
            for_x {
                fprintf(fp, "%d ", univ[y][x]);
            }
            fprintf(fp, "\n");
        };

        fclose(fp);

        clock_t begin = clock();
        evolve();
        clock_t end = clock();

        double time_spent = (double) (end - begin) / CLOCKS_PER_SEC;

        if (time_spent > 1) {
            printf("function performed for a very long time");
            exit(1);
        } else {
            usleep((1 - time_spent) * 1000000);
        }

        counter++;

        // Если сделали 100 итераций, то убьём дочерний процесс
        if (counter == 100) {
            exit(0);
        }
    }
}

void run_server() {
    int welcome_socket;
    int new_socket;

    struct sockaddr_in server_addr;
    struct sockaddr_storage server_storage;
    socklen_t addr_size;

    // Internet domain, Stream socket, Default protocol (TCP)
    welcome_socket = socket(PF_INET, SOCK_STREAM, 0);

    // Configure settings of the server address struct
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(7891);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Set all bits of the padding field to 0
    memset(server_addr.sin_zero, '\0', sizeof server_addr.sin_zero);

    // Bind the address struct to the socket
    bind(welcome_socket, (struct sockaddr *) &server_addr, sizeof(server_addr));

    // Listen on the socket, with 1 max connection requests queued
    listen(welcome_socket, 1);

    if (errno != 0) {
        print_error("Listen socket", true);
    }

    printf("Listening...\n");

    // Accept call creates a new socket for the incoming connection
    addr_size = sizeof server_storage;

    while (true) {
        FILE *fp = fopen(shared_file, "r");

        fseek(fp, 0, SEEK_END);
        long f_size = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        char * buffer = malloc((size_t) (f_size + 1));

        if (errno != 0) {
            print_error("malloc fail", true);
        }

        fread(buffer, (size_t) f_size, 1, fp);

        fclose(fp);

        new_socket = accept(welcome_socket, (struct sockaddr *) &server_storage, &addr_size);

        // Send message to the socket of the incoming connection
        send(new_socket, buffer, (size_t) f_size, 0);

        free(buffer);
    }
}

int main(int argc, char *argv[]) {
    FILE *fp = fopen(shared_file, "a");
    fclose(fp);

    switch(fork()) {
        case -1:
            print_error("Fork", true);
        case 0:
            game();
        default:
            run_server();
    }

    return 0;
}
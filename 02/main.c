#include <stdio.h>  // input and output
#include <stdbool.h>  // true (lol)
#include <unistd.h>  // всякие переменные аля STDIN_FILENO
#include <fcntl.h>  // file controle ?

const int BUF_SIZE = 100;

int create_sparce(char *file_name) {
    int fd = open(file_name, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

    if (fd < 0) {
        return fd;
    }

    char write_buffer[BUF_SIZE];
    int zero_offset = 0;
    int write_len = 0;

    char read_buffer[BUF_SIZE];
    int readed;
    int i;

    while (true) {
        readed = read(STDIN_FILENO, read_buffer, BUF_SIZE);
        i = 0;

        if (readed) {
            while (i < readed) {
                while (i < readed && read_buffer[i] != 0) {
                    write_buffer[write_len] = read_buffer[i];
                    write_len++;
                    i++;
                }

                if (write_len > 0) {
                    write(fd, write_buffer, write_len);
                    write_len = 0;
                }

                while (i < readed && read_buffer[i] == 0) {
                    zero_offset++;
                    i++;
                }

                if (zero_offset > 0) {
                    lseek(fd, zero_offset, SEEK_CUR);
                    zero_offset = 0;
                }
            }
        } else {
            break;
        }
    }
    return close(fd);
}

int main(int argc, char *argv[]) {
    if (create_sparce(argv[1]) < 0) {
        printf("%s\n", "Errors...");
    }
    return 0;
}

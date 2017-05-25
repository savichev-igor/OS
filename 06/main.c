#include <stdio.h>  // input and output
#include <stdbool.h>  // bool
#include <string.h>  // strerror
#include <unistd.h>  // access
#include <stdlib.h>  // malloc

char *global_file_name = NULL;
char *global_lock_file_name = NULL;
char *global_full_path_lock_file_name = NULL;

bool is_file_esists(char *path) {
    return access(path, F_OK) == 0 ? true : false;
}

void *get_lock_file_name(char *file_name) {
    char *file_name_token = strsep(&file_name, ".");
    char *extension_token = strsep(&file_name, ".");

    global_file_name = malloc(strlen(file_name_token) + strlen(extension_token));

    strcpy(global_file_name, file_name_token);
    strcat(global_file_name, ".");
    strcat(global_file_name, extension_token);

    global_lock_file_name = malloc(strlen(file_name_token) + 4);;

    strcpy(global_lock_file_name, file_name_token);
    strcat(global_lock_file_name, ".lck");
}

void *get_lock_file_path(char *file_name) {
    int offset = 5;

    global_full_path_lock_file_name = malloc(strlen(file_name) + offset);

    strcpy(global_full_path_lock_file_name, "/tmp/");
    strcat(global_full_path_lock_file_name, file_name);
}

void create_lock_file() {
    FILE *fp = fopen(global_full_path_lock_file_name, "wa");

    fprintf(fp, "%d\nappend", getpid());

    fclose(fp);
}

void remove_lock_file() {
    remove(global_full_path_lock_file_name);
}

void edit_file(int argc, char *data[]) {
    create_lock_file();

    FILE *fp = fopen(global_file_name, "a");

    fprintf(fp, "\n");

    for (int i = 1; i < argc - 2; i++) {
        fprintf(fp, "%s ", data[i]);
    }

    fprintf(fp, "%s", data[argc - 2]);

    fclose(fp);

    remove_lock_file();
}

int main(int argc, char *argv[]) {
    char *file_name = argv[argc - 1];

    get_lock_file_name(file_name);
    get_lock_file_path(global_lock_file_name);

    while (is_file_esists(global_full_path_lock_file_name)) {}

    edit_file(argc, argv);

    return 0;
}
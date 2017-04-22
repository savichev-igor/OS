#include <glib.h>

#include <stdio.h>
#include <string.h>  // strerror
#include <errno.h>
#include <regex.h>
#include <stdlib.h>  // malloc

void print_error(char *info) {
    printf("%s: %s\n", strerror(errno), info);
    exit(1);
}

int int_sort_function(gconstpointer a, gconstpointer b) {
    int *int_a = (int*) a;
    int *int_b = (int*) b;

    return (*int_a) - (*int_b);
}

void parse_file(char *file_name, GArray *array) {
    FILE *fp = fopen(file_name, "r");

    if (errno != 0) {
        print_error(file_name);
    }

    char *line = NULL;
    size_t len = 0;

    regex_t re;
    regmatch_t match;

    regcomp(&re, "-?[0-9]+", REG_EXTENDED);  // compile regexp

    if (errno != 0) {
        print_error("Regexp compile fail");
    }

    while ((getline(&line, &len, fp)) != -1) {
        while (regexec(&re, line, 1, &match, 0) == 0) {
            size_t len_match = (size_t) (match.rm_eo - match.rm_so);
            char *buffer = (char*) malloc(len_match);

            if (errno != 0) {
                print_error("Memory fail");
            }

            memcpy(buffer, line + match.rm_so, len_match);

            int num = atoi(buffer);
            free(buffer);

            g_array_append_val(array, num);

            line += match.rm_eo;
        }
    }

    fclose (fp);
}

int main(int argc, char *argv[]) {
    GArray *array = g_array_new(FALSE, FALSE, sizeof(int));


    for (int i = 1; i < argc - 1; i++) {
        parse_file(argv[i], array);
    }

    g_array_sort(array, int_sort_function);

    char *result_file_name = argv[argc - 1];

    FILE * result_fp = fopen(result_file_name, "wa");

    if (errno != 0) {
        print_error(result_file_name);
    }

    for (int i = 0; i < array->len; i++) {
        fprintf(result_fp, "%d ", g_array_index(array, int, i));
    }

    fclose(result_fp);
    g_array_free(array, TRUE);

    return 0;
}

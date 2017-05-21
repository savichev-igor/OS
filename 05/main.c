#include <stdio.h>  // input and output
#include <stdlib.h>  // exit
#include <stdbool.h> // true and false
#include <string.h>  // string
#include <errno.h>  // errno
#include <syslog.h>  // syslog
#include <unistd.h>  // fork
#include <signal.h>  // signals

#define MAX_PROC 100
#define MAX_NAME_LEN 100
#define MAX_ARGS_LEN 100
#define offset 5

int MAX_FILE_NAME_LEN = MAX_NAME_LEN + MAX_PROC + offset;

char name[MAX_PROC][MAX_NAME_LEN];
char *args[MAX_PROC][MAX_ARGS_LEN + 1];  // также +1 для NULL терминатора
bool respawnable[MAX_PROC];

pid_t pid_list[MAX_PROC];
int pid_list_tries[MAX_PROC];

int proc_numbers;
int counter_procs = 0;

int g_argc;
char **g_argv;


char *get_file_name(int proc_num) {
    char *file_name = malloc((size_t) MAX_FILE_NAME_LEN);
    sprintf(file_name, "/tmp/%i%s", proc_num, name[proc_num]);

    int i = offset;

    while (file_name[i]) {
        if (file_name[i] == '/') {
            file_name[i] = '-';
        }

        i++;
    }

    return file_name;
}

void create_file(int proc_num) {
    char file_name[MAX_FILE_NAME_LEN];
    strcpy(file_name, get_file_name(proc_num));

    FILE *fp = fopen(file_name, "wa");

    if (errno != 0) {
        syslog(LOG_WARNING, "Error: can't open file: /tmp/%s\n", file_name);
        exit(1);
    }

    fprintf(fp, "%d", pid_list[proc_num]);
    fclose(fp);
}

void remove_file(int proc_num) {
    char file_name[MAX_FILE_NAME_LEN];
    strcpy(file_name, get_file_name(proc_num));

    remove(file_name);

    if (errno != 0) {
        syslog(LOG_WARNING, "Error: can't delete /tmp/%s\n", file_name);
        exit(1);
    }
}

void spawn(int proc_num, bool need_sleep) {
    int cpid = fork();

    switch (cpid) {
        case -1:
            syslog(LOG_WARNING, "%s\n", "Error: can't fork");
            exit(1);
        case 0:
            if (need_sleep) {
                syslog(LOG_WARNING, "%s %s\n", "Error: we have some problems with", name[proc_num]);
                sleep(3600);
                pid_list_tries[proc_num] = 0;
            }

            execvp(name[proc_num], args[proc_num]);

            if (errno != 0) {
                syslog(LOG_WARNING, "%s %s", "Error: exec program", name[proc_num]);
                exit(1);
            }

            syslog(LOG_NOTICE, "%d spawned, pid = %d\n", proc_num, cpid);
            exit(0);
        default:
            pid_list[proc_num] = cpid;  // здесь лежит PID потомка
            create_file(proc_num);
            counter_procs++;
    }
}

void start_programs() {
    if (g_argc != 2) {
        syslog(LOG_WARNING, "Error: incorrect arguments\n");
        exit(1);
    }

    FILE *fp = fopen(g_argv[1], "r");

    if (errno != 0) {
        syslog(LOG_WARNING, "Can't open file\n");
        exit(1);
    }

    char *line = NULL;
    size_t len = 0;

    char *token = NULL;

    char *token_arg = NULL;
    int counter_args = 1;  // в 0 аргументе всегда лежит имя программы

    int counter = 0;
    int proc_num = 0;

    while ((getline(&line, &len, fp)) != -1) {
        while ((token = strsep(&line, ","))) {
            switch (counter) {
                case 0:
                    strcpy(name[proc_num], token);
                    break;
                case 1:
                    args[proc_num][0] = (char *) malloc(strlen(name[proc_num]));
                    strcpy(args[proc_num][0], name[proc_num]);

                    while ((token_arg = strsep(&token, " "))) {
                        args[proc_num][counter_args] = (char *) malloc(MAX_ARGS_LEN);
                        strcpy(args[proc_num][counter_args], token_arg);

                        counter_args++;
                    }

                    args[proc_num][counter_args] = NULL;
                    break;
                case 2:
                    respawnable[proc_num] = strcmp(token, "wait\n") == 0 ? false : true;
                    break;
            }
            counter++;
        }
        counter = 0;
        counter_args = 1;
        proc_num++;
    }

    proc_numbers = proc_num;

    fclose(fp);

    for (int i = 0; i < proc_numbers; i++) {
        spawn(i, false);
    }

    while (counter_procs) {
        int status;
        pid_t cpid = wait(&status);

        for (int i = 0; i < proc_numbers; i++) {
            if (pid_list[i] == cpid) {
                syslog(LOG_NOTICE, "Child %d pid %d finished\n", i, cpid);
                remove_file(i);
                counter_procs--;

                if (respawnable[i]) {
                    if (status != 0) {
                        pid_list_tries[i]++;
                    }

                    if (pid_list_tries[i] == 50) {
                        spawn(i, true);
                    } else {
                        spawn(i, false);
                    }
                }
            }
        }
    }
}

void HUP_handler() {
    for (int i = 0; i < proc_numbers; i++) {
        kill(pid_list[i], SIGKILL);
    }

    start_programs();
}

int main(int argc, char *argv[]) {
    g_argc = argc;
    g_argv = argv;

    signal(SIGHUP, (void (*)(int)) HUP_handler);

    int pid = fork();

    // by http://mainloop.ru/c-language/kak-sozdat-demona-v-linux.html
    switch (pid) {
        case -1:
            syslog(LOG_WARNING, "%s\n", "Error: can't start daemon");
            exit(1);
        case 0:
            setsid();
            chdir("/");
            close((int) stdin);
            close((int) stdout);
            close((int) stderr);
            start_programs();
            exit(0);
        default:
            return 0;
    }
}
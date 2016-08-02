/* simple-shell.c
 *
 * Authors:
 *   Karl Apsite
 *
 * A simplified shell that recieves user input, and attempts to fork and exec
 * the given command.
 */

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>

#include "history.h"
#include "shell_util.h"
#include "jobs.h"

#define SIZE        1024
#define EXIT_NORMAL 255

volatile pid_t last_corpse;
volatile int last_corpse_status;

int do_cmd(char *input, char *cwd, history_t history, FILE *in, FILE *out);

void int_handler(int signum) {
    write(STDOUT_FILENO, "^C\n", 3);
}

void stop_handler(int signum) {
    write(STDOUT_FILENO, "^Z\n", 3);
}

void child_handler(int signum) {
    last_corpse = wait((int *)(&last_corpse_status));
    // remove job from running list
    empty_job(last_corpse);
}

int main(int argc, char* argv[]) {
    char input[SIZE];
    char cwd[SIZE];
    char prompt[SIZE];
    history_t history;
    char ret_val;
    struct sigaction int_a;
    struct sigaction stop_a;
    struct sigaction child_a;

    // register some interrupt handlers
    int_a.sa_handler = int_handler;
    int_a.sa_flags = 0;
    sigemptyset(&int_a.sa_mask);
    sigaction(SIGINT, &int_a, NULL);

    stop_a.sa_handler = stop_handler;
    stop_a.sa_flags = 0;
    sigemptyset(&stop_a.sa_mask);
    sigaction(SIGTSTP, &stop_a, NULL);


    child_a.sa_handler = child_handler;
    child_a.sa_flags = 0;
    sigemptyset(&child_a.sa_mask);
    sigaction(SIGCHLD, &child_a, NULL);

    // crude job handling
    init_job_list();

    // generate a prompt
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("getcwd() error");
        return 1;
    }

    // store history in a temp file somewhere
    if ((history = init_history(SIZE)) == NULL) {
        return 1;
    }

    // generate a prompt first
    gen_prompt(cwd, prompt, SIZE);
    printf("%s", prompt);

    while(1) {
        if (fgets(input, SIZE, stdin) == NULL)
            continue;

        if ((ret_val = do_cmd(input, cwd, history, stdin, stdout)))
            break;

        // regenerate prompt after every command
        if ((ret_val = gen_prompt(cwd, prompt, SIZE)))
            break;
        printf("%s", prompt);
    }

    if (ret_val == EXIT_NORMAL)
        ret_val = 0;

    close_history(history);
    return ret_val;
}

int do_cmd(char *input, char *cwd, history_t history, FILE *in, FILE *out) {
    char *tokens[SIZE];
    char *trim;
    int len;
    int num_tokens;
    int pid;
    int backgnd = 0;

    // trim leading whitespace
    trim = input;
    while (*trim == ' ')
        trim++;

    len = strlen(trim);
    if (len == 1)
        return 0;

    // remove trailing newlines
    if (trim[len-1] == '\n')
        trim[len-1] = '\0';

    // check to see if the user is trying to replay history right away
    if (strncmp(trim, "!", 1) == 0) {
        if (replay_history(history, get_val(trim + sizeof(char)), input)) {
            fprintf(stderr, "Line '%s' not found\n", trim + sizeof(char));
            return 0;
        }
        return do_cmd(input, cwd, history, in, out);
    }

    // don't store history replay's in the history
    append_history(history, trim);

    // tokenize
    num_tokens = tokenize(trim, tokens);

    // built-ins
    // internal exit command
    if (!strncmp(tokens[0], "quit", 5) ||
        !strncmp(tokens[0], "exit", 5) ) {
        return EXIT_NORMAL;
    }

    // print working directory
    if (!strncmp(tokens[0], "pwd", 4)) {
        printf("%s\n", cwd);
        return 0;
    }

    // print all history
    if (!strncmp(tokens[0], "history", 8)) {
        print_history(history);
        return 0;
    }

    // print all jobs
    if (!strncmp(tokens[0], "jobs", 5)) {
        print_job_list();
        return 0;
    }

    // change to a directory
    if (strncmp(tokens[0], "cd", 2) == 0) {
        if (num_tokens > 2) {
            if (chdir(tokens[1])) {
                perror("No such file or directory");
            } else {
                // on success, reset cwd
                if (getcwd(cwd, sizeof(char) * SIZE) == NULL) {
                    perror("getcwd() error");
                    return 1;
                }
            }
        }
        return 0;
    }

    // do some token scraping for redirection
    if (check_for_specials(tokens, num_tokens, &in, &out, &backgnd))
        return 0;

    if ((pid = fork()) < 0) {
        perror("fork failure");
        return 1;
    } else if (pid == 0) {
        // allow inpout and output redirection
        if (in)
            dup2(fileno(in), STDIN_FILENO);
        if (out)
            dup2(fileno(out), STDOUT_FILENO);

        // kick off this wild ride
        if (execvp(tokens[0], tokens) < 0) {
            fprintf(stderr, "No command \'%s\' found\n", tokens[0]);
            exit(1);
        }
        // here be unreachable
    } else {
        // add job to list of running jobs
        len = add_job(pid, tokens);
        if (!backgnd) {
            while(last_corpse != pid);
            printf("Process returned: %d\n", last_corpse_status);
        } else {
            printf("[%d] %d\n", len, pid);
        }
        return 0;
    }

    // The compiler doesn't see the exec, and thinks a
    // child could make it here
    fprintf(stderr, "Warning: child escaped!\n");
    return 0;
}

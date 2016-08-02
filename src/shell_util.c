#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>

#include "shell_util.h"

/*
 * Splits up a line of input into tokens
 */
int tokenize(char *input, char *tokens[]) {
    int tok_idx = 0;
    tokens[tok_idx++] = strtok(input, " ");
    while((tokens[tok_idx++] = strtok(NULL, " ")));
    return tok_idx;
}

/*
 * Fills with prompt with something that resembles a bash directory
 */
int gen_prompt(const char* cwd, char *prompt, size_t len) {
    char *home;

    // get user's home directory
    if ((home = getenv("HOME")) == NULL) {
        if ((home = getpwuid(getuid())->pw_dir) == NULL) {
            return 1;
        }
    }

    if (strncmp(home, cwd, strlen(home)) == 0) {
        snprintf(prompt, len, "~%s$ ", cwd+strlen(home));
    } else {
        snprintf(prompt, len, "%s$ ", cwd);
    }

    return 0;
}

/*
 * Harvests a number from a token.
 * If the token does not start or end with a number, it is thrown out
 * and -1 is returned.
 */
int get_val(const char *tok) {
    char *end;
    int val;
    val = strtol(tok, &end, 10);
    if ('\0' != *end)
        return -1;
    return val;
}

/*
 * Check for special tokens like <, >, and &
 *  '>> file_out' - redirect stdout to fileout (appended)
 *  '> file_out' - redirect stdout to fileout (restart)
 *  'file_in <' - redirect file_in to stdin
 *  '> file <' - pipe output through file
 */
int check_for_specials(char *tokens[], int num_tokens, FILE **in, FILE **out, int *backgnd) {
    int idx;
    int clobber_idx;

    idx = 0;
    clobber_idx = 0;
    while(idx < num_tokens) {
        if (tokens[idx]) {
            if (strncmp(tokens[idx], ">", 1) == 0) {
                if (idx == num_tokens - 1) {
                    fprintf(stderr, "Error: Output redirection given without a file name\n");
                    return 1;
                }
                if ((*out = fopen(tokens[idx+1], strncmp(tokens[idx], ">>", 2) ? "w":"a")) == NULL) {
                    fprintf(stderr, "Error: Couldn't open '%s' for writing\n", tokens[idx+1]);
                    return 1;
                }
                idx += 2;

            } else if (strncmp(tokens[idx], "<", 1) == 0) {
                if (idx == 0) {
                    fprintf(stderr, "Error: Input redirection given without a file name\n");
                    return 1;
                }
                if ((*in = fopen(tokens[idx-1], "r")) == NULL) {
                    fprintf(stderr, "Error: Couldn't open '%s' for reading\n", tokens[idx-1]);
                    return 1;
                }
                clobber_idx--;
                idx++;

            } else if (strncmp(tokens[idx], "&", 1) == 0) {
                *backgnd = 1;
                idx++;
            }
        }
        tokens[clobber_idx++] = tokens[idx++];
    }

    return 0;
}

#ifndef SHELL_UTIL_H
#define SHELL_UTIL_H

typedef struct prompt *prompt_t;

int tokenize(char *input, char *tokens[]);
int gen_prompt(const char *cwd, char *prompt, size_t len);
int get_val(const char *tok);
int check_for_specials(char *tokens[], int num_tokens, FILE **in, FILE **out, int *backgnd);

#endif // SHELL_UTIL_H

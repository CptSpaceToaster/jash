#ifndef HISTORY_H
#define HISTORY_H

typedef struct history *history_t;

history_t init_history(size_t len);
int append_history(history_t H, const char *input);
int print_history(history_t H);
int replay_history(history_t H, int event, char *output);
int close_history(history_t H);

#endif // HISTORY_H

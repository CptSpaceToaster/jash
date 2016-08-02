#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "history.h"

struct history {
    char *buf;
    FILE *fp;
    size_t line_len;
};

/*
 * Create a new history object
 */
history_t init_history(size_t len) {
    history_t H;

    if ((H = malloc(sizeof(struct history))) == NULL) {
        perror("Cannot malloc a history object");
        return NULL;
    }

    H->line_len = sizeof(char) * len;
    if ((H->buf = malloc(H->line_len)) == NULL) {
        perror("Cannot malloc a line buffer");
        free(H);
        return NULL;
    }

    H->fp = tmpfile();
    return H;
}

/*
 * Append to the shell's history
 * Returns 1 if a write fails
 */
int append_history(history_t H, const char *input) {
    if ((fwrite(input, sizeof(char), strlen(input), H->fp) != strlen(input)) || (fwrite("\n", sizeof(char), 1, H->fp) != 1)) {
        perror("Cannot append to history");
        return 1;
    }
    return 0;
}

/*
 * Prints the contents of this history file, with line numbers
 */
int print_history(history_t H) {
    int line_num;

    // print lines one by one
    rewind(H->fp);
    line_num = 0;
    while (fgets(H->buf, H->line_len, H->fp) != NULL) {
        printf("%-4d %s", line_num++, H->buf);
    }

    return 0;
}

/*
 * Attempts to return the line that occured on line 'event'
 * Returns 1 if the event can not be found.
 * If the event is found, it is copied to 'output'
 */
int replay_history(history_t H, int event, char *output) {
    if (event++ < 0)
        return 1;

    rewind(H->fp);
    while (event > 0 && (fgets(H->buf, H->line_len, H->fp) != NULL)) {
        event--;
    }
    fseek(H->fp, 0, SEEK_END);

    if (!event)
        strncpy(output, H->buf, H->line_len);

    return event;
}

/*
 * Close a history object
 */
int close_history(history_t H) {
    free(H->buf);
    return fclose(H->fp);
}

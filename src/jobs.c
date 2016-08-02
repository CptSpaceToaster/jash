#include <stdio.h>
#include <string.h>

#include "jobs.h"

#define MAX_JOBS 30

// this isn't very ideal... but it works for now
typedef struct job {
    int pid;
    char cmd[1024];
} job_t;

job_t job_list[MAX_JOBS];

/*
 * Initialise the job list with "empty" values (-1)
 */
void init_job_list() {
    int i;
    for (i=0; i<MAX_JOBS; i++) {
        job_list[i].pid = -1;
    }
}

/*
 * Add a job to the list, returns -1 if the job could not be added
 */
int add_job(int pid, char *tokens[]) {
    int i;
    int j;
    int offset;
    for (i=0; i<MAX_JOBS; i++) {
        if (job_list[i].pid == -1)
            break;
    }
    if (i == MAX_JOBS)
        return -1;

    job_list[i].pid = pid;

    j = 0;
    offset = 0;
    while(tokens[j]) {
        offset += snprintf(job_list[i].cmd + offset, 1024 - offset, "%s ", tokens[j++]);
    }

    return i;
}

/*
 * Remove a job from the list, returns -1 if the job could not be found
 */
int empty_job(int pid) {
    int i;
    for (i=0; i<MAX_JOBS; i++) {
        if (job_list[i].pid == pid)
            break;
    }
    if (i == MAX_JOBS)
        return -1;
    job_list[i].pid = -1;
    return i;
}

/*
 * mimic bash a bit
 */
void print_job_list() {
    int i;
    for (i=0; i<MAX_JOBS; i++) {
        if (job_list[i].pid == -1)
            continue;
        printf("[%d] %-6d        %s\n", i, job_list[i].pid, job_list[i].cmd);
    }
}

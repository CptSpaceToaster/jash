#ifndef JOBS_H
#define JOBS_H

void init_job_list(void);
int add_job(int pid, char *tokens[]);
int empty_job(int pid);
void print_job_list(void);

#endif // JOBS_H


#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#define MAX_SHOW 10

static const char *SUSPICIOUS[] = {
    "failed password", "invalid user", "unauthorized",
    "permission denied", "access denied", "brute force",
    "sql injection", "segfault", "root login", "port scan",
    "authentication failure", NULL
};

typedef struct {
    long errors, warnings, suspicious, lines;
} Stats;

typedef struct {
    char *start, *end;
    Stats local;
    char *err_lines[MAX_SHOW];
    char *sus_lines[MAX_SHOW];
    int n_err, n_sus;
} Task;

static Stats total;
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

static int is_suspicious(const char *line) {
    for (int i = 0; SUSPICIOUS[i] != NULL; i++) {
        if (strcasestr(line, SUSPICIOUS[i]))
            return 1;
    }
    return 0;
}

static void *worker(void *arg) {
    Task *t = arg;
    char *p = t->start;

    while (p < t->end) {
        char *nl = memchr(p, '\n', (size_t)(t->end - p));

        if (nl != NULL)
            *nl = '\0';

        t->local.lines++;

        if (strcasestr(p, "error")) {
            t->local.errors++;

            if (t->n_err < MAX_SHOW)
                t->err_lines[t->n_err++] = p;
        } else if (strcasestr(p, "warn")) {
            t->local.warnings++;
        }

        if (is_suspicious(p)) {
            t->local.suspicious++;

            if (t->n_sus < MAX_SHOW)
                t->sus_lines[t->n_sus++] = p;
        }

        if (nl == NULL)
            break;

        p = nl + 1;
    }

    pthread_mutex_lock(&lock);

    total.errors += t->local.errors;
    total.warnings += t->local.warnings;
    total.suspicious += t->local.suspicious;
    total.lines += t->local.lines;

    pthread_mutex_unlock(&lock);

    return NULL;
}

int main(int argc, char **argv) {
    if (argc < 2 || argc > 3) {
        fprintf(stderr, "Usage: %s <logfile> [threads]\n", argv[0]);
        return 1;
    }

    int nthreads = 4;

    if (argc == 3) {
        char *endptr;
        long value = strtol(argv[2], &endptr, 10);

        if (*argv[2] == '\0' || *endptr != '\0' ||
            value < 1 || value > 256) {
            fprintf(stderr, "Threads must be between 1 and 256\n");
            return 1;
        }

        nthreads = (int)value;
    }

    FILE *f = fopen(argv[1], "rb");

    if (f == NULL) {
        perror("Cannot open log file");
        return 1;
    }

    if (fseek(f, 0, SEEK_END) != 0) {
        perror("Cannot seek in log file");
        fclose(f);
        return 1;
    }

    long size = ftell(f);

    if (size < 0 || fseek(f, 0, SEEK_SET) != 0) {
        perror("Cannot determine log file size");
        fclose(f);
        return 1;
    }

    char *buf = malloc((size_t)size + 1);

    if (buf == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        fclose(f);
        return 1;
    }

    size_t bytes_read = fread(buf, 1, (size_t)size, f);

    if (bytes_read != (size_t)size || ferror(f)) {
        fprintf(stderr, "Failed to read log file completely\n");
        free(buf);
        fclose(f);
        return 1;
    }

    buf[size] = '\0';
    fclose(f);

    Task *tasks = calloc((size_t)nthreads, sizeof(Task));
    pthread_t *tid = malloc((size_t)nthreads * sizeof(pthread_t));

    if (tasks == NULL || tid == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        free(tasks);
        free(tid);
        free(buf);
        return 1;
    }

    char *pos = buf;
    char *end = buf + size;
    long step = size / nthreads + 1;

    for (int i = 0; i < nthreads; i++) {
        tasks[i].start = pos;

        if (i == nthreads - 1) {
            tasks[i].end = end;
        } else {
            char *e;

            if (pos >= end || step > end - pos) {
                e = end;
            } else {
                e = pos + step;
            }

            /* Move to the end of the current line. */
            while (e < end && *e != '\n')
                e++;

            if (e < end)
                e++;

            tasks[i].end = e;
        }

        pos = tasks[i].end;
    }

    for (int i = 0; i < nthreads; i++) {
        int rc = pthread_create(&tid[i], NULL, worker, &tasks[i]);

        if (rc != 0) {
            fprintf(stderr,
                    "Warning: thread %d could not start; "
                    "processing its task directly.\n", i);

            /* Process this task without creating another thread. */
            worker(&tasks[i]);
            tid[i] = (pthread_t)0;
        }
    }

    /*
     * Join only threads that were successfully created.
     * A zero thread ID is used as the failure marker above.
     */
    for (int i = 0; i < nthreads; i++) {
        if (tid[i] != (pthread_t)0) {
            int rc = pthread_join(tid[i], NULL);

            if (rc != 0) {
                fprintf(stderr, "Error joining thread %d\n", i);
                free(tasks);
                free(tid);
                free(buf);
                return 1;
            }
        }
    }

    printf("\n=== Log Analysis Report (%d threads) ===\n",
           nthreads);
    printf("Lines scanned     : %ld\n", total.lines);
    printf("Errors            : %ld\n", total.errors);
    printf("Warnings          : %ld\n", total.warnings);
    printf("Suspicious events : %ld\n", total.suspicious);

    printf("\n-- Sample errors --\n");

    int shown = 0;

    for (int i = 0; i < nthreads && shown < MAX_SHOW; i++) {
        for (int j = 0;
             j < tasks[i].n_err && shown < MAX_SHOW;
             j++, shown++) {
            printf("  %s\n", tasks[i].err_lines[j]);
        }
    }

    printf("\n-- Sample suspicious events --\n");

    shown = 0;

    for (int i = 0; i < nthreads && shown < MAX_SHOW; i++) {
        for (int j = 0;
             j < tasks[i].n_sus && shown < MAX_SHOW;
             j++, shown++) {
            printf("  %s\n", tasks[i].sus_lines[j]);
        }
    }

    free(tasks);
    free(tid);
    free(buf);

    pthread_mutex_destroy(&lock);

    return 0;
}


/* See the LICENSE file for copyright and license details. */

#include <errno.h>
#include <getopt.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

enum {
        WRONG_FORMAT = -2,
        WRONG_USAGE = -3,
        OUT_OF_RANGE = -4
};

/* the default, minimal and maximal 'nthreads' value */
#define NTHREADS_DEF 10
#define NTHREADS_MAX 100
#define NTHREADS_MIN 1

#define PRINT_USAGE_HINT() printf("Usage: %s [-n nthreads (1:100)].\n", argv[0])

/* args for the thread function 'foo' */
typedef struct {
        int thread_id;
} Args;

/* a thread function that prints a thread id */
void *
foo(void *args_void)
{
        Args *args;

        args = (Args *)args_void;
        printf("Hello World! I'm the %dth thread.\n", args->thread_id);

        return NULL;
}

int
main(int argc, char *argv[])
{
        int nthreads; /* Number of threads */
        pthread_t *threads;
        Args *threads_args;

        { /* Parse options */
        int opt;
        bool n_found;

        n_found = false;
        while ((opt = getopt(argc, argv, "n:")) != -1) {
                switch (opt) {
                case 'n': {
                        char *end;
                        nthreads = strtol(optarg, &end, 10);
                        if (*end != '\0') {
                                fprintf(stderr, "Wrong format of 'nthreads'.\n");
                                PRINT_USAGE_HINT();
                                exit(WRONG_FORMAT);
                        }
                        if (nthreads < NTHREADS_MIN || nthreads > NTHREADS_MAX) {
                                fprintf(stderr, "The 'nthreads' value is out of range.\n");
                                PRINT_USAGE_HINT();
                                exit(OUT_OF_RANGE);
                        }
                        n_found = true;
                        break;
                }
                default:
                        PRINT_USAGE_HINT();
                        exit(WRONG_USAGE);
                        break;
                }
        }
        if (!n_found) {
                nthreads = NTHREADS_DEF;
        }
        }

        threads = (pthread_t *)malloc(nthreads * sizeof(*threads));
        threads_args = (Args *)malloc(nthreads * sizeof(*threads_args));
        for (int i = 0; i < nthreads; ++i) {
                threads_args[i] = (Args){i + 1};
                if (pthread_create(&threads[i], NULL, foo, &threads_args[i])) {
                        perror("pthread_create");
                        exit(errno);
                }
        }

        for (int i = 0; i < nthreads; ++i) {
                if (pthread_join(threads[i], NULL)) {
                        perror("pthread_join");
                        exit(errno);
                }
        }

        free(threads_args);
        free(threads);

        exit(EXIT_SUCCESS);
}

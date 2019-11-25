/* See LICENSE file for copyright and license details. */

#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

enum {
        WRONG_FORMAT = -2,
        WRONG_USAGE = -3,
        OUT_OF_RANGE = -4
};

#define NTHREAD_DEF 10 /* the default 'nthreads' value */

#define PRINT_USAGE_HINT() printf("Usage: %s [-n nthreads (1:100)].\n", argv[0])

int
main(int argc, char *argv[])
{
        int nthreads; /* Number of threads */

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
                        if (nthreads < 1 || nthreads > 100) {
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
                nthreads = NTHREAD_DEF;
        }
        }

        printf("n = %d\n", nthreads);

        exit(EXIT_SUCCESS);
}

/* See the LICENSE file for copyright and license details. */

#include <sys/msg.h>
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
        int msgid;
} Args;

/* message buffer */
typedef struct {
        long mtype;
} MsgBuf;

/* a thread function that prints a thread id */
void *
foo(void *args_void)
{
        Args *args;
        MsgBuf msgbuf;

        args = (Args *)args_void;
        if (msgrcv(args->msgid, &msgbuf, 0, args->thread_id, 0)) {
                perror("msgrcv");
                exit(errno);
        }
        printf("Hello World! I'm the %dth thread.\n", args->thread_id);
        ++msgbuf.mtype;
        if (msgsnd(args->msgid, &msgbuf, 0, IPC_NOWAIT)) {
                perror("msgsnd");
                exit(errno);
        }

        return NULL;
}

int
main(int argc, char *argv[])
{
        int nthreads; /* Number of threads */
        pthread_t *threads;
        Args *threads_args;
        int msgid;
        MsgBuf msgbuf;

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

        if ((msgid = msgget(IPC_PRIVATE, IPC_CREAT | 0666) < 0)) {
                perror("msgget");
                exit(errno);
        }
        msgbuf = (MsgBuf){1};
        if (msgsnd(msgid, &msgbuf, 0, IPC_NOWAIT)) {
                perror("msgsnd");
                exit(errno);
        }
        if ((threads = (pthread_t *)malloc(nthreads * sizeof(*threads))) == NULL) {
                perror("malloc");
                exit(errno);
        }
        if ((threads_args = (Args *)malloc(nthreads * sizeof(*threads_args))) == NULL) {
                perror("malloc");
                exit(errno);
        }
        for (int i = 0; i < nthreads; ++i) {
                threads_args[i] = (Args){i + 1, msgid};
                if (pthread_create(&threads[i], NULL, foo, &threads_args[i])) {
                        perror("pthread_create");
                        exit(errno);
                }
        }

        if (msgrcv(msgid, &msgbuf, 0, nthreads + 1, 0)) {
               perror("msgrcv");
               exit(errno);
        }

        printf("Hello World!\n");

        for (int i = 0; i < nthreads; ++i) {
                if (pthread_join(threads[i], NULL)) {
                        perror("pthread_join");
                        exit(errno);
                }
        }
        if (msgctl(msgid, IPC_RMID, NULL)) {
                perror("msgctl");
                exit(errno);
        }
        free(threads_args);
        free(threads);

        exit(EXIT_SUCCESS);
}

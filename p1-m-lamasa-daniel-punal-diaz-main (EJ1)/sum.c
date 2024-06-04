#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include "options.h"

struct nums {
    long increase;
    long decrease;
    long total;
    long diff;
};

struct args {
    int thread_num;		// application defined thread #
    long iterations;	// number of operations
    struct nums *nums;	// pointer to the counters (shared with other threads)

    pthread_mutex_t *mutex_ej1;  //EJERCICIO 1
};

struct thread_info {
    pthread_t    id;    // id returned by pthread_create()
    struct args *args;  // pointer to the arguments
};

// Threads run on this function
void *decrease_increase(void *ptr)
{
    struct args *args = ptr;
    struct nums *n = args->nums;

    while(args->iterations--) {
        pthread_mutex_lock(args->mutex_ej1); //EJERCICIO 1
        n->decrease--;
        n->increase++;
        long diff = n->total - (n->decrease + n->increase);
        pthread_mutex_unlock(args->mutex_ej1); //EJERCICIO 1;

        if (diff != n->diff) {
            n->diff = diff;
            printf("Thread %d increasing %ld decreasing %ld diff %ld\n",
                   args->thread_num, n->increase, n->decrease, diff);
        }
    }
    return NULL;
}

// start opt.num_threads threads running on decrease_incresase
struct thread_info *start_threads(struct options opt, struct nums *nums)
{
    int i;
    struct thread_info *threads;

    pthread_mutex_t  *mutex_ej1;

    printf("creating %d threads\n", opt.num_threads);
    threads = malloc(sizeof(struct thread_info) * opt.num_threads);

    mutex_ej1 = malloc(sizeof(pthread_mutex_t));  //EJERCICIO 1
    pthread_mutex_init(mutex_ej1, NULL); //EJERCICIO 1

    if (threads == NULL) {
        printf("Not enough memory\n");
        exit(1);
    }

    // Create num_thread threads running decrease_increase
    for (i = 0; i < opt.num_threads; i++) {
        threads[i].args = malloc(sizeof(struct args));
        threads[i].args->mutex_ej1 = mutex_ej1;  //EJERCICIO 1
        threads[i].args->thread_num = i;
        threads[i].args->nums       = nums;
        threads[i].args->iterations = opt.iterations;

        if (0 != pthread_create(&threads[i].id, NULL, decrease_increase, threads[i].args)) {
            printf("Could not create thread #%d", i);
            exit(1);
        }
    }

    return threads;
}

void print_totals(struct nums *nums)
{
    printf("Final: increasing %ld decreasing %ld diff %ld\n",
           nums->increase, nums->decrease, nums->total - (nums->decrease + nums->increase));
}

// wait for all threads to finish, print totals, and free memory
void wait(struct options opt, struct nums *nums, struct thread_info *threads) {
    // Wait for the threads to finish
    for (int i = 0; i < opt.num_threads; i++)
        pthread_join(threads[i].id, NULL);

    print_totals(nums);

    pthread_mutex_destroy(threads->args->mutex_ej1);  //EJERCICIO 1

    for (int i = 0; i < opt.num_threads; i++)
        free(threads[i].args);

    free(threads);
}

int main (int argc, char **argv)
{
    struct options opt;
    struct nums nums;
    struct thread_info *thrs;

    srand(time(NULL));

    // Default values for the options
    opt.num_threads  = 4;
    opt.iterations   = 100000;
    opt.size         = 10;

    read_options(argc, argv, &opt);

    nums.total = opt.iterations * opt.num_threads;
    nums.increase = 0;
    nums.decrease = nums.total;
    nums.diff = 0;
    thrs = start_threads(opt, &nums);
    wait(opt, &nums, thrs);

    return 0;
}
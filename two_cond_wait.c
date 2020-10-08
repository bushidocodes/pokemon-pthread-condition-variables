#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <errno.h>

#include "pokemon.h"

#define THREAD_COUNT 5
pthread_t threads[THREAD_COUNT] = {0};
pthread_mutex_t trainers_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t trainers = PTHREAD_COND_INITIALIZER;
pthread_mutex_t gym_leader_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t gym_leader = PTHREAD_COND_INITIALIZER;

char *trainer_name[5] = {"Ash", "Misty", "Brock", "Jessie", "James"};
const char *trainer_pokemon[5] = {"", "", "", "", ""};
int64_t picks = 0;

void maketimeout(struct timespec *tsp, int64_t secs)
{
    /* Get the current time */
    struct timeval now;
    gettimeofday(&now, NULL);

    /* Set timespec to now */
    tsp->tv_sec = now.tv_sec;
    tsp->tv_nsec = now.tv_usec * 1000;

    /* Add desired offset */
    tsp->tv_sec += secs;
}

void *thd_main(void *arg_raw)
{
    /* We are passing the trainer ID by value */
    int64_t trainer_id = (int64_t)arg_raw;

    /* We have to take the lock to be able to wait on the condition variable */
    pthread_mutex_lock(&trainers_lock);

    while (true)
    {
        /* Uncomment to trigger race condition */
        sleep(1);

        do
        {
            pthread_cond_wait(&trainers, &trainers_lock);
        } while (picks == THREAD_COUNT);

        trainer_pokemon[trainer_id] = get_random_pokemon();
        printf("\t%s chooses %s\n", trainer_name[trainer_id], trainer_pokemon[trainer_id]);

        if (++picks % THREAD_COUNT == 0)
            pthread_cond_signal(&gym_leader);
    }

    int64_t rc = 0;
    pthread_exit((void *)rc);
}

int main(void)
{
    /* Seed Random */
    time_t t;
    srand((unsigned)time(&t));

    /* Create the threads, each representing a trainer, passing a trainer ID */
    for (int64_t i = 0; i < THREAD_COUNT; i++)
    {
        pthread_create(&threads[i], NULL, thd_main, (void *)i);
    }

    pthread_mutex_lock(&gym_leader_lock);

    /* Execute the tournament. Try using signal versus broadcast and with and without the sleep */
    for (int64_t round = 1; round <= 10; round++)
    {
        printf("Round %ld:\n", round);

        pthread_cond_broadcast(&trainers);

        while (picks < THREAD_COUNT)
        {
            /* Set a timeout one second in the future in case of race condition */
            struct timespec tsp;
            maketimeout(&tsp, 2);

            if (pthread_cond_timedwait(&gym_leader, &gym_leader_lock, &tsp) == ETIMEDOUT)
            {
                printf("Timed out!\n");
                pthread_cond_signal(&trainers);
            };
        }

        picks = 0;
    }

    exit(EXIT_SUCCESS);
}

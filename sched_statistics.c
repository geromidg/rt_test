/**
  * @file sched_statistics.c
  * @brief Implements a statistics module to monitor the scheduler's timing.
  *
  * @author Dimitrios Panagiotis G. Geromichalos (dgeromichalos)
  * @date September, 2016
  */

/******************************** Inclusions *********************************/

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "sched_statistics.h"

/***************************** Static Variables ******************************/

static struct timespec last_statistics_timer;

static u8_t is_first_cycle = 1u;

static u64_t number_of_calls;

static float cycle_time;
static float cur_error;
static float avg_error = 0.f;
static float min_error;
static float max_error = -1.f;

/************************ Static Function Prototypes *************************/

/**
  * @brief Update the errors according to the time delta.
  * @param time_delta The time difference between the current and previous cycle.
  * @return Void.
  */
static void updateStatistics(s32_t time_delta);

/***************************** Static Functions ******************************/

void updateStatistics(s32_t time_delta)
{
    if (is_first_cycle)
    {
        is_first_cycle = 0;
        return;
    }

    cur_error = cycle_time - time_delta;
    cur_error = (cur_error >= 0) ? cur_error : -cur_error;

    avg_error = ((avg_error * number_of_calls) + cur_error) / (number_of_calls + 1);
    number_of_calls++;

    if (cur_error < min_error)
        min_error = cur_error;

    if (cur_error > max_error)
        max_error = cur_error;
}

/***************************** Public Functions ******************************/

void InitStatistics(float cycle, s32_t sched_timer_nsec)
{
    cycle_time = cycle;

    min_error = cycle_time;
    last_statistics_timer.tv_nsec = sched_timer_nsec;
}

void PrintStatistics(void)
{
    struct timespec current_t;
    s32_t time_delta;

    clock_gettime(CLOCK_MONOTONIC, &current_t);

    time_delta = current_t.tv_nsec - last_statistics_timer.tv_nsec;
    time_delta = (time_delta >= 0) ? time_delta : (time_delta + 1000000000u);  /* FIXME: Use NSEC_PER_SEC */
    updateStatistics(time_delta);

    last_statistics_timer.tv_nsec = current_t.tv_nsec;

    /* printf("\rtime=%.6f s , cur_error=%05.2f us , avg_error=%05.2f us , min_error=%05.2f us , max_error=%05.2f us", */
    printf("time=%.6f s , cur_error=%05.2f us , avg_error=%05.2f us , min_error=%05.2f us , max_error=%05.2f us\n",
        current_t.tv_sec + (current_t.tv_nsec / (float)1000000000u),
        cur_error / 1000.f,
        avg_error / 1000.f,
        min_error / 1000.f,
        max_error / 1000.f);

    fflush(stdout);
}

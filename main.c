/**
  * @file main.c
  * @brief Contains the scheduler and the entry point for the implementation of Project 2 on BCM2837.
  *
  * The scheduler is the main entry of the system.
  * Its purpose is to execute and monitor all the tasks needed to complete a full cycle.
  *
  * Two tasks run in parallel (in separate threads) using the Round-robin scheduling policy.
  * Also, all both tasks share the same core (CPU affinity is 1).
  *
  * These tasks, are:
  * 1. The MAIN task, that calls all the subtasks which execute the main functionality.
  *    For the system to be cyclic executive, all of its subtasks are executed in a circular order.
  *    Each subtask has a specific time slice (interval) under which it has to perform its function,
  *    otherwise it is preempted.
  *    All the time intervals, should total to the cycle time of the system (e.g. 40 ms).
  *
  * 2. The CAN_IRQ task, that handles the interrupts and implements the appropriate ISRs.
  *    The MCP2515 CAN controller is hosted in BCM2837 using the GPIO interface via SPI.
  *    Inside Linux, a CAN driver is implemented to handle the communication.
  *    It is implemented as a network driver in the user-space using the preexisting SocketCan.
  *    All the CAN frames received, go into a callback and store the data to an appropriate buffer.
  *    Later, a scheduled task prepares all the received data that is later inputted to the algorithm.
  *
  * In addition, the INIT task initializes the system and runs only once on startup.
  *
  * The MAIN task consists of 5 subtasks that are executed in a pipeline manner.
  * The most important subtasks are the RECV, ALGO and SEND.
  *
  * Finally, there is the NOOP (no operation) subtask, that is running when all
  * the other subtasks have finished, in order to achieve a stable cycle.
  *
  * No preemption is currently done when a task exceeds its predefined execution time.
  *
  * @author Dimitrios Panagiotis G. Geromichalos (dgeromichalos)
  * @date September, 2016
  */

/******************************** Inclusions *********************************/

#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

#include <sched.h>
#include <pthread.h>
#include <sys/mman.h>

#include "sched_statistics.h"

/***************************** Macro Definitions *****************************/

/** The CPU affinity of the process. */
#define NUM_CPUS (0u)

/** The priority that will be given to the created tasks (threads) from the OS.
  * Since the PRREMPT_RT uses 50 as the priority of kernel tasklets and
  * interrupt handlers by default, the maximum available priority is chosen.
  * The priority of each task should be the same, since the Round-Robin
  * scheduling policy is used and each task is executed with the same time slice.
  */
#define TASK_PRIORITY (49u)

/** This is the maximum size of the stack which is guaranteed safe access without faulting. */
#define MAX_SAFE_STACK (128u * 1024u)

/** The number of nsecs per sec. */
#define NSEC_PER_SEC (1000000000u)

/** The scheduler's tick interval: 250us */
#define SCHED_TICK (250000u)

/** The cycle time between the task calls (40ms). */
#define CYCLE_TIME ((SCHED_TICK) * 160u)

/**
  * @defgroup task_intervals The time intervals that each task is allowed to run.
  * @todo Represent all tasks and their intervals in an array!
  *
  * @{
  */
#define DUMMY_SUBTASK_INTERVAL ((SCHED_TICK) * 40u)

#define NOOP_SUBTASK_INTERVAL ((CYCLE_TIME) - (DUMMY_SUBTASK_INTERVAL))
/** @} */

/***************************** Static Variables ******************************/

/** The main timer of the scheduler. */
static struct timespec main_task_timer;

/******************** Static General Function Prototypes *********************/

/**
  * @brief Prefault the stack segment that belongs to this process.
  * @return Void.
  */
static void prefaultStack(void);

/**
  * @brief Update the scheduler's timer with a new interval.
  * @param interval The interval needed to perform the update.
  * @return Void.
  */
static void updateInterval(long interval);

/**
  * @brief Checks if a tasks has exceeded its predefined excecution time.
  * @details No preemption is done at the moment, only a warning is displayed.
  * @todo Print task name as well!
  * @return Void.
  */
/* static void check_task_execution_time(void); */

/**
  * @brief Execute a subtask.
  * @details Updates the scheduler's timer with the subtask's interval,
  *          calls the subtasks' routine and the sleeps for the remaining time.
  * @param task Function pointer to the subtask.
  * @todo Monitor and preempt the task if its time slice is exceeded!
  * @return Void.
  */
static void runTask(void (*task)(void), long interval);

/********************* Static Task Function Prototypes ***********************/

static void DUMMY_SUBTASK(void);
static void NOOP_SUBTASK(void);

static void INIT_TASK(void);
static void EXIT_TASK(void);

static void* MAIN_TASK(void* ptr);

/************************** Static General Functions *************************/

void prefaultStack(void)
{
        unsigned char dummy[MAX_SAFE_STACK];

        memset(dummy, 0, MAX_SAFE_STACK);

        return;
}

void updateInterval(long interval)
{
        main_task_timer.tv_nsec += interval;

        /* normalize time (when nsec have overflowed) */
        while (main_task_timer.tv_nsec >= NSEC_PER_SEC)
        {
                main_task_timer.tv_nsec -= NSEC_PER_SEC;
                main_task_timer.tv_sec++;
        }
}

/*
void check_task_execution_time(void)
{
        struct timespec current_timer;

        clock_gettime(CLOCK_MONOTONIC, &current_timer);

        if ((main_task_timer.tv_sec == current_timer.tv_sec) ?
                (main_task_timer.tv_nsec < current_timer.tv_nsec) : (main_task_timer.tv_sec < current_timer.tv_sec))
        {
                printf("Task exceeded its predefined execution time!");
        }
}
*/

void runTask(void (*task)(void), long interval)
{
        /* calculate next shot */
        updateInterval(interval);

        /* run the task */
        task();

        /* it should not exceed interval. no preemption is done at the moment!!! */
        /* check_task_execution_time(); */

        /* sleep for the remaining duration */
        (void)clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &main_task_timer, NULL);
}

/**************************** Static Task Functions **************************/

void DUMMY_SUBTASK(void)
{
        PrintStatistics();
}

void NOOP_SUBTASK(void)
{
}

/*****************************************************************************/

void INIT_TASK(void)
{
}

void* MAIN_TASK(void* ptr)
{
        /* Synchronize scheduler's timer. */
        clock_gettime(CLOCK_MONOTONIC, &main_task_timer);

        InitStatistics((float)CYCLE_TIME, main_task_timer.tv_nsec);

        while (1)
        {
                runTask(DUMMY_SUBTASK, DUMMY_SUBTASK_INTERVAL);
                runTask(NOOP_SUBTASK, NOOP_SUBTASK_INTERVAL);
        }

        return (void*)NULL;
}

void EXIT_TASK(void)
{
}

/********************************** Main Entry *******************************/

int main(void)
{
        cpu_set_t mask;

        pthread_t thread_1;
        pthread_attr_t attr_1;
        struct sched_param parm_1;

        /* pthread_t thread_2; */
        /* pthread_attr_t attr_2; */
        /* struct sched_param parm_2; */

        /*********************************************************************/

        /* Check if the task intervals have been set correctly. */
        if (NOOP_SUBTASK_INTERVAL < 0)
        {
                perror("total task intervals exceed cycle time!");
                exit(-1);
        }

        /* Lock memory. */
        if(mlockall(MCL_CURRENT|MCL_FUTURE) == -1)
        {
                perror("mlockall failed");
                exit(-2);
        }

        prefaultStack();

        CPU_ZERO(&mask);
        CPU_SET(NUM_CPUS, &mask);
        if (sched_setaffinity(0, sizeof(mask), &mask) == -1)
        {
                perror("Could not set CPU Affinity");
                exit(-3);
        }

        /*********************************************************************/

        INIT_TASK();

        /*********************************************************************/

        pthread_attr_init(&attr_1);
        pthread_attr_getschedparam(&attr_1, &parm_1);
        parm_1.sched_priority = TASK_PRIORITY;
        pthread_attr_setschedpolicy(&attr_1, SCHED_RR);
        pthread_attr_setschedparam(&attr_1, &parm_1);

        (void)pthread_create(&thread_1, &attr_1, (void*)MAIN_TASK, (void*)NULL);
        pthread_setschedparam(thread_1, SCHED_RR, &parm_1);

        /*********************************************************************/

        // pthread_attr_init(&attr_2);
        // pthread_attr_getschedparam(&attr_2, &parm_2);
        // parm_2.sched_priority = TASK_PRIORITY;
        // pthread_attr_setschedpolicy(&attr_2, SCHED_RR);
        // pthread_attr_setschedparam(&attr_2, &parm_2);

        // (void)pthread_create(&thread_2, &attr_2, (void*)CAN_IRQ_TASK, (void*)NULL);
        // pthread_setschedparam(thread_2, SCHED_RR, &parm_2);

        /*********************************************************************/

        pthread_join(thread_1, NULL);
        // pthread_join(thread_2, NULL);

        EXIT_TASK();

        /*********************************************************************/

        exit(0);
}

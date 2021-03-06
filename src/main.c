/**
  * @file main.c
  * @brief Contains the scheduler and the entry point for
  *        the implementation of RTTest on BCM2837.
  *
  * @author Dimitrios Panagiotis G. Geromichalos (geromidg@gmai.com)
  * @date August, 2017
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
  * scheduling policy is used and each task is executed with the same
  * time slice.
  */
#define TASK_PRIORITY (49u)

/** This is the maximum size of the stack which is
  * guaranteed safe access without faulting.
  */
#define MAX_SAFE_STACK (128u * 1024u)

/** The number of nsecs per sec/msec. */
#define NSEC_PER_SEC (1000000000ul)
#define NSEC_PER_MSEC (1000000ul)

/***************************** Static Variables ******************************/

/** The cycle time between the task calls. */
static u64_t cycle_time;

/** The number of cycles the program will run for. */
static u64_t cycle_num;

/** The saved timestamp of each sample. */
static f32_t* timestamps;

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
static void updateInterval(u64_t interval);

/********************* Static Task Function Prototypes ***********************/

static void INIT_TASK(int argc, char** argv);
static void* MAIN_TASK(void* ptr);
static void EXIT_TASK(void);

/************************** Static General Functions *************************/

void prefaultStack(void)
{
  unsigned char dummy[MAX_SAFE_STACK];

  memset(dummy, 0, MAX_SAFE_STACK);

  return;
}

void updateInterval(u64_t interval)
{
  main_task_timer.tv_nsec += interval;

  /* Normalize time (when nsec have overflowed) */
  while (main_task_timer.tv_nsec >= NSEC_PER_SEC)
  {
    main_task_timer.tv_nsec -= NSEC_PER_SEC;
    main_task_timer.tv_sec++;
  }
}

/***************************** Static Task Functions *************************/

void INIT_TASK(int argc, char** argv)
{
  if (argc != 3)
  {
    perror("Wrong number of arguments");
    exit(-4);
  }

  cycle_time = strtoul(argv[1], NULL, 0) * NSEC_PER_MSEC;
  cycle_num = strtoul(argv[2], NULL, 0);

  if (!(timestamps = (f32_t*) malloc(sizeof(f32_t) * cycle_num)))
  {
    perror("Memory allocation failed!");
    exit(-5);
  }
}

void* MAIN_TASK(void* ptr)
{
  u32_t i;

  /* Synchronize scheduler's timer. */
  clock_gettime(CLOCK_MONOTONIC, &main_task_timer);

  initStatistics((f32_t)cycle_time, main_task_timer.tv_nsec);

  for (i = 0; i < cycle_num; i++)
  {
    /* Calculate next shot */
    updateInterval(cycle_time);

    /* Store the timestamp */
    timestamps[i] = getTimestamp();

    /* Sleep for the remaining duration */
    (void)clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &main_task_timer, NULL);
  }

  FILE *file = fopen("timestamps.txt", "w");

  if (file != NULL)
  {
    printStatistics(NULL);  /* Print statistics to console */
    printStatistics(file);  /* Print statistics to file */

    printf("\n# Timestamps #\n");
    fprintf(file, "\n# Timestamps #\n");

    for (i = 0; i < cycle_num; i++)
    {
      printf("%.5f\n", timestamps[i]);
      fprintf(file, "%.5f\n", timestamps[i]);
    }

    fclose(file);
  }

  return (void*)NULL;
}

void EXIT_TASK(void)
{
  free(timestamps);
}

/********************************** Main Entry *******************************/

int main(int argc, char** argv)
{
  cpu_set_t mask;

  pthread_t thread_1;
  pthread_attr_t attr_1;
  struct sched_param parm_1;

  /***********************************/

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

  /***********************************/

  INIT_TASK(argc, argv);

  /***********************************/

  pthread_attr_init(&attr_1);
  pthread_attr_getschedparam(&attr_1, &parm_1);
  parm_1.sched_priority = TASK_PRIORITY;
  pthread_attr_setschedpolicy(&attr_1, SCHED_RR);
  pthread_attr_setschedparam(&attr_1, &parm_1);

  (void)pthread_create(&thread_1, &attr_1, (void*)MAIN_TASK, (void*)NULL);
  pthread_setschedparam(thread_1, SCHED_RR, &parm_1);

  /***********************************/

  pthread_join(thread_1, NULL);

  EXIT_TASK();

  /***********************************/

  exit(0);
}

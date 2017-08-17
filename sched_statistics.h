/**
  * @file sched_statistics.h
  * @brief Contains the declarations of functions defined in sched_statistics.c.
  *
  * @author Dimitrios Panagiotis G. Geromichalos (dgeromichalos)
  * @date September, 2016
  */

#ifndef SCHED_STATISTICS_H
#define SCHED_STATISTICS_H

#ifdef __cplusplus
extern "C" {
#endif

/******************************** Inclusions *********************************/

#include "data_types.h"

/***************************** Public Functions ******************************/

/**
  * @brief Initialize the statistics module.
  * @details This module should be initialized just before the main task
  *          is executed, so that the scheduler's timer is known.
  * @param cycle The cycle time of the system.
  * @param sched_timer_nsec The scheduler's timer in nsec.
  * @return Void.
  */
void InitStatistics(float cycle, s32_t sched_timer_nsec);

/**
  * @brief Print the scheduler's statistics to the screen.
  * @warning If the system's cycle time is too low, the buffer might overflow!
  * @return Void.
  */
void PrintStatistics(void);

/*****************************************************************************/

#ifdef __cplusplus
}
#endif

#endif  /* SCHED_STATISTICS_H */
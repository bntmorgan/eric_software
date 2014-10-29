#ifndef __HAL_SLEEP_H
#define __HAL_SLEEP_H

#include <stdint.h>

enum sleep_error {
  SLEEP_OK,
  SLEEP_OVERFLOW
};

void sleep_isr(void);
int usleep(uint32_t usec);
/**
 * Wait for an event b (boolean) for usec useconds
 * if b occured then o = b else o = 0 for timeout
 */
int usleep_until(uint32_t usec, int *b, int *o);

#endif /* __HAL_SLEEP_H */

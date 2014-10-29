#include <challenge.h>
#include <hw/sysctl.h>
#include <hal/time.h>
#include <hal/sleep.h>
#include <stdint.h>
#include <stdio.h>
#include <hm.h>
#include <hw/hm.h>

#define ABS(x) (((x) < 0) ? -(x) : (x))

#define CHALLENGE_PERIOD 5000000 // 5 secondes
#define CHALLENGE_TIME 10000 // Expected execution time

static inline void wait(void) {
  int z, k;
  for (k=0; k < 0x100; k++)
    for (z=0; z< 0x100; z++)
      asm("nop;");
}

void time_tick(void) {
}

// XXX
uint8_t s;
void challenge_init(void) {
  // XXX
  s = 0;
  time_init();
  hm_init();
}

void challenge_run(void) {
  int ok;
  struct timestamp t;
  hm_enable_irq();
  while (1) {
    s++;
    if (s > 20) {
      break;
    }
    // Wait for expansion rom reading
    usleep_until(CHALLENGE_PERIOD, &hm_read_exp, &ok);
    hm_read_exp = 0;
    if (!ok) {
      printf("Host didn't read hm expansion rom\n");
      break;
    }
    // Wait for BAR write
    usleep_until(CHALLENGE_TIME, &hm_write_bar, &ok);
    hm_write_bar = 0;
    if (!ok) {
      printf("Host didn't write the BARs\n");
      break;
    }
    // Test the results
    // Succeded
    time_get(&t);
    printf("Uptime : Secs : 0x%08x ; USecs 0x%08x\n", t.sec, t.usec);
  }
  hm_disable_irq();
}

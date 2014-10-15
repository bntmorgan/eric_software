#include <stdio.h>
#include <stdint.h>
#include <hw/sysctl.h>
#include <hw/interrupts.h>
#include <hal/sleep.h>
#include <irq.h>

static uint8_t reached;

void sleep_isr(void) {
  reached = 1;
  irq_ack(IRQ_TIMER1);
}

void sleep_init(uint32_t compare) {
	unsigned int mask;
	
	// printf("SLEEP: system timer start\n");

	irq_ack(IRQ_TIMER1);
	CSR_TIMER1_COUNTER = 0;
	CSR_TIMER1_COMPARE = compare;
	CSR_TIMER1_CONTROL = TIMER_ENABLE;

	mask = irq_getmask();
	mask |= IRQ_TIMER1;
	irq_setmask(mask);
}

void sleep_off(void) {
	unsigned int mask;

	mask = irq_getmask();
	mask &= ~IRQ_TIMER1;
	irq_setmask(mask);

	// printf("SLEEP: system timer stopped\n");
}

int usleep(uint32_t usec) {
  uint64_t counter = (CSR_FREQUENCY / 1000000) * usec;
  if (counter > (uint32_t)-1) {
    printf("Unable to load the timer, counter too high\n");
    return SLEEP_OVERFLOW;
  }
  sleep_init((uint32_t)counter);
  while (!reached);
  sleep_off();
  reached = 0;
  return SLEEP_OK;
}

int usleep_until(uint32_t usec, int *b, int *o) {
  uint64_t counter = (CSR_FREQUENCY / 1000000) * usec;
  if (counter > (uint32_t)-1) {
    printf("Unable to load the timer, counter too high\n");
    return SLEEP_OVERFLOW;
  }
  sleep_init((uint32_t)counter);
  while (1) {
    if (*b) {
      *o = *b;
      break;
    }
    if (reached) {
      *o = 0;
      break;
    }
  }
  sleep_off();
  reached = 0;
  return SLEEP_OK;
}

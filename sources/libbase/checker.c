#include <hw/checker.h>
#include <hw/interrupts.h>
#include <stdio.h>
#include <irq.h>

int cend = 0;

void wait(void) {
  int z, k;
  for (k=0; k < 0x100; k++) {
    for (z=0; z< 0x100; z++) {
      asm("nop;");
    }
  }
}

void checker_init(void) {
  int mask;

	irq_ack(IRQ_CHECKER);

	mask = irq_getmask();
	mask |= IRQ_CHECKER;
	irq_setmask(mask);
}

void checker_isr(void) {
  cend = 1;

  // Clear the events and so the interrupt
  CHECKER_CSR_STAT = CHECKER_STAT_EVENT_DONE | CHECKER_STAT_EVENT_ERROR;

	irq_ack(IRQ_CHECKER);
}

void checker_dummy_start(int low_value, int high_value) {
  int stat, ctrl, low, high, irqs, mask;

  // Reset cend
  cend = 0;

  // Set the values : Actually address is use like a counter
  CHECKER_CSR_ADDRESS_LOW = low_value;
  CHECKER_CSR_ADDRESS_HIGH = high_value;

  // Read the values to be sure
  low = CHECKER_CSR_ADDRESS_LOW;
  high = CHECKER_CSR_ADDRESS_HIGH;

  printf("Low 0x%08x, high 0x%0x\n", low, high);

  // Enable interrupts and start checker in dummy mode
  ctrl = CHECKER_CTRL_IRQ_EN | CHECKER_CTRL_MODE_DUMMY |
    CHECKER_CTRL_START;
  printf("start CTRL 0x%08x\n", ctrl);
  // Ziehen !
  CHECKER_CSR_CTRL = ctrl; 

  // Wait for the end of time lulz
  while (1) {
    stat = CHECKER_CSR_STAT;
    ctrl = CHECKER_CSR_CTRL;
    printf("STAT 0x%08x, CTRL 0x%08x\n", stat, ctrl);
    irqs = irq_pending();
    mask = irq_getmask();
    printf("IRQs 0x%08x, MASK 0x%08x\n", irqs, mask);

    if (cend) {
      printf("This is finished !\n");
      break;
    } else {
      printf("Waiting again !\n");
      wait();
    }
  }
}

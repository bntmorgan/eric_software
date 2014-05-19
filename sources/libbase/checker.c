#include <hw/checker.h>
#include <hw/interrupts.h>
#include <stdio.h>
#include <irq.h>
#include <checker_int.h>

static int checker_end;
static int checker_error;

void wait(void) {
  int z, k;
  for (k=0; k < 0x100; k++) {
    for (z=0; z< 0x100; z++) {
      __asm__ __volatile__("nop;");
    }
  }
}

void checker_init(void) {
  int mask;

  checker_end = 0;
  checker_error = 0;

	irq_ack(IRQ_CHECKER);

	mask = irq_getmask();
	mask |= IRQ_CHECKER;
	irq_setmask(mask);
}

void checker_isr(void) {
  int checker_stat = CHECKER_CSR_STAT;

  if (checker_stat & CHECKER_STAT_EVENT_USER_IRQ) { 
    checker_int_enqueue(CHECKER_CSR_MODE_DATA_LOW);
  } else if (checker_stat & CHECKER_STAT_EVENT_DONE) {
    checker_end = 1;
  } else if (checker_stat & CHECKER_STAT_EVENT_ERROR) {
    checker_end = 1;
    checker_error = 1;
  }

  // Clear the events and so the interrupt
  CHECKER_CSR_STAT = CHECKER_STAT_EVENT_DONE | CHECKER_STAT_EVENT_USER_IRQ |
    CHECKER_STAT_EVENT_ERROR;
	irq_ack(IRQ_CHECKER);
}

void checker_print_hm_stat(void) {
  int stat_trn, stat_trn_cpt_rx, stat_trn_cpt_tx;
  stat_trn = CHECKER_CSR_STAT_TRN;
  stat_trn_cpt_rx = CHECKER_CSR_STAT_TRN_CPT >> 16;
  stat_trn_cpt_tx = CHECKER_CSR_STAT_TRN_CPT & 0xffff;
  printf("cpt_rx(0x%04x) cpt_tx(0x%04x)\n", stat_trn_cpt_rx, stat_trn_cpt_tx);
  printf("lnk_up(%01x), rsrc_rdy_n(%01x), tdst_rdy_n(%01x), "
      "trn_tbuf_av(%02x), state_b(%01x), state_a(%01x), state_rx(%01x), "
      "state_tx(%01x), cpt_drop(%02x)\n", (stat_trn >> 0) & 1,
      (stat_trn >> 1) & 1, (stat_trn >> 2) & 1, (stat_trn >> 3) & 0x3f,
      (stat_trn >> 9) & 3, (stat_trn >> 11) & 3, (stat_trn >> 13) & 3,
      (stat_trn >> 15) & 3, (stat_trn >> 17) & 0xff);
}

void checker_single_start(int low_value, int high_value) {
  int ctrl, low, high, ud, cpt = 0, stat_trn, stat_trn_cpt_rx, stat_trn_cpt_tx;

  // Reset checker_int
  checker_init();
  checker_int_init();

  // Set the values : Actually address is use like a counter
  CHECKER_CSR_ADDRESS_LOW = low_value;
  CHECKER_CSR_ADDRESS_HIGH = high_value;

  // Read the values to be sure
  low = CHECKER_CSR_ADDRESS_LOW;
  high = CHECKER_CSR_ADDRESS_HIGH;

  // Stops the last launch
  ctrl = 0x0;
  printf("ctrl stop checker : 0x%08x\n", ctrl);
  CHECKER_CSR_CTRL = ctrl;
  // Enable interrupts and start checker in dummy mode
  ctrl = CHECKER_CTRL_IRQ_EN | CHECKER_CTRL_MODE_SINGLE | CHECKER_CTRL_START;
  printf("start CTRL 0x%08x\n", ctrl);
  // Ziehen !
  CHECKER_CSR_CTRL = ctrl;
  printf("CTRL after start 0x%08x\n", CHECKER_CSR_CTRL);

  checker_print_hm_stat();

  // Wait for the end of time lulz
  while (1) {
    if (checker_end) {
      checker_int_iterate(1, NULL);
      while (!checker_int_iterate(0, &ud)) {
        printf("Iterated %x\n", ud);
      }
      if (checker_error) {
        printf("Stopped on error\n");
      }
      break;
    } else {
      stat_trn = CHECKER_CSR_STAT_TRN;
      stat_trn_cpt_rx = CHECKER_CSR_STAT_TRN_CPT >> 16;
      stat_trn_cpt_tx = CHECKER_CSR_STAT_TRN_CPT & 0xffff;
      printf("Waiting again : %08x\n", cpt);
      checker_print_hm_stat();
      wait();
      if (cpt < 5) {
        cpt++;
      } else {
        printf("Number of waits exeeded\n");
        break;
      }
    }
  }
}

void checker_dummy_start(int low_value, int high_value) {
  int ctrl, low, high, ud, cpt = 0;

  // Reset checker_int
  checker_init();
  checker_int_init();

  // Set the values : Actually address is use like a counter
  CHECKER_CSR_ADDRESS_LOW = low_value;
  CHECKER_CSR_ADDRESS_HIGH = high_value;

  // Read the values to be sure
  low = CHECKER_CSR_ADDRESS_LOW;
  high = CHECKER_CSR_ADDRESS_HIGH;

  printf("Low 0x%08x, high 0x%0x\n", low, high);

  // Stops the last launch
  ctrl = 0x0;
  printf("ctrl stop checker : 0x%08x\n", ctrl);
  // Enable interrupts and start checker in dummy mode
  ctrl = CHECKER_CTRL_IRQ_EN | CHECKER_CTRL_MODE_DUMMY | CHECKER_CTRL_START;
  printf("start CTRL 0x%08x\n", ctrl);
  // Ziehen !
  CHECKER_CSR_CTRL = ctrl; 
  printf("CTRL after start 0x%08x\n", CHECKER_CSR_CTRL);

  // Wait for the end of time lulz
  while (1) {
    if (checker_end) {
      checker_int_iterate(1, NULL);
      while (!checker_int_iterate(0, &ud)) {
        printf("Iterated %x\n", ud);
      }
      if (checker_error) {
        printf("Stopped on error\n");
      }
      break;
    } else {
      printf("Waiting again !\n");
      wait();
      if (cpt < 40) {
        cpt++;
      } else {
        printf("Number of waits exeeded\n");
        break;
      }
    }
  }
}

/**
 * Memory
 */

int test_write(int *addr, int value) {
  *addr = value;
  if (*addr == value) {
    return 1;
  } else {
    printf("TEST WRITE FAILED :(((\n");
    return 0;
  }
}

int test_read(int *addr, int value) {
  printf("test_read: 0x%08x\n", *addr);
  if (*addr == value) {
    return 1;
  } else {
    printf("TEST WRITE FAILED :(((\n");
    return 0;
  }
}

void checker_memory_test(void) {
  printf("CHECKER: Wishbone checker memory read/write test\n");
  int *ptr;
  int j, ok=1;
  ptr = (int *)0x20000000;
  for (j = 0; j < 0x100; j++) {
    if (!test_write(ptr, 0xcacacaca)) {
      ok = 0;
      break;
    }
    ptr++;
  }
  ptr = (int *)0x20000000;
  for (j = 0; j < 0x100; j++) {
    if (!test_write(ptr, 0x00000000)) {
      ok = 0;
      break;
    }
    ptr++;
  }
  if (ok) {
    printf("CHECKER: passed\n");
  }
}

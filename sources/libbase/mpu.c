#include <hw/mpu.h>
#include <hw/interrupts.h>
#include <stdio.h>
#include <irq.h>
#include <mpu_int.h>

static int mpu_end;
static int mpu_error;

static void wait(void) {
  int z, k;
  for (k=0; k < 0x100; k++) {
    for (z=0; z< 0x100; z++) {
      __asm__ __volatile__("nop;");
    }
  }
}

void mpu_init(void) {
  int mask;

  mpu_end = 0;
  mpu_error = 0;

	irq_ack(IRQ_MPU);

	mask = irq_getmask();
	mask |= IRQ_MPU;
	irq_setmask(mask);
}

void mpu_isr(void) {
  int mpu_stat = MPU_CSR_STAT;

  if (mpu_stat & MPU_STAT_EVENT_USER_IRQ) { 
    mpu_int_enqueue(MPU_CSR_USER_DATA_LOW);
  } else if (mpu_stat & MPU_STAT_EVENT_DONE) {
    mpu_end = 1;
  } else if (mpu_stat & MPU_STAT_EVENT_ERROR) {
    mpu_end = 1;
    mpu_error = 1;
  }

  // Clear the events and so the interrupt
  MPU_CSR_STAT = MPU_STAT_EVENT_DONE | MPU_STAT_EVENT_USER_IRQ |
    MPU_STAT_EVENT_ERROR;
	irq_ack(IRQ_MPU);
}

void mpu_start(void) {
  int ctrl, ud, cpt = 0;

  // Reset mpu_int
  mpu_init();
  mpu_int_init();

  // Stops the last launch
  ctrl = 0x0;
  printf("ctrl stop mpu : 0x%08x\n", ctrl);
  MPU_CSR_CTRL = ctrl;
  // Enable interrupts and start mpu in single mode
  ctrl = MPU_CTRL_IRQ_EN | MPU_CTRL_START;
  printf("start CTRL 0x%08x\n", ctrl);
  // Ziehen !
  MPU_CSR_CTRL = ctrl;
  printf("CTRL after start 0x%08x\n", MPU_CSR_CTRL);

  // Wait for the end of time lulz
  while (1) {
    if (mpu_end) {
      mpu_int_iterate(1, NULL);
      while (!mpu_int_iterate(0, &ud)) {
        printf("Iterated %x\n", ud);
      }
      if (mpu_error) {
        printf("Stopped on error\n");
      }
      break;
    } else {
      wait();
      printf("STAT %08x\n", MPU_CSR_STAT);
      if (cpt < 20) {
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

void mpu_memory_test(void) {
  printf("MPU: Wishbone mpu memory read/write test\n");
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
    printf("MPU: passed\n");
  }
}

#include <hw/mpu.h>
#include <hw/interrupts.h>
#include <stdio.h>
#include <irq.h>
#include <mpu_int.h>

static int mpu_end;
static int mpu_error;

static void wait(void) {
  int /*z,*/ k;
  for (k=0; k < 0x1000; k++) {
    // for (z=0; z< 0x10000; z++) {
      __asm__ __volatile__("nop;");
    // }
  }
}

void mpu_init(void) {
  mpu_end = 0;
  mpu_error = 0;

	irq_ack(IRQ_MPU);
}

void mpu_enable_irq(void) {
  int mask, ie;

	mask = irq_getmask();
	mask |= IRQ_MPU;
	irq_setmask(mask);

  ie = irq_getie();
  ie |= IRQ_MPU;
  irq_setie(ie);
}

void mpu_disable_irq(void) {
  int mask, ie;

  ie = irq_getie();
  ie &= ~IRQ_MPU;
  irq_setie(ie);

	mask = irq_getmask();
	mask &= ~IRQ_MPU;
	irq_setmask(mask);
}

void mpu_isr(void) {
  int mpu_stat = MPU_CSR_STAT;

  if (mpu_stat & MPU_STAT_EVENT_USER_IRQ) { 
    mpu_int_enqueue(MPU_CSR_USER_DATA_LOW, MPU_CSR_USER_DATA_HIGH);
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

int mpu_start(void) {
  int ctrl, cpt = 0;

  // Reset mpu_int
  mpu_init();
  mpu_int_init();

  // Stops the last launch
  ctrl = 0x0;
  // printf("ctrl stop mpu : 0x%08x\n", ctrl);
  MPU_CSR_CTRL = ctrl;
  // Enable interrupts and start mpu in single mode
  ctrl = MPU_CTRL_IRQ_EN | MPU_CTRL_START;
  // printf("start CTRL 0x%08x\n", ctrl);
  // Ziehen !
  MPU_CSR_CTRL = ctrl;
  // printf("CTRL after start 0x%08x\n", MPU_CSR_CTRL);

  mpu_enable_irq();

  // Wait for the end of time lulz
  while (1) {
    if (mpu_end) {
      break;
    } else {
      wait();
      // printf("STAT %08x\n", MPU_CSR_STAT);
      if (cpt < 20) {
        cpt++;
      } else {
        break;
      }
    }
  }

  mpu_disable_irq();
  if (mpu_error) {
    printf("Stopped on error\n");
    return -1;
  } else if (cpt == 20) {
    printf("Number of waits exeeded\n");
    return -2;
  }
  return 0;
}

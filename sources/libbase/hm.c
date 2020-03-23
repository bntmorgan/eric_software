#include <hw/hm.h>
#include <hm.h>
#include <hw/interrupts.h>
#include <hw/sysctl.h>
#include <stdio.h>
#include <irq.h>
#include <stdint.h>

static int hm_end;
static int hm_rx_timeout;
static int hm_tx_timeout;
static int hm_wr_timeout;
int hm_read_exp;
int hm_write_bar;
uint32_t hm_read_exp_time;
uint32_t hm_write_bar_time;

static void wait(void) {
  int /* z, */k;
  for (k=0; k < 0x1000; k++) {
    // for (z=0; z< 0x100; z++) {
      __asm__ __volatile__("nop;");
    // }
  }
}

void hm_init(void) {
  hm_end = 0;
  hm_rx_timeout = 0;
  hm_tx_timeout = 0;
  hm_read_exp = 0;
  hm_write_bar = 0;

  HM_CSR_STAT = 0xffffffff;

  irq_ack(IRQ_HM);
}

void hm_set_bar_bitmap(uint32_t bar_bitmap) {
  HM_CSR_BAR_BITMAP = bar_bitmap;
}

void hm_enable_irq(void) {
  int mask, ie;

	mask = irq_getmask();
	mask |= IRQ_HM;
	irq_setmask(mask);

  ie = irq_getie();
  ie |= IRQ_HM;
  irq_setie(ie);

  HM_CSR_CTRL |= HM_CTRL_IRQ_EN;
}

void hm_disable_irq(void) {
  int mask, ie;

  HM_CSR_CTRL &= ~HM_CTRL_IRQ_EN;

  ie = irq_getie();
  ie &= ~IRQ_HM;
  irq_setie(ie);

	mask = irq_getmask();
	mask &= ~IRQ_HM;
	irq_setmask(mask);
}

void hm_isr(void) {
  int stat = HM_CSR_STAT;

  if (stat & HM_STAT_EVENT_RX_TIMEOUT) {
    hm_rx_timeout = 1;
    hm_end = 1;
  }
  if (stat & HM_STAT_EVENT_TX_TIMEOUT) {
    hm_tx_timeout = 1;
    hm_end = 1;
  }
  if (stat & HM_STAT_EVENT_WR_TIMEOUT) {
    hm_wr_timeout = 1;
    hm_end = 1;
  }
  if (stat & HM_STAT_EVENT_DONE) {
    hm_end = 1;
  }
  if (stat & HM_STAT_EVENT_READ_EXP) {
    hm_read_exp = 1;
  }
  if (stat & HM_STAT_EVENT_WRITE_BAR) {
    hm_write_bar = 1;
  }

  // Clear the events and so the interrupt
  HM_CSR_STAT = stat;

  irq_ack(IRQ_HM);
}

void hm_service(void) {
  if (irq_pending() & IRQ_HM) {
    hm_isr();
  }
}


void hm_stop_all(void) {
  hm_init();
  HM_CSR_CTRL = 0x0;
}

int hm_start_read(int low, int high) {
  int ctrl, cpt = 0;

  // Reset hm_int
  hm_init();

  // Stops the last launch
  ctrl = 0x0;

  // Write the address to read
  HM_CSR_ADDRESS_LOW = low;
  HM_CSR_ADDRESS_HIGH = high;

  // printf("ctrl stop hm : 0x%08x\n", ctrl);
  HM_CSR_CTRL = ctrl;
  // Enable interrupts and start hm in single mode
  ctrl = HM_CTRL_IRQ_EN | HM_CTRL_START_READ /* | HM_CTRL_NON_SNOOP */;
  // Ziehen !
  HM_CSR_CTRL = ctrl;

  // Wait for the end of time lulz
  while (1) {
    hm_service();
    if (hm_end) {
      if (hm_rx_timeout) {
        printf("Cpt 0x%x, Stopped on rx timeout\n", cpt);
        return HM_ERROR_RX_TIMEOUT;
      } else if (hm_tx_timeout) {
        printf("Cpt 0x%x, Stopped on tx timeout \n", cpt);
        return HM_ERROR_TX_TIMEOUT;
      }
      return HM_OK;
    } else {
      wait();
      if (cpt < 0x3ff) {
        cpt++;
      } else {
        printf("Number of waits exeeded\n");
        return HM_ERROR_TIMEOUT;
      }
    }
  }
}

int hm_start_write_pwn(int low, int high, int data) {
  // Reset hm_int
  hm_init();

  // Reset hardware
  HM_CSR_CTRL = 0x0;

  // Write address
  HM_CSR_ADDRESS_LOW = low;
  HM_CSR_ADDRESS_HIGH = high;
  // Data to write
  HM_CSR_DATA = data;

  // Print what to do
  printf("low : 0x%08x, high : 0x%08x, data : 0x%08x\n",
      HM_CSR_ADDRESS_LOW, HM_CSR_ADDRESS_HIGH, HM_CSR_DATA);

  HM_CSR_CTRL = HM_CTRL_IRQ_EN | HM_CTRL_START_WRITE | HM_CTRL_LOCK_WRITE;

  return HM_OK;
}

int hm_start_write(int low, int high, int data, int no_snoop) {
  int ctrl, cpt = 0;

  // Reset hm_int
  hm_init();

//   printf("low : 0x%08x, high : 0x%08x, data : 0x%08x\n", HM_CSR_ADDRESS_LOW,
//       HM_CSR_ADDRESS_HIGH, HM_CSR_DATA);

  // Stops the last launch : reset the hardware
  ctrl = 0x0;

  if (no_snoop) {
    ctrl |= HM_CTRL_NO_SNOOP;
  }

//   printf("ctrl stop hm : 0x%08x\n", ctrl);
  HM_CSR_CTRL = ctrl;

  // Write the address to read
  HM_CSR_ADDRESS_LOW = low;
  HM_CSR_ADDRESS_HIGH = high;
  HM_CSR_DATA = data;

  // Override ID haha lol
  HM_CSR_ID = 0x0100;
  // Enable interrupts and start hm in single mode
  ctrl = HM_CTRL_IRQ_EN | HM_CTRL_START_WRITE;
//   printf("start CTRL 0x%08x\n", ctrl);
  // Ziehen !
  HM_CSR_CTRL = ctrl;
//   printf("CTRL after start 0x%08x\n", HM_CSR_CTRL);

  // Wait for the end of time lulz
  while (1) {
    hm_service();
    if (hm_end) {
      if (hm_wr_timeout) {
        printf("Stopped on wr timeout\n");
        return HM_ERROR_WR_TIMEOUT;
      }
//       printf("Write finished !\n");
      return HM_OK;
    } else {
      wait();
      if (cpt < 0x1ff) {
        cpt++;
      } else {
        printf("Number of waits exeeded\n");
        return HM_ERROR_TIMEOUT;
      }
    }
  }
}

void hm_stat(void) {
  printf("ADDR low (@0x%08x), ADDR high (@0x%08x)\n", HM_CSR_ADDRESS_LOW,
      HM_CSR_ADDRESS_HIGH);
  printf("STAT (0x%x), RX cpt (0x%x), RX state (0x%x), TX cpt (0x%x), TX "
      "state (0x%x), state(0x%x), \n    RX TLP DW (0x%x), CPT TX DROP(0x%x), "
      "TX ERROR (0x%x), CPT TX START(0x%x)\n",
      HM_CSR_STAT, HM_CSR_CPT_RX, HM_CSR_STATE_RX, HM_CSR_CPT_TX,
      HM_CSR_STATE_TX, HM_CSR_STATE_TX, HM_CSR_RX_TLP_DW, HM_CSR_CPT_TX_DROP,
      HM_CSR_TX_ERROR, HM_CSR_CPT_TX_START);
}

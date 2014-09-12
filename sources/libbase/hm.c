#include <hw/hm.h>
#include <hm.h>
#include <hw/interrupts.h>
#include <stdio.h>
#include <irq.h>

static int hm_end;
static int hm_rx_timeout;
static int hm_tx_timeout;

static void wait(void) {
  int /* z, */k;
  for (k=0; k < 0x100; k++) {
    // for (z=0; z< 0x100; z++) {
      // __asm__ __volatile__("nop;");
    // }
  }
}

void hm_init(void) {
  hm_end = 0;
  hm_rx_timeout = 0;
  hm_tx_timeout = 0;

	irq_ack(IRQ_HM);
}

void hm_service(void) {
  int stat = HM_CSR_STAT;

  if (irq_pending() & IRQ_HM) {
    if (stat & HM_STAT_EVENT_RX_TIMEOUT) { 
      hm_rx_timeout = 1;
      hm_end = 1;
    } else if (stat & HM_STAT_EVENT_TX_TIMEOUT) {
      hm_tx_timeout = 1;
      hm_end = 1;
    } else if (stat & HM_STAT_EVENT_DONE) {
      hm_end = 1;
    }

    // Clear the events and so the interrupt
    HM_CSR_STAT = HM_STAT_EVENT_DONE | HM_STAT_EVENT_RX_TIMEOUT |
      HM_STAT_EVENT_TX_TIMEOUT;

	  irq_ack(IRQ_HM);
  }
}

int hm_start(int low, int high) {
  int ctrl, cpt = 0;

  // Reset hm_int
  hm_init();

  // Write the address to read
  HM_CSR_ADDRESS_LOW = low;
  HM_CSR_ADDRESS_HIGH = high;

  // printf("low : 0x%08x, high : 0x%08x\n", HM_CSR_ADDRESS_LOW,
  //     HM_CSR_ADDRESS_HIGH);

  // Stops the last launch
  ctrl = 0x0;
  // printf("ctrl stop hm : 0x%08x\n", ctrl);
  HM_CSR_CTRL = ctrl;
  // Enable interrupts and start hm in single mode
  ctrl = HM_CTRL_IRQ_EN | HM_CTRL_START;
  // printf("start CTRL 0x%08x\n", ctrl);
  // Ziehen !
  HM_CSR_CTRL = ctrl;
  // printf("CTRL after start 0x%08x\n", HM_CSR_CTRL);

  // Wait for the end of time lulz
  while (1) {
    hm_service();
    if (hm_end) {
      if (hm_rx_timeout) {
        printf("Stopped on rx timeout\n");
        return HM_ERROR_RX_TIMEOUT;
      } else if (hm_tx_timeout) {
        printf("Stopped on tx timeout\n");
        return HM_ERROR_TX_TIMEOUT;
      }
      // printf("Read finished !\n");
      return HM_OK;
    } else {
      wait();
      if (cpt < 4) {
        cpt++;
      } else {
        printf("Number of waits exeeded\n");
        return HM_ERROR_TIMEOUT;
      }
    }
  }
}

void hm_stat(void) {
  printf("STAT (0x%x), RX cpt (0x%x), RX state (0x%x), TX cpt (0x%x), TX "
      "state (0x%x)\n", HM_CSR_STAT, HM_CSR_CPT_RX, HM_CSR_STATE_RX,
      HM_CSR_CPT_TX, HM_CSR_STATE_TX);
}

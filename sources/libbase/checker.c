#include <hw/checker.h>
#include <hw/interrupts.h>
#include <stdio.h>
#include <irq.h>
#include <checker_int.h>

static int checker_end;
static int checker_error;

static void wait(void) {
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

void checker_single_start(void) {
  int ctrl, ud, cpt = 0, stat_trn, stat_trn_cpt_rx, stat_trn_cpt_tx;

  // Reset checker_int
  checker_init();
  checker_int_init();

  // Stops the last launch
  ctrl = 0x0;
  printf("ctrl stop checker : 0x%08x\n", ctrl);
  CHECKER_CSR_CTRL = ctrl;
  // Enable interrupts and start checker in single mode
  ctrl = CHECKER_CTRL_IRQ_EN | CHECKER_CTRL_MODE_SINGLE | CHECKER_CTRL_START;
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
      stat_trn = CHECKER_CSR_STAT_TRN;
      stat_trn_cpt_rx = CHECKER_CSR_STAT_TRN_CPT >> 16;
      stat_trn_cpt_tx = CHECKER_CSR_STAT_TRN_CPT & 0xffff;
      printf("Waiting again : %08x\n", cpt);
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

void checker_read_start(int low_value, int high_value) {
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
  // Enable interrupts and start checker in read mode
  ctrl = CHECKER_CTRL_IRQ_EN | CHECKER_CTRL_MODE_READ | CHECKER_CTRL_START;
  printf("start CTRL 0x%08x\n", ctrl);
  // Ziehen !
  CHECKER_CSR_CTRL = ctrl;
  printf("CTRL after start 0x%08x\n", CHECKER_CSR_CTRL);

  // Wait for the end of time lulz
  while (1) {
    printf("STAT after start 0x%08x\n", CHECKER_CSR_STAT);
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
      wait();
      if (cpt < 10) {
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

/**
 * PCI config space decoding
 */
void checker_pci_dump_command(void) {
  pci_command_t f = {CHECKER_CSR_CFG_COMMAND}; 
  printf("COMMAND : raw(0x%02x), io_space_enable(%d), memory_space_enable(%d),\n"
      "bus_master_enable(%d), parity_error_response(%d), serr_enable(%d),\n"
      "interrupt_disable(%d)\n", f.raw, f.io_space_enable,
      f.memory_space_enable, f.bus_master_enable, f.parity_error_response,
      f.serr_enable, f.interrupt_disable);
}

void checker_pci_dump_dstatus(void) {
  pci_dstatus_t f = {CHECKER_CSR_CFG_DSTATUS}; 
  printf("DSTATUS : transaction_pending(%x), aux_power_detected(%x),\n"
      "unsupported_request_detected(%x), fatal_error_detected(%x),\n"
      "non_fatal_error_detected(%x), correctable_error_detected(%x)\n",
      f.transaction_pending, f.aux_power_detected,
      f.unsupported_request_detected, f.fatal_error_detected,
      f.non_fatal_error_detected, f.correctable_error_detected);
}

void checker_pci_dump_dcommand(void) {
  pci_dcommand_t f = {CHECKER_CSR_CFG_DCOMMAND}; 
  printf("DCOMMAND : max_read_request_size(0x%x), enable_no_snoop(%x),\n"
      "auxiliary_power_pm_enable(%x), phantom_fonctions_enable(%x),\n"
      "extended_tag_field_enable(%x), max_payload_size(0x%x),\n"
      "enable_relaxed_ordering(%x), unsupported_request_reporting_enable(%x),\n"
      "fatal_error_reporting_enable(%x), non_fatal_error_reporting_enable(%x),\n"
      "correctable_error_reporting_enable(%x)\n", f.max_read_request_size,
      f.enable_no_snoop, f.auxiliary_power_pm_enable,
      f.phantom_fonctions_enable, f.extended_tag_field_enable,
      f.max_payload_size, f.enable_relaxed_ordering,
      f.unsupported_request_reporting_enable, f.fatal_error_reporting_enable,
      f.non_fatal_error_reporting_enable, f.correctable_error_reporting_enable);
}

void checker_pci_dump_lstatus(void) {
  pci_lstatus_t f = {CHECKER_CSR_CFG_LSTATUS}; 
  printf("LSTATUS : link_autonomous_bandwidth_status(%x),\n"
      "link_bandwidth_management_status(%x), data_link_layer_link_active(%x),\n"
      "slot_clock_configuration(%x), link_training(%x),\n"
      "negociated_link_width(0x%x), current_link_speed(0x%x)\n",
      f.link_autonomous_bandwidth_status, f.link_bandwidth_management_status,
      f.data_link_layer_link_active, f.slot_clock_configuration,
      f.link_training, f.negociated_link_width, f.current_link_speed);
}

void checker_pci_dump_lcommand(void) {
  pci_lcommand_t f = {CHECKER_CSR_CFG_LCOMMAND}; 
  printf("LCOMMAND : link_autonomous_bandwidth_interrupt_enable(%x),\n"
      "link_bandwidth_management_interrupt_enable(%x),\n"
      "hardware_autonomous_width_disable(%x),\n"
      "enable_clock_power_management(%x), extended_synch(%x),\n"
      "common_clock_configuration(%x), retrain_link(%x), link_disable(%x),\n"
      "read_completion_boundary(%x), active_state_link_pm_control(0x%x)\n",
      f.link_autonomous_bandwidth_interrupt_enable,
      f.link_bandwidth_management_interrupt_enable,
      f.hardware_autonomous_width_disable, f.enable_clock_power_management,
      f.extended_synch, f.common_clock_configuration, f.retrain_link,
      f.link_disable, f.read_completion_boundary,
      f.active_state_link_pm_control);
}

void checker_pci_dump_dcommand2(void) {
  pci_dcommand2_t f = {CHECKER_CSR_CFG_DCOMMAND2}; 
  printf("DCOMMAND2 : completion_timeout_disable(0x%x),\n"
      "completion_timeout_value(0x%x)\n", f.completion_timeout_disable,
      f.completion_timeout_value);
}

void checker_pci_dump_address(void) {
  unsigned int pci_addr = CHECKER_CSR_CFG_PCI_ADDR;
  printf("Device addr %02x:%02x.%01x\n", (pci_addr >> 8) & 0xff, (pci_addr >>
        3) & 0x1f, (pci_addr >> 0) & 0x7);
}

void checker_fc_dump(void) {
  printf(
      "completion_data(0x%x), "
      "completion header(0x%x), "
      "non_posted_data(0x%x), "
      "non_posted_header(0x%x), "
      "posted_data(0x%x), "
      "posted_header(0x%x)\n", CHECKER_CSR_TRN_FC_CPLD,
      CHECKER_CSR_TRN_FC_CPLH, CHECKER_CSR_TRN_FC_NPD, CHECKER_CSR_TRN_FC_NPH,
      CHECKER_CSR_TRN_FC_PD, CHECKER_CSR_TRN_FC_PH);
}

void checker_fc_dump_receive_buffer_available_space(void) {
  printf("receive_buffer_available_space : \n  ");
  CHECKER_CSR_TRN_FC_SEL = CHECKER_TRN_FC_SEL_RBAS;
  checker_fc_dump();
}

void checker_fc_dump_receive_credits_granted_to_the_link_partner(void) {
  printf("receive_credits_granted_to_the_link_partner : \n  ");
  CHECKER_CSR_TRN_FC_SEL = CHECKER_TRN_FC_SEL_RCGTTLP;
  checker_fc_dump();
}

void checker_fc_dump_receive_credits_consumed(void) {
  printf("receive_credits_consumed : \n  ");
  CHECKER_CSR_TRN_FC_SEL = CHECKER_TRN_FC_SEL_RCC;
  checker_fc_dump();
}

void checker_fc_dump_transmit_user_credits_available(void) {
  printf("transmit_user_credits_available : \n  ");
  CHECKER_CSR_TRN_FC_SEL = CHECKER_TRN_FC_SEL_TUCA;
  checker_fc_dump();
}

void checker_fc_dump_transmit_credit_limit(void) {
  printf("transmit_credit_limit : \n  ");
  CHECKER_CSR_TRN_FC_SEL = CHECKER_TRN_FC_SEL_TCL;
  checker_fc_dump();
}

void checker_fc_dump_transmit_credit_consumed(void) {
  printf("transmit_credit_consumed : \n  ");
  CHECKER_CSR_TRN_FC_SEL = CHECKER_TRN_FC_SEL_TCC;
  checker_fc_dump();
}

void checker_fc_dump_all(void) {
  checker_fc_dump_receive_buffer_available_space();
  checker_fc_dump_receive_credits_granted_to_the_link_partner();
  checker_fc_dump_receive_credits_consumed();
  checker_fc_dump_transmit_user_credits_available();
  checker_fc_dump_transmit_credit_limit();
  checker_fc_dump_transmit_credit_consumed();
}

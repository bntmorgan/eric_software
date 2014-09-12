#include <hw/trn.h>
#include <trn.h>

#include <stdio.h>

void trn_stat_dump(void) {
  unsigned int trn_stat = TRN_CSR_STAT_TRN;
  printf("trn_lnk_up_n(%x)\n", trn_stat);
}

/**
 * PCI config space decoding
 */
void trn_pci_dump_command(void) {
  pci_command_t f = {TRN_CSR_CFG_COMMAND}; 
  printf("COMMAND : raw(0x%02x), io_space_enable(%d), memory_space_enable(%d),\n"
      "bus_master_enable(%d), parity_error_response(%d), serr_enable(%d),\n"
      "interrupt_disable(%d)\n", f.raw, f.io_space_enable,
      f.memory_space_enable, f.bus_master_enable, f.parity_error_response,
      f.serr_enable, f.interrupt_disable);
}

void trn_pci_dump_dstatus(void) {
  pci_dstatus_t f = {TRN_CSR_CFG_DSTATUS}; 
  printf("DSTATUS : transaction_pending(%x), aux_power_detected(%x),\n"
      "unsupported_request_detected(%x), fatal_error_detected(%x),\n"
      "non_fatal_error_detected(%x), correctable_error_detected(%x)\n",
      f.transaction_pending, f.aux_power_detected,
      f.unsupported_request_detected, f.fatal_error_detected,
      f.non_fatal_error_detected, f.correctable_error_detected);
}

void trn_pci_dump_dcommand(void) {
  pci_dcommand_t f = {TRN_CSR_CFG_DCOMMAND}; 
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

void trn_pci_dump_lstatus(void) {
  pci_lstatus_t f = {TRN_CSR_CFG_LSTATUS}; 
  printf("LSTATUS : link_autonomous_bandwidth_status(%x),\n"
      "link_bandwidth_management_status(%x), data_link_layer_link_active(%x),\n"
      "slot_clock_configuration(%x), link_training(%x),\n"
      "negociated_link_width(0x%x), current_link_speed(0x%x)\n",
      f.link_autonomous_bandwidth_status, f.link_bandwidth_management_status,
      f.data_link_layer_link_active, f.slot_clock_configuration,
      f.link_training, f.negociated_link_width, f.current_link_speed);
}

void trn_pci_dump_lcommand(void) {
  pci_lcommand_t f = {TRN_CSR_CFG_LCOMMAND}; 
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

void trn_pci_dump_dcommand2(void) {
  pci_dcommand2_t f = {TRN_CSR_CFG_DCOMMAND2}; 
  printf("DCOMMAND2 : completion_timeout_disable(0x%x),\n"
      "completion_timeout_value(0x%x)\n", f.completion_timeout_disable,
      f.completion_timeout_value);
}

void trn_pci_dump_address(void) {
  unsigned int pci_addr = TRN_CSR_CFG_PCI_ADDR;
  printf("Device addr %02x:%02x.%01x\n", (pci_addr >> 8) & 0xff, (pci_addr >>
        3) & 0x1f, (pci_addr >> 0) & 0x7);
}

void trn_fc_dump(void) {
  printf(
      "completion_data(0x%x), "
      "completion header(0x%x), "
      "non_posted_data(0x%x), "
      "non_posted_header(0x%x), "
      "posted_data(0x%x), "
      "posted_header(0x%x)\n", TRN_CSR_TRN_FC_CPLD,
      TRN_CSR_TRN_FC_CPLH, TRN_CSR_TRN_FC_NPD, TRN_CSR_TRN_FC_NPH,
      TRN_CSR_TRN_FC_PD, TRN_CSR_TRN_FC_PH);
}

void trn_fc_dump_receive_buffer_available_space(void) {
  printf("receive_buffer_available_space : \n  ");
  TRN_CSR_TRN_FC_SEL = TRN_TRN_FC_SEL_RBAS;
  trn_fc_dump();
}

void trn_fc_dump_receive_credits_granted_to_the_link_partner(void) {
  printf("receive_credits_granted_to_the_link_partner : \n  ");
  TRN_CSR_TRN_FC_SEL = TRN_TRN_FC_SEL_RCGTTLP;
  trn_fc_dump();
}

void trn_fc_dump_receive_credits_consumed(void) {
  printf("receive_credits_consumed : \n  ");
  TRN_CSR_TRN_FC_SEL = TRN_TRN_FC_SEL_RCC;
  trn_fc_dump();
}

void trn_fc_dump_transmit_user_credits_available(void) {
  printf("transmit_user_credits_available : \n  ");
  TRN_CSR_TRN_FC_SEL = TRN_TRN_FC_SEL_TUCA;
  trn_fc_dump();
}

void trn_fc_dump_transmit_credit_limit(void) {
  printf("transmit_credit_limit : \n  ");
  TRN_CSR_TRN_FC_SEL = TRN_TRN_FC_SEL_TCL;
  trn_fc_dump();
}

void trn_fc_dump_transmit_credit_consumed(void) {
  printf("transmit_credit_consumed : \n  ");
  TRN_CSR_TRN_FC_SEL = TRN_TRN_FC_SEL_TCC;
  trn_fc_dump();
}

void trn_fc_dump_all(void) {
  trn_fc_dump_receive_buffer_available_space();
  trn_fc_dump_receive_credits_granted_to_the_link_partner();
  trn_fc_dump_receive_credits_consumed();
  trn_fc_dump_transmit_user_credits_available();
  trn_fc_dump_transmit_credit_limit();
  trn_fc_dump_transmit_credit_consumed();
}

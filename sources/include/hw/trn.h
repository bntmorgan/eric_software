#ifndef __HW_TRN_H
#define __HW_TRN_H

#include <hw/common.h>
#include <stdint.h>

#define TRN_CSR_STAT_TRN          MMPTR(0xe0002000)
#define TRN_CSR_CFG_PCI_ADDR      MMPTR(0xe0002004)
#define TRN_CSR_CFG_COMMAND       MMPTR(0xe0002008)
#define TRN_CSR_CFG_DSTATUS       MMPTR(0xe000200c)
#define TRN_CSR_CFG_DCOMMAND      MMPTR(0xe0002010)
#define TRN_CSR_CFG_DCOMMAND2     MMPTR(0xe0002014)
#define TRN_CSR_CFG_LSTATUS       MMPTR(0xe0002018)
#define TRN_CSR_CFG_LCOMMAND      MMPTR(0xe000201c)
#define TRN_CSR_TRN_FC_CPLD       MMPTR(0xe0002020)
#define TRN_CSR_TRN_FC_CPLH       MMPTR(0xe0002024)
#define TRN_CSR_TRN_FC_NPD        MMPTR(0xe0002028)
#define TRN_CSR_TRN_FC_NPH        MMPTR(0xe000202c)
#define TRN_CSR_TRN_FC_PD         MMPTR(0xe0002030)
#define TRN_CSR_TRN_FC_PH         MMPTR(0xe0002034)
#define TRN_CSR_TRN_FC_SEL        MMPTR(0xe0002038)

// Register FC SEL
#define TRN_TRN_FC_SEL_RBAS       (0x0)
#define TRN_TRN_FC_SEL_RCGTTLP    (0x1)
#define TRN_TRN_FC_SEL_RCC        (0x2)
#define TRN_TRN_FC_SEL_TUCA       (0x4)
#define TRN_TRN_FC_SEL_TCL        (0x5)
#define TRN_TRN_FC_SEL_TCC        (0x6)

// CFG registers

typedef union _pci_command_t {
  uint16_t raw; 
  struct {
    uint16_t:5;
    uint16_t interrupt_disable:1;
    uint16_t:1;
    uint16_t serr_enable:1;
    uint16_t:1;
    uint16_t parity_error_response:1;
    uint16_t:3;
    uint16_t bus_master_enable:1;
    uint16_t memory_space_enable:1;
    uint16_t io_space_enable:1;
  }; // Big endian !
} __attribute__((packed)) pci_command_t;

typedef union _pci_dstatus_t {
  uint16_t raw; 
  struct {
    uint16_t:10;
    uint16_t transaction_pending:1;
    uint16_t aux_power_detected:1;
    uint16_t unsupported_request_detected:1;
    uint16_t fatal_error_detected:1;
    uint16_t non_fatal_error_detected:1;
    uint16_t correctable_error_detected:1;
  }; // Big endian !
} __attribute__((packed)) pci_dstatus_t;

typedef union _pci_dcommand_t {
  uint16_t raw; 
  struct {
    uint16_t:1;
    uint16_t max_read_request_size:3;
    uint16_t enable_no_snoop:1;
    uint16_t auxiliary_power_pm_enable:1;
    uint16_t phantom_fonctions_enable:1;
    uint16_t extended_tag_field_enable:1;
    uint16_t max_payload_size:3;
    uint16_t enable_relaxed_ordering:1;
    uint16_t unsupported_request_reporting_enable:1;
    uint16_t fatal_error_reporting_enable:1;
    uint16_t non_fatal_error_reporting_enable:1;
    uint16_t correctable_error_reporting_enable:1;
  }; // Big endian !
} __attribute__((packed)) pci_dcommand_t;

typedef union _pci_lstatus_t {
  uint16_t raw; 
  struct {
    uint16_t link_autonomous_bandwidth_status:1;
    uint16_t link_bandwidth_management_status:1;
    uint16_t data_link_layer_link_active:1;
    uint16_t slot_clock_configuration:1;
    uint16_t link_training:1;
    uint16_t:1;
    uint16_t negociated_link_width:6;
    uint16_t current_link_speed:4;
  }; // Big endian !
} __attribute__((packed)) pci_lstatus_t;

typedef union _pci_lcommand_t {
  uint16_t raw; 
  struct {
    uint16_t:4;
    uint16_t link_autonomous_bandwidth_interrupt_enable:1;
    uint16_t link_bandwidth_management_interrupt_enable:1;
    uint16_t hardware_autonomous_width_disable:1;
    uint16_t enable_clock_power_management:1;
    uint16_t extended_synch:1;
    uint16_t common_clock_configuration:1;
    uint16_t retrain_link:1;
    uint16_t link_disable:1;
    uint16_t read_completion_boundary:1;
    uint16_t :1;
    uint16_t active_state_link_pm_control:2;
  }; // Big endian !
} __attribute__((packed)) pci_lcommand_t;

typedef union _pci_dcommand2_t {
  uint16_t raw; 
  struct {
    uint16_t reserved:11;
    uint16_t completion_timeout_disable:1;
    uint16_t completion_timeout_value:4;
  }; // Big endian !
} __attribute__((packed)) pci_dcommand2_t;

#endif /* __HW_TRN_H */


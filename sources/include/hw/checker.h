#ifndef __HW_CHECKER_H
#define __HW_CHECKER_H

#include <hw/common.h>
#include <stdint.h>

#define CHECKER_CSR_ADDRESS_LOW       MMPTR(0xe000f000)
#define CHECKER_CSR_ADDRESS_HIGH      MMPTR(0xe000f004)
#define CHECKER_CSR_CTRL              MMPTR(0xe000f008)
#define CHECKER_CSR_STAT              MMPTR(0xe000f00C)
#define CHECKER_CSR_MODE_DATA_LOW     MMPTR(0xe000f010)
#define CHECKER_CSR_MODE_DATA_HIGH    MMPTR(0xe000f014)
#define CHECKER_CSR_STAT_TRN_CPT      MMPTR(0xe000f018)
#define CHECKER_CSR_STAT_TRN          MMPTR(0xe000f01C)
#define CHECKER_CSR_CFG_PCI_ADDR      MMPTR(0xe000f020)
#define CHECKER_CSR_CFG_COMMAND       MMPTR(0xe000f024)
#define CHECKER_CSR_CFG_DSTATUS       MMPTR(0xe000f028)
#define CHECKER_CSR_CFG_DCOMMAND      MMPTR(0xe000f02c)
#define CHECKER_CSR_CFG_DCOMMAND2     MMPTR(0xe000f030)
#define CHECKER_CSR_CFG_LSTATUS       MMPTR(0xe000f034)
#define CHECKER_CSR_CFG_LCOMMAND      MMPTR(0xe000f038)
#define CHECKER_CSR_TRN_FC_CPLD       MMPTR(0xe000f03c)
#define CHECKER_CSR_TRN_FC_CPLH       MMPTR(0xe000f040)
#define CHECKER_CSR_TRN_FC_NPD        MMPTR(0xe000f044)
#define CHECKER_CSR_TRN_FC_NPH        MMPTR(0xe000f048)
#define CHECKER_CSR_TRN_FC_PD         MMPTR(0xe000f04c)
#define CHECKER_CSR_TRN_FC_PH         MMPTR(0xe000f050)
#define CHECKER_CSR_TRN_FC_SEL        MMPTR(0xe000f054)

#define CHECKER_ADDR_MPU              0x20000000
#define CHECKER_ADDR_HM               0x20008000

// Modes
#define CHECKER_MODE_SINGLE           (0)
#define CHECKER_MODE_AUTO             (1)
#define CHECKER_MODE_READ             (2)
#define CHECKER_MODE_DUMMY            (3)

// Register Ctrl
#define CHECKER_CTRL_IRQ_EN           (0x1)
#define CHECKER_CTRL_MODE_SINGLE      (CHECKER_MODE_SINGLE << 1)
#define CHECKER_CTRL_MODE_AUTO        (CHECKER_MODE_AUTO << 1)
#define CHECKER_CTRL_MODE_READ        (CHECKER_MODE_READ << 1)
#define CHECKER_CTRL_MODE_DUMMY       (CHECKER_MODE_DUMMY << 1)
#define CHECKER_CTRL_START            (0x8)

// Register Status
#define CHECKER_STAT_EVENT_DONE       (0x1)
#define CHECKER_STAT_EVENT_ERROR      (0x2)
#define CHECKER_STAT_EVENT_USER_IRQ   (0x4)

// Register FC SEL
#define CHECKER_TRN_FC_SEL_RBAS       (0x0)
#define CHECKER_TRN_FC_SEL_RCGTTLP    (0x1)
#define CHECKER_TRN_FC_SEL_RCC        (0x2)
#define CHECKER_TRN_FC_SEL_TUCA       (0x4)
#define CHECKER_TRN_FC_SEL_TCL        (0x5)
#define CHECKER_TRN_FC_SEL_TCC        (0x6)

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

#endif /* __HW_CHECKER_H */

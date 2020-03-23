#ifndef __HW_HM_H
#define __HW_HM_H

#include <hw/common.h>
#include <stdint.h>

#define HM_CSR_CTRL              MMPTR(0xe000d000)
#define HM_CSR_STAT              MMPTR(0xe000d004)
#define HM_CSR_ADDRESS_LOW       MMPTR(0xe000d008)
#define HM_CSR_ADDRESS_HIGH      MMPTR(0xe000d00C)
#define HM_CSR_CPT_RX            MMPTR(0xe000d010)
#define HM_CSR_CPT_TX            MMPTR(0xe000d014)
#define HM_CSR_STATE_RX          MMPTR(0xe000d018)
#define HM_CSR_STATE_TX          MMPTR(0xe000d01C)
#define HM_CSR_STATE             MMPTR(0xe000d020)
#define HM_CSR_BAR_BITMAP        MMPTR(0xe000d024)
#define HM_CSR_BAR_NUMBER        MMPTR(0xe000d028)
#define HM_CSR_DATA              MMPTR(0xe000d02C)
#define HM_CSR_ID                MMPTR(0xe000d030)
#define HM_CSR_RX_TLP_DW         MMPTR(0xe000d034)
#define HM_CSR_CPT_TX_DROP       MMPTR(0xe000d038)
#define HM_CSR_TX_ERROR          MMPTR(0xe000d03c)
#define HM_CSR_CPT_TX_START      MMPTR(0xe000d040)

#define HM_MEMORY_ADDR           MMPTR(0x20000000)
#define HM_EXPANSION_ROM_ADDR    MMPTR(0x20001000)
#define HM_BAR_ADDR              MMPTR(0x20002000)

// Register Ctrl
#define HM_CTRL_IRQ_EN           (0x1)
#define HM_CTRL_START_READ       (0x2)
#define HM_CTRL_START_WRITE      (0x4)
#define HM_CTRL_DEBUG            (0x8)
#define HM_CTRL_NO_SNOOP         (0x10)
#define HM_CTRL_OVERRIDE_ID      (0x20)
#define HM_CTRL_LOCK_WRITE       (0x40)

// Register Status
#define HM_STAT_EVENT_DONE       (0x1)
#define HM_STAT_EVENT_TX_TIMEOUT (0x2)
#define HM_STAT_EVENT_RX_TIMEOUT (0x4)
#define HM_STAT_EVENT_READ_EXP   (0x8)
#define HM_STAT_EVENT_WRITE_BAR  (0x10)
#define HM_STAT_EVENT_WR_TIMEOUT (0x20)

// Bar registers
#define HM_BAR_CTRL              MMPTR(0x20002040)

// Bar control register
#define HM_BAR_CTRL_EN           (0x1)

#endif /* __HW_HM_H */

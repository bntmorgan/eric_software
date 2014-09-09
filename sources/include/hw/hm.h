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

#define HM_MEMORY_ADDR           MMPTR(0x20000000)

// Register Ctrl
#define HM_CTRL_IRQ_EN           (0x1)
#define HM_CTRL_START            (0x2)

// Register Status
#define HM_STAT_EVENT_DONE       (0x1)
#define HM_STAT_EVENT_TX_TIMEOUT (0x2)
#define HM_STAT_EVENT_RX_TIMEOUT (0x4)

#endif /* __HW_HM_H */

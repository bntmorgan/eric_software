#ifndef __HW_CHECKER_H
#define __HW_CHECKER_H

#include <hw/common.h>

#define CHECKER_CSR_ADDRESS_LOW       MMPTR(0xe000f000)
#define CHECKER_CSR_ADDRESS_HIGH      MMPTR(0xe000f004)
#define CHECKER_CSR_CTRL              MMPTR(0xe000f008)
#define CHECKER_CSR_STAT              MMPTR(0xe000f00C)
#define CHECKER_CSR_MODE_DATA_LOW     MMPTR(0xe000f010)
#define CHECKER_CSR_MODE_DATA_HIGH    MMPTR(0xe000f014)
#define CHECKER_CSR_STAT_TRN_CPT      MMPTR(0xe000f018)
#define CHECKER_CSR_STAT_TRN          MMPTR(0xe000f01C)

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
#define CHECKER_STAT_EVENT_DONE     (0x1)
#define CHECKER_STAT_EVENT_ERROR    (0x2)
#define CHECKER_STAT_EVENT_USER_IRQ (0x4)

#endif /* __HW_CHECKER_H */


#ifndef __HW_MPU_H
#define __HW_MPU_H

#include <hw/common.h>
#include <stdint.h>

#define MPU_CSR_CTRL              MMPTR(0xe000c000)
#define MPU_CSR_STAT              MMPTR(0xe000c004)
#define MPU_CSR_USER_DATA_LOW     MMPTR(0xe000c008)
#define MPU_CSR_USER_DATA_HIGH    MMPTR(0xe000c00C)

#define MPU_MEMORY_ADDR           MMPTR(0x10000000)

// Register Ctrl
#define MPU_CTRL_IRQ_EN           (0x1)
#define MPU_CTRL_START            (0x2)

// Register Status
#define MPU_STAT_EVENT_DONE       (0x1)
#define MPU_STAT_EVENT_ERROR      (0x2)
#define MPU_STAT_EVENT_USER_IRQ   (0x4)

#endif /* __HW_MPU_H */

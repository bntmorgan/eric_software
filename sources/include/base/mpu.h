#ifndef __MPU_H
#define __MPU_H

void mpu_isr(void);

int mpu_start(void);

void mpu_init(void);

void mpu_memory_test(void);

#endif /* __MPU_H */

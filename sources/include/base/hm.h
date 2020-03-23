#ifndef __HM_H
#define __HM_H

#include <stdint.h>

// Return codes
#define HM_OK               0x0
#define HM_ERROR_RX_TIMEOUT 0x1
#define HM_ERROR_TX_TIMEOUT 0x2
#define HM_ERROR_TIMEOUT    0x3
#define HM_ERROR_WR_TIMEOUT 0x4

void hm_isr(void);

int hm_start_read(int low, int high);

int hm_start_write_pwn(int low, int high, int data);

int hm_start_write(int low, int high, int data, int no_snoop);

void hm_init(void);

void hm_stop_all(void);

void hm_stat(void);

void hm_set_bar_bitmap(uint32_t bar_bitmap);

void hm_enable_irq(void);

void hm_disable_irq(void);

extern int hm_read_exp;
extern int hm_write_bar;
extern uint32_t hm_read_exp_time;
extern uint32_t hm_write_bar_time;

#endif /* __HM_H */

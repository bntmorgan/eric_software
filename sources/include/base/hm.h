#ifndef __HM_H
#define __HM_H

// Return codes
#define HM_OK               0x0
#define HM_ERROR_RX_TIMEOUT 0x1
#define HM_ERROR_TX_TIMEOUT 0x2
#define HM_ERROR_TIMEOUT    0x3

void hm_isr(void);

int hm_start(int low, int high);

void hm_init(void);

void hm_stat(void);

#endif /* __HM_H */

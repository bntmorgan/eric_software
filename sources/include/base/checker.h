#ifndef __CHECKER_H
#define __CHECKER_H

void checker_isr(void);

void checker_dummy_start(int low_value, int high_value);
void checker_single_start(int low_value, int high_value);

void checker_init(void);

void checker_memory_test(void);

void checker_print_hm_stat(void);

#endif /* __CHECKER_H */

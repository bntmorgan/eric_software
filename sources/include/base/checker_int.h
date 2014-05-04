#ifndef __CHECKER_INT__
#define __CHECKER_INT__

#define CHECKER_INT_MAX 256

typedef struct _user_data_t {
  int l;
  int h;
} user_data_t;

void checker_int_init(void);
// void checker_int_enqueue(int l, int h);
void checker_int_enqueue(int user_data);
int checker_int_dequeue(int *user_data);
int checker_int_iterate(int init, int *user_data);
void checker_int_test(void);

#endif//_CHECKER_INT__

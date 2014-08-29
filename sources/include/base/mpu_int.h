#ifndef __MPU_INT__
#define __MPU_INT__

#define MPU_INT_MAX 256

typedef struct _user_data_t {
  int l;
  int h;
} user_data_t;

void mpu_int_init(void);
// void mpu_int_enqueue(int l, int h);
void mpu_int_enqueue(int user_data);
int mpu_int_dequeue(int *user_data);
int mpu_int_iterate(int init, int *user_data);
void mpu_int_test(void);

#endif//_MPU_INT__

#include <stdio.h>

#include <mpu_int.h>

// user_data_t ints[MPU_INT_MAX];
int ints[MPU_INT_MAX];
int ints_tail;
int ints_head;
int ints_size;

void mpu_int_init(void) {
  printf("ints_tail %x, ints_head %x, ints_size %x\n", ints_tail, ints_head,
      ints_size);
  ints_tail = 0;
  ints_head = 0;
  ints_size = 0;
}

void mpu_int_enqueue(int user_data) {
  ints[ints_head] = user_data;
  // Increment queue head
  ints_head = ((ints_head + 1)) % MPU_INT_MAX;
  // Increment queue tail if head == tail
  if (ints_size == MPU_INT_MAX) {
    ints_tail = ints_head;
  } else {
    ints_size++;
  }
}

int mpu_int_dequeue(int *user_data) {
  if (ints_size > 0) {
    *user_data = ints[ints_tail];
    ints_tail = ((ints_tail + 1)) % MPU_INT_MAX; 
    ints_size--;
    return 0;
  }
  return -1;
}

int mpu_int_iterate(int init, int *user_data) {
  static int tidx;
  static int cpt;
  if (init) {
    tidx = ints_tail;
    cpt = 0;
    return 0;
  }
  if (cpt < ints_size) {
    *user_data = ints[tidx];
    tidx = ((tidx + 1)) % MPU_INT_MAX; 
    cpt++;
    return 0;
  }
  return -1;
}

void mpu_int_test(void) {
  int i;
  int ud;
  // normal usage
  mpu_int_init();
  for (i = 0; i < MPU_INT_MAX + 2; i++) {
    mpu_int_enqueue(i);
  }
  while (!mpu_int_dequeue(&ud)) {
    printf("Dequeued %x\n", ud);
  }
  // Iterator
  mpu_int_init();
  for (i = 0; i < MPU_INT_MAX + 2; i++) {
    mpu_int_enqueue(i);
  }
  mpu_int_iterate(1, NULL);
  while (!mpu_int_iterate(0, &ud)) {
    printf("Iterated %x\n", ud);
  }
}

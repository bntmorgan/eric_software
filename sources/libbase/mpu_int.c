#include <stdio.h>

#include <mpu_int.h>

// user_data_t ints[MPU_INT_MAX];
user_data_t ints[MPU_INT_MAX];
int ints_tail;
int ints_head;
int ints_size;

void mpu_int_init(void) {
//   printf("ints_tail %x, ints_head %x, ints_size %x\n", ints_tail, ints_head,
//       ints_size);
  ints_tail = 0;
  ints_head = 0;
  ints_size = 0;
}

void mpu_int_enqueue(int udl, int udh) {
  ints[ints_head].l = udl;
  ints[ints_head].h = udh;
  // Increment queue head
  ints_head = ((ints_head + 1)) % MPU_INT_MAX;
  // Increment queue tail if head == tail
  if (ints_size == MPU_INT_MAX) {
    ints_tail = ints_head;
  } else {
    ints_size++;
  }
}

int mpu_int_dequeue(int *udl, int *udh) {
  if (ints_size > 0) {
    *udl = ints[ints_tail].l;
    *udh = ints[ints_tail].h;
    ints_tail = ((ints_tail + 1)) % MPU_INT_MAX; 
    ints_size--;
    return 0;
  }
  return -1;
}

int mpu_int_iterate(int init, int *udl, int *udh) {
  static int tidx;
  static int cpt;
  if (init) {
    tidx = ints_tail;
    cpt = 0;
    return 0;
  }
  if (cpt < ints_size) {
    *udl = ints[tidx].l;
    *udh = ints[tidx].h;
    tidx = ((tidx + 1)) % MPU_INT_MAX; 
    cpt++;
    return 0;
  }
  return -1;
}

void mpu_int_test(void) {
  int i;
  int udl, udh;
  // normal usage
  mpu_int_init();
  for (i = 0; i < MPU_INT_MAX + 2; i++) {
    mpu_int_enqueue(i, i);
  }
  while (!mpu_int_dequeue(&udl, &udh)) {
    printf("Dequeued %x %x\n", udl, udh);
  }
  // Iterator
  mpu_int_init();
  for (i = 0; i < MPU_INT_MAX + 2; i++) {
    mpu_int_enqueue(i, i);
  }
  mpu_int_iterate(1, NULL, NULL);
  while (!mpu_int_iterate(0, &udl, &udh)) {
    printf("Iterated %x %x\n", udl, udh);
  }
}

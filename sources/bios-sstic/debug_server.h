#ifndef __DEBUG_SERVER_H
#define __DEBUG_SERVER_H

#include <stdint.h>

#define MTU       1532

void debug_server_run(void);
int debug_server_init(unsigned int ip);

enum DEBUG_SERVER_MESSAGE_TYPES {
  MESSAGE_MESSAGE,
  MESSAGE_EXEC_CONTINUE,
  MESSAGE_INFO,
  MESSAGE_MEMORY_READ,
  MESSAGE_MEMORY_DATA,
  MESSAGE_MEMORY_WRITE,
  MESSAGE_COMMIT,
  MESSAGE_MEMORY_SCAN,
  MESSAGE_MEMORY_SCAN_REPORT
};

//
// Messages
//

typedef struct _message {
  uint8_t type;
} __attribute__((packed)) message;

typedef struct _message_info {
  uint8_t type;
  uint64_t length;
} __attribute__((packed)) message_info;

typedef struct _message_memory_read {
  uint8_t type;
  uint64_t address;
  uint64_t length;
} __attribute__((packed)) message_memory_read;

typedef struct _message_memory_data {
  uint8_t type;
  uint64_t address;
  uint64_t length;
} __attribute__((packed)) message_memory_data;

typedef struct _message_memory_write {
  uint8_t type;
  uint64_t address;
  uint64_t length;
} __attribute__((packed)) message_memory_write;

typedef struct _message_commit {
  uint8_t type;
  uint8_t ok;
} __attribute__((packed)) message_commit;

typedef struct _message_memory_scan {
  uint8_t type;
  uint64_t address;
  uint64_t pages;
} __attribute__((packed)) message_memory_scan;

typedef struct _message_memory_scan_report {
  uint8_t type;
  uint8_t code;
  uint64_t length;
} __attribute__((packed)) message_memory_scan_report;

static inline void *message_check_type(message *m, uint8_t type) {
  if (m->type == type) {
    return m;
  } else {
    return (void *)0x0;
  }
}

#endif /* __DEBUG_SERVER_H */

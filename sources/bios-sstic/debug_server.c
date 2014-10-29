#include <debug_server.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <net/microudp.h>

#include <hw/hm.h>
#include <hm.h>

#include <mpu.h>
#include <mpu_int.h>

#define PORT_OUT	54323
#define PORT_IN		54322

static uint8_t *tbuf;
uint8_t rbuf[MTU];
static uint8_t is_msg;

static inline void wait(void) {
  int z, k;
  for (k=0; k < 0x100; k++)
    for (z=0; z< 0x100; z++)
      asm("nop;");
}

uint8_t debug_server_recv(void) {
  is_msg = 0;
	while (!is_msg) {
		microudp_service();
	}
  return 0;
}

uint8_t debug_server_send(uint32_t length) {
	microudp_send(PORT_IN, PORT_OUT, length);
  return 0;
}

void debug_server_handle_memory_read(message_memory_read *mr) {
  uint64_t length = (mr->length + sizeof(message_memory_data) > MTU) ?
    MTU - sizeof(message_memory_data) : mr->length;
  uint32_t addr_low = mr->address;
  uint32_t addr_high = mr->address >> 32;
  uint32_t length_low = length;
  uint32_t length_high = length >> 32;
  printf("Read HM @ 0x%08x%08x 0x%08x%08x bytes\n", addr_high, addr_low,
      length_high, length_low);

  // Get the data from host mem
  hm_start(addr_low, addr_high);

  // Answer to the client 
  message_memory_data *r = (message_memory_data *)tbuf;
  r->type = MESSAGE_MEMORY_DATA;
  r->address = mr->address;
  r->length = length;
  uint8_t *b = (uint8_t *)&tbuf[0] + sizeof(message_memory_data);
  memcpy(b, (void *)&HM_MEMORY_ADDR, length);

  // Send the message
  debug_server_send(length + sizeof(message_memory_data));
}

void debug_server_handle_memory_scan(message_memory_scan *mr) {
  int udl, udh, rc;
  uint32_t i = 0, j;
  uint64_t *data = (uint64_t *)((uint8_t *)tbuf + sizeof(message_memory_scan_report));
  uint32_t addr_low = mr->address;
  uint32_t addr_high = mr->address >> 32;
  uint32_t pages = mr->pages;
  // Answer to the client 
  message_memory_scan_report *r = (message_memory_scan_report *)tbuf;

  r->type = MESSAGE_MEMORY_SCAN_REPORT;
  r->length = 0;

  printf("Run MPU @ 0x%08x%08x for 0x%08x times\n", addr_high, addr_low, pages);

  // Runs number of asked times
  for (j = 0; j < pages; j++) {
    // Runs HM on the current page
    rc = hm_start((addr_low & ~0xfff), addr_high);
    if(rc) {
      printf("Failed to read the page STAT %08x\n", HM_CSR_STAT);
      r->code = -1;
      break;
    }
    // Runs MPU on the current page
    if (mpu_start() == -1) {
      r->code = -1;
      // Error, it's time to go
      break;
    } else {
      r->code = 0;
      mpu_int_iterate(1, NULL, NULL);
      while (!mpu_int_iterate(0, &udl, &udh)) {
        // printf("Iterated %x %x\n", udl, udh);
        if (MTU < ((r->length + 1) * 16) + sizeof(message_memory_scan_report)) {
          // We send immediately the current report
          r->code = 1;
          debug_server_send(r->length * 16 +
              sizeof(message_memory_scan_report));
          // Reset length
          r->length = 0;
        }
        r->code = 0;
        r->length++;
        data[2 * i] = addr_low | ((uint64_t)addr_high << 32);
        data[2 * i + 1] = udl | ((uint64_t)udh << 32);
        i++;
      }
    }
    mr->address += 0x1000; 
    addr_low = mr->address;
    addr_high = mr->address >> 32;
  }

  // Send the message
  debug_server_send(r->length * 16 + sizeof(message_memory_scan_report));
}

static void rx_callback(unsigned int src_ip, unsigned short src_port, unsigned
    short dst_port, void *_data, unsigned int length) {
  is_msg = 1;
  memcpy(&rbuf[0], _data, length);
}

int debug_server_init(unsigned int ip) {
	tbuf = microudp_get_tx_buffer();
	microudp_set_callback(rx_callback);
	if(!microudp_arp_resolve(ip))
		return -1;
  return 0;
}

void debug_server_run(void) {
  message *mr = (message *)&rbuf[0];
  mr->type = MESSAGE_MESSAGE;
  while (mr->type != MESSAGE_EXEC_CONTINUE) {
    debug_server_recv();
    // We have received a message
    printf("debug_server : message received\n");
    switch (mr->type) {
      case MESSAGE_MEMORY_READ:
        debug_server_handle_memory_read((message_memory_read*)mr);
        break;
      case MESSAGE_MEMORY_SCAN:
        debug_server_handle_memory_scan((message_memory_scan*)mr);
        break;
      default: {
        // nothing
      }
    }
  }
}

#include <challenge.h>
#include <hw/sysctl.h>
#include <hal/time.h>
#include <hal/sleep.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <hm.h>
#include <hw/hm.h>
#include <debug_server.h>
#include <net/microudp.h>
#include <net/tftp.h>
#include <system.h>
#include <crc.h>
#include <env/challenge.h>

#define ABS(x) (((x) < 0) ? -(x) : (x))

#define CHALLENGE_PERIOD 5050000 // 5 secondes
#define CHALLENGE_PERIOD_MIN_DISPLAY  1000000 // 1 seconde
#define BUF_SIZE 4096

static uint32_t period = CHALLENGE_PERIOD;

static inline void wait(void) {
  int z, k;
  for (k=0; k < 0x100; k++)
    for (z=0; z< 0x100; z++)
      asm("nop;");
}

static inline void wait2(void) {
  int z;
  for (z=0; z< 0x100; z++)
    asm("nop;");
}

void challenge_init(void) {
}

void challenge_set_period(uint32_t p) {
  period = p;
}

static char cb[BUF_SIZE];
static solution_header sh;
static char *solution;

int challenge_dl(char *name, unsigned char *mac, unsigned char *lip, unsigned
    char *rip) {
  int r;
  if (name == NULL || strlen(name) == 0) {
    name = "exp.bin";
  }
  microudp_start(mac, IPTOINT(lip[0], lip[1], lip[2], lip[3]));
  int ip = IPTOINT(rip[0], rip[1], rip[2], rip[3]);

  // Get the challenge
  r = tftp_get(ip, name, &cb[0]);
  if (r == -1) {
    printf("Failed to download : %s\n", name);
    return r;
  }
  printf("Received %d bytes\n", r);
  flush_cpu_dcache();

  // Compute the CRC
	unsigned int length = *(uint32_t *)&cb[0x00];
	unsigned int expected_crc = *(uint32_t *)&cb[0x04];
	unsigned int actual_crc = crc32((unsigned char *)&cb[0x08], length);

  printf("CRC : expected = 0x%08x, actual = 0x%08x\n", expected_crc,
      actual_crc);
  if (expected_crc != actual_crc) {
    printf("CRC check failed, leaving...\n");
    return -1;
  }

  // Parse the solution header
  memset(&sh, 0, sizeof(solution_header));
  sh.size = cb[0x0b] << 24 | cb[0x0a] << 16 | cb[0x09] << 8 | cb[0x08];
  sh.time = cb[0x0f] << 24 | cb[0x0e] << 16 | cb[0x0d] << 8 | cb[0x0c];
  sh.bar_bitmap = cb[0x13] << 24 | cb[0x12] << 16 | cb[0x11] << 8 | cb[0x10];
  solution = &cb[0x14];
  printf("Solution header : size = 0x%08x, time = 0x%08x, bar_bitmap = 0x%08x\n"
      , sh.size, sh.time, sh.bar_bitmap);
  hm_set_bar_bitmap(sh.bar_bitmap);
  
  // Finally copy the challenge
  char *dst = (char *)&HM_EXPANSION_ROM_ADDR;
  char *src = &cb[0x08 + sh.size];
  memcpy(dst, src, length - 0x8 - sh.size);
  return r;
}

void challenge_send_result(uint8_t ok) {
	uint8_t *tbuf = microudp_get_tx_buffer();

  message_user_defined *m = (message_user_defined *)&tbuf[0];
  uint8_t *data = tbuf + sizeof(message_user_defined);
  m->type = MESSAGE_USER_DEFINED;
  m->user_type = USER_DEFINED_LOG_CHALLENGE;
  m->length = 1;
  data[0] = ok;
  debug_server_send(sizeof(message_user_defined) + 1);
}

void challenge_run(void) {
  struct timestamp t1, t2, tc, tl;
  int c = 1, o = 1, ok1 = 0, ok2 = 0;
  uint32_t stat;
  uint64_t diff;
  // Configure BAR ctrl register
  HM_BAR_CTRL = HM_BAR_CTRL_EN;
  printf("Enable BAR0 control register : 0x%08x\n", HM_BAR_CTRL);
  // Init that shit
  hm_init();
  HM_CSR_STAT = -1;
  time_get(&tl);
  // Run motherfucker run !
  while (c) {
    while (c && !(ok1 && ok2)) {
      // Get time
      time_get(&tc);
      // Get the current time
      stat = HM_CSR_STAT;
      // Overwait ?
      // TODO convertir en usec
      diff = ((uint64_t)tc.sec * 1000000 + tc.usec) - ((uint64_t)tl.sec *
          1000000 + tl.usec);
      // User stop ?
      c = (HM_BAR_CTRL & HM_BAR_CTRL_EN) == HM_BAR_CTRL_EN;
      o = diff < period;
      if (!c || !o) {
        break;
      }
      if (stat & HM_STAT_EVENT_READ_EXP) {
        t1 = tc;
        HM_CSR_STAT = HM_STAT_EVENT_READ_EXP;
        ok1 = 1;
      }
      if (stat & HM_STAT_EVENT_WRITE_BAR) {
        t2 = tc;
        HM_CSR_STAT = HM_STAT_EVENT_WRITE_BAR;
        ok2 = 1;
      }
    }
    ok1 = 0, ok2 = 0;
    if (!o) {
      tl = tc;
    } else {
      tl = t2;
    }
    if (c) {
      diff = ((uint64_t)t2.sec * 1000000 + t2.usec) - ((uint64_t)t1.sec *
          1000000 + t1.usec);
      if (!o) {
        if (period > CHALLENGE_PERIOD_MIN_DISPLAY) {
          printf("Missed challenge period\n");
        }
        challenge_send_result(0);
      } else if (sh.time < diff) {
        if (period > CHALLENGE_PERIOD_MIN_DISPLAY) {
          printf("VMM didn't write the solution in time : 0x%08x%08x\n", diff &
              0xffffffff, diff >> 32);
        }
        challenge_send_result(0);
      } else {
        if (period > CHALLENGE_PERIOD_MIN_DISPLAY) {
          printf("t1 = 0x%08x.0x%08x, t2 = 0x%08x.0x%08x, diff = 0x%08x%08x\n",
              t1.sec, t1.usec, t2.sec, t2.usec, diff & 0xffffffff, diff >> 32);
        }
        challenge_send_result(1);
      }
    }
  }
  if (!c) {
    printf("User stop!\n");
  }
}
    // printf("rom %d, bar %d, ctrl 0x%08x, status 0x%08x, \n", hm_read_exp,
    //     hm_write_bar, HM_CSR_CTRL, HM_CSR_STAT);


// void challenge_run(void) {
//   int ok;
//   struct timestamp t;
//   hm_enable_irq();
//   // Configure BAR ctrl register
//   HM_BAR_CTRL = HM_BAR_CTRL_EN;
//   printf("Enable BAR0 control register : 0x%08x\n", HM_BAR_CTRL);
//   while ((HM_BAR_CTRL & HM_BAR_CTRL_EN) == HM_BAR_CTRL_EN) {
//     // Wait for expansion rom reading
//     usleep_until(CHALLENGE_PERIOD, &hm_read_exp, &ok);
//     hm_read_exp = 0;
//     if (!ok) {
//       printf("Host didn't read hm expansion rom\n");
//       challenge_send_result(0);
//       continue;
//     }
//     // Wait for BAR write
//     usleep_until(sh.time, &hm_write_bar, &ok);
//     hm_write_bar = 0;
//     if (!ok) {
//       printf("Host didn't write the BARs\n");
//       challenge_send_result(0);
//       continue;
//     }
//     // Test the results
//     uint32_t i;
//     char *b = (char *)&HM_BAR_ADDR;
//     for (i = 0; i < sh.size; i++) {
//       if (solution[i] != b[i]) {
//         ok = 0;
//         challenge_send_result(0);
//         break;
//       }
//     }
//     if (!ok) {
//       continue;
//     }
//     // Send the results
//     challenge_send_result(1);
//     // Succeded
//     time_get(&t);
//     printf("Uptime : Secs : 0x%08x ; USecs 0x%08x\n", t.sec, t.usec);
//   }
//   hm_disable_irq();
// }

// void challenge_run(void) {
//   int ok;
//   struct timestamp t;
//   hm_enable_irq();
//   // Configure BAR ctrl register
//   HM_BAR_CTRL = HM_BAR_CTRL_EN;
//   printf("Enable BAR0 control register : 0x%08x\n", HM_BAR_CTRL);
//   // *(uint8_t*)0x2000201f = 0xff;
//   while ((HM_BAR_CTRL & HM_BAR_CTRL_EN) == HM_BAR_CTRL_EN) {
//     // Wait for expansion rom reading
//     usleep_until(CHALLENGE_PERIOD, &hm_read_exp, &ok);
//     hm_read_exp = 0;
//     if (!ok) {
//       printf("Host didn't read hm expansion rom\n");
//       challenge_send_result(0);
//       continue;
//     }
//     // Wait for BAR write
//     usleep_until(sh.time, &hm_write_bar, &ok);
//     hm_write_bar = 0;
//     // *(uint8_t*)0x2000201f = 0xff;
//     if (!ok) {
//       printf("Host didn't write the BARs\n");
//       challenge_send_result(0);
//       continue;
//     }
//     // Test the results
//     uint32_t i;
//     char *b = (char *)&HM_BAR_ADDR;
//     for (i = 0; i < sh.size; i++) {
//       if (solution[i] != b[i]) {
//         ok = 0;
//         challenge_send_result(0);
//         break;
//       }
//     }
//     if (!ok) {
//       continue;
//     }
//     // Send the results
//     challenge_send_result(1);
//     // Succeded
//     time_get(&t);
//     printf("Uptime : Secs : 0x%08x ; USecs 0x%08x\n", t.sec, t.usec);
//   }
//   hm_disable_irq();
// }

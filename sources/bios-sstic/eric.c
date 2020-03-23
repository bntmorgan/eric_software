
void challenge_run(void) {
  int ok;
  struct timestamp t;
  hm_enable_irq();
  // Configure BAR ctrl register
  HM_BAR_CTRL = HM_BAR_CTRL_EN;
  printf("Enable BAR0 control register : 0x%08x\n", HM_BAR_CTRL);
  // *(uint8_t*)0x2000201f = 0xff;
  while ((HM_BAR_CTRL & HM_BAR_CTRL_EN) == HM_BAR_CTRL_EN) {
    // Wait for expansion rom reading
    usleep_until(CHALLENGE_PERIOD, &hm_read_exp, &ok);
    hm_read_exp = 0;
    if (!ok) {
      printf("Host didn't read hm expansion rom\n");
      challenge_send_result(0);
      continue;
    }
    // Wait for BAR write
    usleep_until(sh.time, &hm_write_bar, &ok);
    hm_write_bar = 0;
    // *(uint8_t*)0x2000201f = 0xff;
    if (!ok) {
      printf("Host didn't write the BARs\n");
      challenge_send_result(0);
      continue;
    }
    // Test the results
    uint32_t i;
    char *b = (char *)&HM_BAR_ADDR;
    for (i = 0; i < sh.size; i++) {
      if (solution[i] != b[i]) {
        ok = 0;
        challenge_send_result(0);
        break;
      }
    }
    if (!ok) {
      continue;
    }
    // Send the results
    challenge_send_result(1);
    // Succeded
    time_get(&t);
    printf("Uptime : Secs : 0x%08x ; USecs 0x%08x\n", t.sec, t.usec);
  }
  hm_disable_irq();
}

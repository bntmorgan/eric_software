#ifndef __CHALLENGE_H
#define __CHALLENGE_H

#include <stdint.h>

int challenge_dl(char *name, unsigned char *mac, unsigned char *lip, unsigned
    char *rip);
void challenge_init(void);
void challenge_run(void);
void challenge_set_period(uint32_t p);

#endif /* __CHALLENGE_H */
